#pragma once

#include <Windows.h>
#include <string>
using namespace std;

class ProgressScanDialog
{
public:
    ProgressScanDialog();
    ~ProgressScanDialog();

    bool Create();
    void Destroy();
    bool IsValid() const { return m_hWnd != NULL; }

    void SetProgress(int iFilesScanned, int iTotalFiles, const wstring& sCurrentFile);
    void SetStatus(const wstring& sStatus);

    void ProcessMessages();

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd;
    HWND m_hProgressBar;
    HWND m_hStatusText;
    HWND m_hFileText;
    int m_iFilesScanned;

    static const wchar_t* CLASSNAME;
};

class ProgressLoadDialog
{
public:
    ProgressLoadDialog();
    ~ProgressLoadDialog();

    bool Create();
    void Destroy();
    bool IsValid() const { return m_hWnd != NULL; }

    void SetFilename(const std::wstring& sFilename);
    void SetStatus(const std::wstring& sStatus);
    void SetTrack(int iCurrent, int iTotal);
    void SetEvent(int iCurrent, int iTotal);
    void SetProgress(int iCurrent, int iTotal);

    HWND GetHwnd() const { return m_hWnd; }

    void ProcessMessages();

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd;
    HWND m_hProgressBar;
    HWND m_hStatusText;
    HWND m_hFileText;
    HWND m_hTrack;
    HWND m_hEvent;

    static const wchar_t* CLASSNAME;
};
