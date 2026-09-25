// ShellExecAsUser.cpp : Defines the entry point for the DLL application.
//
#include <windows.h>
#include <shlDisp.h>
#include <atlbase.h>
#include <Exdisp.h>
#include <shobjidl.h>
#include <shlguid.h>
//#include <process.h>
#include "nsis/pluginapi.h" // nsis plugin
#include <shellApi.h>
//#pragma comment(linker,"/merge:.rdata=.text")

//VistaTools
#define NO_DLL_IMPORTS 1
#define DONTWANT_RunNonElevated 1
#define IMPLEMENT_VISTA_TOOLS
#include "VistaTools.cxx"
//////////////////////////////////////

//missing in old sdk, had to replace shlwapi.lib as well
//LWSTDAPI IUnknown_QueryService(__in IUnknown* punk, __in REFGUID guidService, __in REFIID riid, __deref_out void ** ppvOut);


HINSTANCE g_hInstance;
HWND g_hwndParent;
HWND g_hwnd;
HANDLE g_hThread;

typedef struct _ExecShellParams {
	TCHAR action[50];
	TCHAR command[_MAX_PATH];
	TCHAR params[1024];
	TCHAR directory[_MAX_PATH];
	int iShow;
} ExecShellParams;

UINT_PTR NSISPluginCallback(enum NSPIM Event) 
{
	switch(Event) 
	{
	case NSPIM_UNLOAD:
		if (g_hThread) {
			//OutputDebugString("ExecShellAsUser: NSPIM_UNLOAD wait...");
			::PostMessage(g_hwnd, WM_USER+11, 0, 0);
			::WaitForSingleObject(g_hThread, INFINITE);
			::CloseHandle(g_hThread);
			//OutputDebugString("ExecShellAsUser: NSPIM_UNLOAD");
		}
		break;
	}
	return NULL;
}

//http://brandonlive.com/2008/04/27/getting-the-shell-to-run-an-application-for-you-part-2-how/
static CComQIPtr<IShellDispatch2> retrieveDesktop()
{
	CComPtr<IShellWindows> psw;
	HRESULT hr = psw.CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_LOCAL_SERVER);
	if (SUCCEEDED(hr))
	{
		HWND hwnd;
		CComPtr<IDispatch> pdisp;
		VARIANT vEmpty;
		vEmpty.vt = VT_EMPTY;
		if (S_OK == (hr = psw->FindWindowSW(&vEmpty, &vEmpty, 0x8/*SWC_DESKTOP*/, (long*)&hwnd, SWFO_NEEDDISPATCH, &pdisp)))
		{
			CComPtr<IShellBrowser> psb;

			hr = IUnknown_QueryService(pdisp, SID_STopLevelBrowser, __uuidof(IShellBrowser), (void**) &psb); //IID_PPV_ARGS(&psb)
			if (SUCCEEDED(hr))
			{
				CComPtr<IShellView> psv;
				hr = psb->QueryActiveShellView(&psv);

				CComPtr<IDispatch> pdispBackground;
				HRESULT hr = psv->GetItemObject(SVGIO_BACKGROUND, __uuidof(IDispatch), (void**) &pdispBackground);
				if (SUCCEEDED(hr))
				{
					if (CComQIPtr<IShellFolderViewDual> psfvd = pdispBackground)
					{
						CComPtr<IDispatch> pdisp;
						hr = psfvd->get_Application(&pdisp);
						if (SUCCEEDED(hr))
						{
							return CComQIPtr<IShellDispatch2>(pdisp);
						}
					}
				}
			}
		} else {
			TCHAR buf[100];
			wsprintf(buf, _T("ShellExecAsUser: FindWindowSW failed: %x"), hr);
			OutputDebugString(buf);
		}
	}
	return CComQIPtr<IShellDispatch2>();
}

static int getShowMode(const TCHAR* s)
{
	if (s[0] == 0) return SW_SHOWNORMAL;
	const TCHAR* strModes[] = { _T("SW_SHOWDEFAULT"), _T("SW_SHOWNORMAL"), _T("SW_SHOWMAXIMIZED"), _T("SW_SHOWMINIMIZED"), _T("SW_HIDE") };
	int modes[] = { SW_SHOWDEFAULT, SW_SHOWNORMAL, SW_SHOWMAXIMIZED, SW_SHOWMINIMIZED, SW_HIDE };
	for (int i=0; i<sizeof(strModes)/sizeof(TCHAR*); ++i) {
		if (lstrcmp(s, strModes[i]) == 0)
			return modes[i];
	}
	return SW_SHOWNORMAL;
}

static bool runCommand(IShellDispatch2* psd, ExecShellParams* params)
{
	if (!psd) return false;
	CComBSTR command(params->command);
	CComVariant vAction, vParams, vDir;
	if (params->action[0] != 0) vAction = params->action;
	if (params->params[0] != 0) vParams = params->params;
	if (params->directory[0] != 0) vDir = params->directory;
	CComVariant vShow;
	if (params->iShow != -1) vShow = params->iShow;
//#pragma push_macro("ShellExecute")
//#undef ShellExecute
	HRESULT hr = psd->ShellExecute(command, vParams, vDir, vAction, vShow); 
//#pragma pop_macro("ShellExecute")
	return hr == S_OK;
}

static LRESULT WndProc(HWND hWnd, UINT uMsg,WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:
		{
			void* param = ((LPCREATESTRUCT)lParam)->lpCreateParams;
			::SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)param);
			break;
		}
		case WM_USER+10:
		{
			ExecShellParams* p = (ExecShellParams*)lParam;
			IShellDispatch2* pSD = (IShellDispatch2*)::GetWindowLongPtr(hWnd, GWL_USERDATA);
			runCommand(pSD, p);
			delete p;
			break;
		}
		case WM_USER+11:
			DestroyWindow(hWnd);
			break;
		case WM_DESTROY:
		{	
			PostQuitMessage(0);
			break;
		}
		default:
			return ::DefWindowProc(hWnd,uMsg,wParam,lParam);
	};
	return 0;
}

HWND CreateMessageWindow(void* data)
{
    WNDCLASSEX wincl;
	ZeroMemory(&wincl, sizeof(WNDCLASSEX));
    wincl.hInstance = g_hInstance;
    wincl.lpszClassName = _T("NSIS_DESKTOP_LAUNCH_CLASS");
    wincl.lpfnWndProc = (WNDPROC)WndProc;      
    wincl.cbSize = sizeof (WNDCLASSEX);

	ATOM atom = RegisterClassEx (&wincl);

	return CreateWindowEx(
           0,
           MAKEINTATOM(atom),
           _T(""),
           0,
           0,      
           0,      
           0,           
           0,            
           HWND_MESSAGE,        
           NULL,                
           wincl.hInstance,    
           data); 

}

static unsigned __stdcall WorkerThreadProc(void* pParam)
{
	HANDLE hRunEvent = (HANDLE) pParam;
	::CoInitialize(NULL);
	{
		CComPtr<IShellDispatch2> pSD = retrieveDesktop();
		OutputDebugString(pSD.p ? _T("ExecShellAsUser: got desktop") : _T("ExecShellAsUser: failed to retrieve desktop!"));
		if (pSD.p) g_hwnd = CreateMessageWindow(pSD.p);
		SetEvent(hRunEvent);

		if (g_hwnd) {
			MSG msg;
			while ( GetMessage(&msg, NULL, 0, 0) > 0 )
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	::CoUninitialize();
	OutputDebugString(_T("ExecShellAsUser: thread finished"));
	return 0;
}

#define EMPTY_TO_NULL(s) (s[0] == 0 ? NULL : s)

extern "C"
void __declspec(dllexport) ShellExecAsUser(HWND hwndParent, int string_size, 
                                      TCHAR *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
	static int initDone=0;

	g_hwndParent=hwndParent;

	EXDLL_INIT();
	if (initDone == 0) {
		initDone=1;
		extra->RegisterPluginCallback(g_hInstance, NSISPluginCallback);
		if (IsVista() && IsElevated(NULL) == S_OK) {
			//note: COM calls to shell interfaces have to be executed on another thread,
			//otherwise I got RPC_E_CANTCALLOUT_ININPUTSYNCCALL error.
			HANDLE hRunEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL); //manual reset
			g_hThread = (HANDLE)_beginthreadex( NULL, 0, WorkerThreadProc, hRunEvent, 0, NULL);
			if (g_hThread) ::WaitForSingleObject(hRunEvent, INFINITE);
			CloseHandle(hRunEvent);
			OutputDebugString(_T("ExecShellAsUser: elevated process detected"));
		} else {
			OutputDebugString(_T("ExecShellAsUser: process is not elevated, will fallback to ShellExecute"));
		}
	}

	// action command [parameters] [SW_SHOWDEFAULT | SW_SHOWNORMAL | SW_SHOWMAXIMIZED | SW_SHOWMINIMIZED | SW_HIDE]
	ExecShellParams execShellParams;
	ExecShellParams* params = g_hwnd ? new ExecShellParams : &execShellParams;
	TCHAR show[50];
	popstring(params->action);
	popstring(params->command);
	popstring(params->params);
	popstring(show);
	params->iShow = getShowMode(show);
	TCHAR* dir = getuservariable(INST_OUTDIR);

	if (g_hwnd) {
		if (dir) lstrcpyn(params->directory, dir, sizeof(params->directory)/sizeof(params->directory[0]));
		::PostMessage(g_hwnd, WM_USER+10, 0, (LPARAM)params);
	} else {
		::ShellExecute(NULL, EMPTY_TO_NULL(params->action), params->command, EMPTY_TO_NULL(params->params), dir, params->iShow);
	}
} 

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if(ul_reason_for_call==DLL_PROCESS_ATTACH) 
	{
		g_hInstance = (HINSTANCE) hModule;
	} else if (ul_reason_for_call==DLL_PROCESS_DETACH) {
		OutputDebugString(_T("ExecShellAsUser: DLL_PROCESS_DETACH"));
	}
    return TRUE;
}

