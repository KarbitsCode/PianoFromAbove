/*************************************************************************************************
*
* File: Globals.h
*
* Description: Global variables. Mostly window handlers.
*
* Copyright (c) 2010 Brian Pantano. All rights reserved.
*
*************************************************************************************************/
#pragma once

#include <Windows.h>
#include "Notification.h"
#include "Misc.h"

extern HINSTANCE g_hInstance;
extern HWND g_hWnd;
extern HWND g_hWndBar;
extern HWND g_hWndLibDlg;
extern HWND g_hWndGfx;
extern TSQueue< MSG > g_MsgQueue; // Producer/consumer to hold events for our game thread
extern LPWSTR g_sMIDILoadPending;
extern IMMDeviceEnumerator* g_pDeviceEnumerator;
extern AudioNotificationClient* g_pAudioNotify;
extern BOOL g_bAboutToRestart;

// Video recording notifications
#define WM_RECORDPROGRESS ( WM_APP + 10 ) // wParam: seconds of video written, lParam: percent of the song done
#define WM_RECORDDONE     ( WM_APP + 11 ) // wParam: MB_ICON* for the message, lParam: heap-allocated wstring* to show (receiver deletes)

#define ERRORANDRETURN( hwnd, msg, retval ) { MessageBox( ( hwnd ), ( msg ), TEXT( "Error" ), MB_OK | MB_ICONERROR ); return ( retval ); }
