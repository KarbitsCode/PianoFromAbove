#include <sstream>
#include <vector>
#include "VideoRecorder.h"

bool VideoRecorder::Start( const std::wstring &sOutFile, int iWidth, int iHeight, int iFPS, std::wstring &sError )
{
    if ( m_hProcess )
    {
        sError = L"A recording is already in progress.";
        return false;
    }

    // yuv420p needs even dimensions, so trim an odd pixel row/column instead of failing
    int iEvenWidth = iWidth & ~1;
    int iEvenHeight = iHeight & ~1;
    if ( iEvenWidth < 2 || iEvenHeight < 2 )
    {
        sError = L"The display is too small to record.";
        return false;
    }

    m_sOutFile = sOutFile;
    m_sLogFile = sOutFile + L".ffmpeg.log";

    // Raw BGRX in -> H.264 mp4 out. The scale filter tags the RGB->YUV conversion
    // as BT.709 so colors match what players assume for HD video.
    std::wostringstream cmd;
    cmd << L"ffmpeg.exe -y -hide_banner -loglevel warning"
        << L" -f rawvideo -pixel_format bgr0 -video_size " << iWidth << L"x" << iHeight
        << L" -framerate " << iFPS << L" -i -"
        << L" -vf \"crop=" << iEvenWidth << L":" << iEvenHeight
        << L":0:0,scale=out_color_matrix=bt709:out_range=tv,format=yuv420p\""
        << L" -c:v libx264 -preset veryfast -crf 18"
        << L" -colorspace bt709 -color_primaries bt709 -color_trc bt709 -color_range tv"
        << L" -movflags +faststart"
        << L" \"" << sOutFile << L"\"";
    std::wstring sCmd = cmd.str();
    std::vector< wchar_t > vCmd( sCmd.begin(), sCmd.end() );
    vCmd.push_back( L'\0' ); // CreateProcess wants a writable, null-terminated buffer

    // The child inherits the read end; this write end must not be inherited
    // or ffmpeg would never see the pipe close.
    SECURITY_ATTRIBUTES sa = { sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE };
    HANDLE hRead = NULL, hWrite = NULL;
    if ( !CreatePipe( &hRead, &hWrite, &sa, 0 ) )
    {
        sError = L"Could not create a pipe to ffmpeg.";
        return false;
    }
    SetHandleInformation( hWrite, HANDLE_FLAG_INHERIT, 0 );

    // ffmpeg's messages go to a log file or silence if we can't make one
    HANDLE hLog = CreateFile( m_sLogFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &sa,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( hLog == INVALID_HANDLE_VALUE )
        hLog = CreateFile( L"NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    STARTUPINFO si = { 0 };
    si.cb = sizeof( si );
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = hRead;
    si.hStdOutput = hLog;
    si.hStdError = hLog;

    PROCESS_INFORMATION pi = { 0 };
    BOOL bStarted = CreateProcess( NULL, &vCmd[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi );
    DWORD dwError = GetLastError();

    // The child has its own copies now
    CloseHandle( hRead );
    if ( hLog != INVALID_HANDLE_VALUE ) CloseHandle( hLog );

    if ( !bStarted )
    {
        CloseHandle( hWrite );
        DeleteFile( m_sLogFile.c_str() );
        if ( dwError == ERROR_FILE_NOT_FOUND || dwError == ERROR_PATH_NOT_FOUND )
            sError = L"Could not find ffmpeg.exe. Put it next to this program or add it to your PATH.";
        else
            sError = L"Could not launch ffmpeg.exe (error " + std::to_wstring( dwError ) + L").";
        return false;
    }

    CloseHandle( pi.hThread );
    m_hProcess = pi.hProcess;
    m_hPipe = hWrite;
    return true;
}

bool VideoRecorder::WriteFrame( const void *pData, size_t cbData )
{
    if ( !m_hPipe ) return false;

    const BYTE *pBytes = static_cast< const BYTE* >( pData );
    while ( cbData > 0 )
    {
        DWORD cbChunk = static_cast< DWORD >( cbData > ( 1u << 20 ) ? ( 1u << 20 ) : cbData );
        DWORD cbWritten = 0;
        if ( !WriteFile( m_hPipe, pBytes, cbChunk, &cbWritten, NULL ) || cbWritten == 0 )
            return false; // ffmpeg died (broken pipe)
        pBytes += cbWritten;
        cbData -= cbWritten;
    }
    return true;
}

bool VideoRecorder::Finish()
{
    if ( !m_hProcess ) return true;

    // EOF on stdin tells ffmpeg to flush the encoder and write the mp4 index
    if ( m_hPipe )
    {
        CloseHandle( m_hPipe );
        m_hPipe = NULL;
    }

    DWORD dwWait = WaitForSingleObject( m_hProcess, 120000 );
    DWORD dwExit = 1;
    if ( dwWait == WAIT_OBJECT_0 )
        GetExitCodeProcess( m_hProcess, &dwExit );
    else
        TerminateProcess( m_hProcess, 1 ); // Stuck sometimes, for some reason

    CloseProcess();

    bool bOK = ( dwWait == WAIT_OBJECT_0 && dwExit == 0 );
    if ( bOK ) DeleteFile( m_sLogFile.c_str() );
    return bOK;
}

void VideoRecorder::Abort()
{
    if ( !m_hProcess ) return;

    if ( m_hPipe )
    {
        CloseHandle( m_hPipe );
        m_hPipe = NULL;
    }
    TerminateProcess( m_hProcess, 1 );
    WaitForSingleObject( m_hProcess, 5000 );
    CloseProcess();

    DeleteFile( m_sOutFile.c_str() ); // An mp4 that never got its index isn't playable anyway
}

void VideoRecorder::CloseProcess()
{
    CloseHandle( m_hProcess );
    m_hProcess = NULL;
}
