#pragma once

#include <Windows.h>
#include <string>
using namespace std;

class ProgressDialog
{
public:
    ProgressDialog();
    ~ProgressDialog();

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
