/*************************************************************************************************
*
* File: ConfigProcs.h
*
* Description: Defines configuration GUI functions. Not C++ :/
*
* Copyright (c) 2010 Brian Pantano. All rights reserved.
*
*************************************************************************************************/
#pragma once

#include <Windows.h>

#include "GameState.h"
#include "Config.h"
#include "MIDI.h"

// Message handlers for the configuration property sheets
enum PreferencePages
{
    PP_VISUAL = 1 << 0,
    PP_AUDIO = 1 << 1,
    PP_VIDEO = 1 << 2,
    PP_CONTROLS = 1 << 3,
    PP_LIBRARY = 1 << 4,
    PP_ALL = (1 << 5) - 1
};
enum PreferenceFlags
{
    PPF_RENDER = 1 << 0 // Opened from Render Video: leaves alone what doesn't belong there
};
VOID DoPreferences( HWND hWndOwner, UINT uPages = PP_ALL, UINT uFlags = 0 );
VOID Changed( HWND hWnd );

INT_PTR WINAPI VisualProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
VOID SetVisualProc( HWND hWnd, const VisualSettings &cVisual );
VOID FillKeysDropdown( HWND hWnd, VisualSettings::Accidentals eAccidentals );

INT_PTR WINAPI NoteSpanProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
void MIDIInProc( unsigned char cStatus, unsigned char cParam1, unsigned char cParam2, int iMilliSecs, void *pUserData );

INT_PTR WINAPI AudioProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
VOID SetAudioProc( HWND hWnd, const AudioSettings &cAudio );

INT_PTR WINAPI VideoProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
VOID SetVideoProc( HWND hWnd, const VideoSettings &cVideo, const PlaybackSettings &cPlayback );

INT_PTR WINAPI ControlsProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

INT_PTR WINAPI LibraryProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
BOOL ToggleYN( HWND hWndListview, int iItem );

BOOL GetCustomSettings( MainScreen *pGameState, HWND hWndOwner = NULL );
INT_PTR WINAPI TracksProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
