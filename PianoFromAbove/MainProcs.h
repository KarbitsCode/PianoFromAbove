/*************************************************************************************************
*
* File: MainProcs.h
*
* Description: Defines the main GUI functions. Not C++ :/
*
* Copyright (c) 2010 Brian Pantano. All rights reserved.
*
*************************************************************************************************/
#pragma once

#include <Windows.h>
#include <CommCtrl.h>
#include <memory>
#include <string>
using namespace std;

class MainScreen;
class PreferencesSnapshot;
struct RenderSettingsState
{
    MainScreen *pGameState = NULL;
    wstring sLoadedFile;
    shared_ptr< PreferencesSnapshot > pSnapshot;
};

// Message handlers for the main windows
LRESULT WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
HMENU GetMainMenu();
VOID SizeWindows( int iMainWidth, int iMainHeight );

LRESULT WINAPI GfxProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
VOID CopyMenuState( HMENU hMenuSrc, HMENU hMenuDest );

LRESULT WINAPI BarProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
HWND CreateRebar( HWND hWndOwner );
VOID DrawSliderChannel( LPNMCUSTOMDRAW lpnmcd, HWND hWndOwner );

LRESULT WINAPI PosnProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
VOID GetChannelRect( HWND hWnd, RECT *rcChannel );
VOID GetThumbRect( HWND hWnd, int iPosition, const RECT *rcChannel, RECT *rcThumb );
INT GetThumbPosition( short iXPos, RECT *rcChannel );
VOID MoveThumbPosition( int iPositionNew, int &iPosition, HWND hWnd, RECT *rcChannel, RECT *rcThumbOld, BOOL bUpdateGame = TRUE );

INT_PTR WINAPI LibDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
VOID PopulateLibrary( HWND hWndLibrary );
VOID SortLibrary( HWND hWndLibrary, INT iSortCol );
INT CALLBACK CompareLibrary( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );
VOID AddSingleLibraryFile( HWND hWndLibrary, const wstring &sFile );
BOOL PlayLibrary( HWND hWndLibrary, int iItem, bool bCustomSettings = false );

INT_PTR WINAPI AboutProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

VOID HandOffMsg( UINT msg, WPARAM wParam, LPARAM lParam );
VOID ShowLibrary( BOOL bShow );
VOID ShowControls( BOOL bShow );
VOID ShowKeyboard( BOOL bShow );
VOID ShowNoteLabels( BOOL bShow );
VOID SetOnTop( BOOL bOnTop );
VOID SetFullScreen( BOOL bFullScreen );
VOID SetZoomMove( BOOL bZoomMove );
VOID SetMute( BOOL bMute );
VOID SetSpeed( DOUBLE dSpeed );
VOID SetNSpeed( DOUBLE dSpeed );
VOID SetVolume( DOUBLE dVolume );
VOID SetPosition( INT iPosition );
VOID SetPlayable( BOOL bPlayable );
VOID SetPlayMode( INT ePlayMode );
VOID SetPlayPauseStop( BOOL bPlay, BOOL bPause, BOOL bStop );
BOOL PlayFile( const wstring &sFile, bool bCustomSettings = false, bool bLibraryEligible = false );

BOOL RenderVideo( MainScreen *pGameState, const wstring &sOutFile, const wstring &sFFmpegPath, HWND hWndNotify = NULL, shared_ptr< void > pRenderGuard = nullptr );
wstring GetPathText( HWND hWnd, int iId );
bool BrowseForFile( HWND hWnd, int iEditId, LPCTSTR sFilter, LPCTSTR sTitle, bool bSave, LPCTSTR sDefExt );
bool LoadMIDIForRender( HWND hWnd, RenderSettingsState &state, const wstring &sFile );
bool EnsureMIDILoad( HWND hWnd, RenderSettingsState &state );
bool StartRender( HWND hWnd, RenderSettingsState &state );
VOID ShowRenderDialog( HWND hWndOwner );
INT_PTR WINAPI RenderSettingsProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

VOID CheckActivity( BOOL bIsActive, POINT *ptNew = NULL, BOOL bToggleEnable = false );
