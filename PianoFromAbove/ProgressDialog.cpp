#include <Windows.h>
#include <CommCtrl.h>
#include "ProgressDialog.h"

const wchar_t* ProgressDialog::CLASSNAME = L"PianoFromAboveProgress";

ProgressDialog::ProgressDialog() : m_hWnd(NULL), m_hProgressBar(NULL), m_hStatusText(NULL), m_hFileText(NULL), m_iFilesScanned(0) {}

ProgressDialog::~ProgressDialog()
{
    Destroy();
}

bool ProgressDialog::Create()
{
    if (m_hWnd != NULL)
        return true;

    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(ProgressDialog*);
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

void ProgressDialog::Destroy()
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

void ProgressDialog::SetProgress(int iFilesScanned, int iTotalFiles, const wstring& sCurrentFile)
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

void ProgressDialog::SetStatus(const wstring& sStatus)
{
    if (!IsValid())
        return;

    SetWindowText(m_hStatusText, sStatus.c_str());
    ProcessMessages();
}

void ProgressDialog::ProcessMessages()
{
    MSG msg = { 0 };
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK ProgressDialog::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            ProgressDialog* pThis = reinterpret_cast<ProgressDialog*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
            return 0;
        }
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}
