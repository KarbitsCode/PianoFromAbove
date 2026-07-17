#include <Windows.h>
#include <CommCtrl.h>
#include "ProgressDialog.h"

const wchar_t* ProgressScanDialog::CLASSNAME = L"PianoFromAboveProgressScan";

ProgressScanDialog::ProgressScanDialog() : m_hWnd(NULL), m_hProgressBar(NULL), m_hStatusText(NULL), m_hFileText(NULL), m_iFilesScanned(0) {}

ProgressScanDialog::~ProgressScanDialog()
{
    Destroy();
}

bool ProgressScanDialog::Create()
{
    if (m_hWnd != NULL)
        return true;

    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(ProgressScanDialog*);
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_WAIT);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = CLASSNAME;

    if (!RegisterClassEx(&wc))
    {
        // Class might already be registered from previous instance
        DWORD dwErr = GetLastError();
        if (dwErr != ERROR_CLASS_ALREADY_EXISTS)
            return false;
    }

    // Create progress window
    m_hWnd = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        CLASSNAME,
        L"Scanning MIDI Library...",
        WS_POPUP | WS_CAPTION,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
        NULL, NULL, GetModuleHandle(NULL), this
    );

    if (!m_hWnd)
        return false;

    // Create status text
    m_hStatusText = CreateWindowEx(
        0,
        L"STATIC",
        L"Scanning for MIDI files...",
        WS_CHILD | WS_VISIBLE,
        10, 10, 380, 20,
        m_hWnd, NULL, GetModuleHandle(NULL), NULL
    );

    // Create progress bar
    m_hProgressBar = CreateWindowEx(
        0,
        PROGRESS_CLASS,
        NULL,
        WS_CHILD | WS_VISIBLE,
        10, 40, 360, 20,
        m_hWnd, NULL, GetModuleHandle(NULL), NULL
    );

    SendMessage(m_hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(m_hProgressBar, PBM_SETSTEP, 1, 0);
    SendMessage(m_hProgressBar, PBM_SETPOS, 0, 0);
    SetClassLongPtr(m_hProgressBar, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_WAIT));

    // Create progress text
    m_hFileText = CreateWindowEx(
        0,
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE,
        10, 60, 380, 110,
        m_hWnd, NULL, GetModuleHandle(NULL), NULL
    );

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    return true;
}

void ProgressScanDialog::Destroy()
{
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = NULL;
        m_hProgressBar = NULL;
        m_hStatusText = NULL;
        m_hFileText = NULL;
    }
}

void ProgressScanDialog::SetProgress(int iFilesScanned, int iTotalFiles, const wstring& sCurrentFile)
{
    if (!IsValid())
        return;

    m_iFilesScanned = iFilesScanned;

    // Update progress bar
    int iProgress = (iTotalFiles > 0) ? (iFilesScanned * 100 / iTotalFiles) : 0;
    SendMessage(m_hProgressBar, PBM_SETPOS, iProgress, 0);

    wstring sDisplay = sCurrentFile;
    if (sDisplay.length() > 40)
    {
        // Truncate if too long
        sDisplay = L"..." + sDisplay.substr(sDisplay.length() - 37);
    }

    wchar_t szBuffer[256];
    swprintf_s(szBuffer, 256, L"Files scanned: %d\n\nCurrent file:\n%s", iFilesScanned, sDisplay.c_str());
    SetWindowText(m_hFileText, szBuffer);

    ProcessMessages();
}

void ProgressScanDialog::SetStatus(const wstring& sStatus)
{
    if (!IsValid())
        return;

    SetWindowText(m_hStatusText, sStatus.c_str());
    ProcessMessages();
}

void ProgressScanDialog::ProcessMessages()
{
    MSG msg = { 0 };
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK ProgressScanDialog::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            ProgressScanDialog* pThis = reinterpret_cast<ProgressScanDialog*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
            return 0;
        }
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}


const wchar_t* ProgressLoadDialog::CLASSNAME = L"PianoFromAboveProgressLoad";

ProgressLoadDialog::ProgressLoadDialog() : m_hWnd(NULL), m_hWndParent(NULL), m_hTrackProgressBar(NULL), m_hTrackStatusText(NULL), m_hEventProgressBar(NULL), m_hEventStatusText(NULL) {}

ProgressLoadDialog::~ProgressLoadDialog()
{
    Destroy();
}

bool ProgressLoadDialog::Create(HWND hWndParent)
{
    if (m_hWnd != NULL)
        return true;

    if (hWndParent != NULL)
    {
        m_hWndParent = hWndParent;
        EnableWindow(m_hWndParent, FALSE);
    }

    const int width = 420;
    const int height = 160;

    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(ProgressLoadDialog*);
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_WAIT);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = CLASSNAME;

    if (!RegisterClassEx(&wc))
    {
        // Class might already be registered from previous instance
        DWORD dwErr = GetLastError();
        if (dwErr != ERROR_CLASS_ALREADY_EXISTS)
            return false;
    }

    // Create progress window
    m_hWnd = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        CLASSNAME,
        L"Loading...",
        WS_POPUP | WS_CAPTION | WS_VISIBLE,
        x, y, width, height,
        NULL, NULL, GetModuleHandle(NULL), this
    );

    if (!m_hWnd)
        return false;

    // Create track progress bar
    m_hTrackProgressBar = CreateWindowEx(
        0,
        PROGRESS_CLASS,
        NULL,
        WS_CHILD | WS_VISIBLE,
        10, 10, 380, 10,
        m_hWnd, NULL, GetModuleHandle(NULL), NULL
    );

    // Create track status text
    m_hTrackStatusText = CreateWindowEx(
        0,
        L"STATIC",
        L"Loading...",
        WS_CHILD | WS_VISIBLE,
        10, 30, 380, 20,
        m_hWnd, NULL, GetModuleHandle(NULL), NULL
    );

    SendMessage(m_hTrackProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(m_hTrackProgressBar, PBM_SETSTEP, 1, 0);
    SendMessage(m_hTrackProgressBar, PBM_SETPOS, 0, 0);
    SetClassLongPtr(m_hTrackProgressBar, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_WAIT));

    // Create event progress bar
    m_hEventProgressBar = CreateWindowEx(
        0,
        PROGRESS_CLASS,
        NULL,
        WS_CHILD | WS_VISIBLE,
        10, 60, 380, 10,
        m_hWnd, NULL, GetModuleHandle(NULL), NULL
    );

    // Create event status text
    m_hEventStatusText = CreateWindowEx(
        0,
        L"STATIC",
        L"Loading...",
        WS_CHILD | WS_VISIBLE,
        10, 80, 380, 20,
        m_hWnd, NULL, GetModuleHandle(NULL), NULL
    );

    //SendMessage(m_hEventProgressBar, PBM_SETMARQUEE, TRUE, 50);
    SendMessage(m_hEventProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(m_hEventProgressBar, PBM_SETSTEP, 1, 0);
    SendMessage(m_hEventProgressBar, PBM_SETPOS, 0, 0);
    SetClassLongPtr(m_hEventProgressBar, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_WAIT));

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    return true;
}

void ProgressLoadDialog::Destroy()
{
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = NULL;
        m_hTrackProgressBar = NULL;
        m_hTrackStatusText = NULL;
        m_hEventProgressBar = NULL;
        m_hEventStatusText = NULL;
    }
    if (m_hWndParent)
    {
        EnableWindow(m_hWndParent, TRUE);
        SetForegroundWindow(m_hWndParent);
        m_hWndParent = NULL;
    }
}

void ProgressLoadDialog::SetStatus(HWND hwnd, const std::wstring& sStatus)
{
    if (!IsValid())
        return;

    SetWindowText(hwnd, sStatus.c_str());
    ProcessMessages();
}

void ProgressLoadDialog::SetProgress(HWND hwnd, int iCurrent, int iTotal)
{
    if (!IsValid())
        return;

    int iProgress = (iTotal > 0) ? (iCurrent * 100 / iTotal) : 0;
    SendMessage(hwnd, PBM_SETPOS, iProgress, 0);
    ProcessMessages();
}

void ProgressLoadDialog::SetFilename(const std::wstring& sFilename)
{
    if (!IsValid())
        return;

    SetWindowText(m_hWnd, (L"Loading " + sFilename + L"...").c_str());
    ProcessMessages();
}

void ProgressLoadDialog::SetTrackProgress(int iCurrent, int iTotal)
{
    SetStatus(m_hTrackStatusText, L"Parsing track " +
        std::to_wstring(iCurrent) +
        L" of " +
        std::to_wstring(iTotal) + L"...");
    SetProgress(m_hTrackProgressBar, iCurrent, iTotal);
}

void ProgressLoadDialog::SetEventProgress(int iCurrent, int iTotal)
{
    SetStatus(m_hEventStatusText, L"Parsing event " +
        std::to_wstring(iCurrent) +
        L" of " +
        std::to_wstring(iTotal) + L"...");
    SetProgress(m_hEventProgressBar, iCurrent, iTotal);
}

void ProgressLoadDialog::ProcessMessages()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK ProgressLoadDialog::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        ProgressLoadDialog* pThis = reinterpret_cast<ProgressLoadDialog*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
        return 0;
    }
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}
