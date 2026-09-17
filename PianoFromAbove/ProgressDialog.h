#pragma once

#include <Windows.h>
#include <string>

class ProgressScanDialog
{
public:
    ProgressScanDialog();
    ~ProgressScanDialog();

    ProgressScanDialog(const ProgressScanDialog&) = delete;
    ProgressScanDialog& operator=(const ProgressScanDialog&) = delete;

    bool Create();
    void Destroy();
    bool IsValid() const { return m_hWnd != NULL; }

    void SetProgress(int iFilesScanned, int iTotalFiles, const std::wstring& sCurrentFile);
    void SetStatus(const std::wstring& sStatus);

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

    ProgressLoadDialog(const ProgressLoadDialog&) = delete;
    ProgressLoadDialog& operator=(const ProgressLoadDialog&) = delete;

    bool Create(HWND hWndParent = NULL);
    void Destroy();
    bool IsValid() const { return m_hWnd != NULL; }

    void SetStatus(HWND hwnd, const std::wstring& sStatus);
    void SetProgress(HWND hwnd, int iCurrent, int iTotal);
    void SetFilename(const std::wstring& sFilename);
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

    ProgressStatusDialog(const ProgressStatusDialog&) = delete;
    ProgressStatusDialog& operator=(const ProgressStatusDialog&) = delete;

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
