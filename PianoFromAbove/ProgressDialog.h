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

    bool Create(HWND hWndParent = NULL);
    void Destroy();
    bool IsValid() const { return m_hWnd != NULL; }

    void SetFilename(const std::wstring& sFilename);
    void SetStatus(HWND hWnd, const std::wstring& sStatus);
    void SetProgress(HWND hWnd, int iCurrent, int iTotal);
    void SetTrackProgress(int iCurrent, int iTotal);
    void SetEventProgress(int iCurrent, int iTotal);

    void ProcessMessages();

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd;
    HWND m_hWndParent;
    HWND m_hTrackProgressBar;
    HWND m_hTrackStatusText;
    HWND m_hEventProgressBar;
    HWND m_hEventStatusText;

    static const wchar_t* CLASSNAME;
};

class ProgressStatusDialog
{
public:
    ProgressStatusDialog();
    ~ProgressStatusDialog();

    bool Create(HWND hWndParent = NULL);
    void Destroy();
    bool IsValid() const { return m_hWnd != NULL; }

    void SetFilename(const std::wstring& sFilename);
    void SetStatus(const std::wstring& sStatus);

    void ProcessMessages();

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd;
    HWND m_hWndParent;
    HWND m_hStatusText;
    HWND m_hProgressBar;

    static const wchar_t* CLASSNAME;
};
