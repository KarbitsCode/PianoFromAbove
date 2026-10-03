#pragma once

#include <Windows.h>
#include <string>

class VideoRecorder
{
public:
    VideoRecorder() : m_hProcess( NULL ), m_hPipe( NULL ) {}
    ~VideoRecorder() { Finish(); }

    // Finds the ffmpeg to run.
    static bool Locate( const std::wstring &sFFmpegPath, std::wstring &sResolved, std::wstring &sError );

    // Frames are expected as tightly packed BGRX pixels (iWidth * iHeight * 4 bytes each).
    bool Start( const std::wstring &sOutFile, int iWidth, int iHeight, int iFPS, const std::wstring &sFFmpegPath, std::wstring &sError );

    // Feeds one frame to ffmpeg. Blocks while ffmpeg's encoder is busy.
    bool WriteFrame( const void *pData, size_t cbData ) const;

    // Closes ffmpeg's stdin so it can finish the file, then waits for it to exit.
    // Returns true if ffmpeg exited cleanly
    bool Finish();

    // Kills ffmpeg and deletes the unfinished video
    void Abort();

    bool IsActive() const { return m_hProcess != NULL; }
    const std::wstring& GetOutFile() const { return m_sOutFile; }
    const std::wstring& GetLogFile() const { return m_sLogFile; }

private:
    void CloseProcess();

    HANDLE m_hProcess; // ffmpeg process
    HANDLE m_hPipe; // write end of ffmpeg's stdin
    std::wstring m_sOutFile;
    std::wstring m_sLogFile; // ffmpeg's stdout/stderr
};
