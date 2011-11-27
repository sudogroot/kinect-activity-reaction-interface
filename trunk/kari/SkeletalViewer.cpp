/************************************************************************
*																		*
*	SkeletalViewer.cpp -- This module provides sample code used to	  *
*						 demonstrate Kinect NUI processing			 *
*																		*
* Copyright (c) Microsoft Corp. All rights reserved.					*
*																		*
* This code is licensed under the terms of the						  *
* Microsoft Kinect for Windows SDK (Beta) from Microsoft Research		*
* License Agreement: http://research.microsoft.com/KinectSDK-ToU		*
*																		*
************************************************************************/

// Note: 
//  1. DirectX SDK must be installed before compiling. 
//  2. DirectX SDK include path should be added after the VC include
//	 path, because strsafe.h in DirectX SDK may be older.
//  3. platform SDK lib path should be added before the VC lib
//	 path, because uuid.lib in VC lib path may be older




#include "stdafx.h"
#include "SkeletalViewer.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Global Variables:
CKARIApp			g_CKARIApp;	// Application class
HINSTANCE			g_hInst;				// current instance
HWND				g_hWndApp;				// Windows Handle to main application
TCHAR				g_szAppTitle[256];		// Application title
NUI_IMAGE_FRAME *	pImageFrame_depth;
LPTSTR				commandLineString;


int APIENTRY _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow ) {
	MSG			msg;
	WNDCLASS	wc;
	
	commandLineString = lpCmdLine;

	// Store the instance handle
	g_hInst = hInstance;

	LoadString( g_hInst, IDS_APPTITLE, g_szAppTitle, sizeof( g_szAppTitle ) / sizeof( g_szAppTitle[0] ) );

	// Dialog custom window class
	ZeroMemory( &wc, sizeof( wc ) );
	wc.style		= CS_HREDRAW | CS_VREDRAW;
	wc.cbWndExtra	= DLGWINDOWEXTRA;
	wc.hInstance	= hInstance;
	wc.hCursor		= LoadCursor( NULL,IDC_ARROW );
	wc.hIcon		= LoadIcon( hInstance, MAKEINTRESOURCE( IDI_SKELETALVIEWER ) );
	wc.lpfnWndProc	= DefDlgProc;
	wc.lpszClassName= SZ_APPDLG_WINDOW_CLASS;

	if( !RegisterClass( &wc ) )
		return (0);

	// Create main application window
	g_hWndApp = CreateDialogParam( g_hInst, MAKEINTRESOURCE(IDD_APP), NULL, (DLGPROC) CKARIApp::WndProc, NULL );

	// Show window
	ShowWindow( g_hWndApp, nCmdShow );
	UpdateWindow( g_hWndApp );

	// Main message loop:
	while( GetMessage( &msg, NULL, 0, 0 ) ) {
		// If a dialog message
		if( g_hWndApp != NULL && IsDialogMessage( g_hWndApp, &msg ) )
			continue;

		// otherwise do default window processing
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	
	return ( msg.wParam );
}

LONG CALLBACK CKARIApp::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	
	switch( message ) {
		case WM_INITDIALOG: {

			LOGFONT lf;

			// Clean state the class
			g_CKARIApp.Nui_Zero();

			// Bind application window handle
			g_CKARIApp.m_hWnd = hWnd;

			// Initialize and start NUI processing
			g_CKARIApp.Nui_Init( commandLineString );

			// Set the font for Frames Per Second
			GetObject( (HFONT)GetStockObject( DEFAULT_GUI_FONT ), sizeof( lf ), &lf );
			lf.lfHeight *= 3;
			
			g_CKARIApp.m_hFontFPS = CreateFontIndirect( &lf );
			
			SendDlgItemMessage( hWnd, IDC_FPS,				WM_SETFONT, (WPARAM)g_CKARIApp.m_hFontFPS, 0 );
			
			SendDlgItemMessage( hWnd, IDC_CAMERA_W,			WM_SETFONT, (WPARAM)g_CKARIApp.m_hFontFPS, 0 );
			SendDlgItemMessage( hWnd, IDC_CAMERA_ANGLE,		WM_SETFONT, (WPARAM)g_CKARIApp.m_hFontFPS, 0 );
			
			SendDlgItemMessage( hWnd, IDC_PERSON_HEIGHT,	WM_SETFONT, (WPARAM)g_CKARIApp.m_hFontFPS, 0 );
			SendDlgItemMessage( hWnd, IDC_PERSON_ELEVATION,	WM_SETFONT, (WPARAM)g_CKARIApp.m_hFontFPS, 0 );
			
			SendDlgItemMessage( hWnd, IDC_DISP_MISC,		WM_SETFONT, (WPARAM)g_CKARIApp.m_hFontFPS, 0 );
			
		}
		break;

		// If the titlebar X is clicked destroy app
		case WM_CLOSE:
			DestroyWindow( hWnd );
		break;

		case WM_DESTROY:
			// Uninitialize NUI
			g_CKARIApp.Nui_UnInit();

			// Other cleanup
			DeleteObject( g_CKARIApp.m_hFontFPS );

			// Quit the main message pump
			PostQuitMessage(0);
		break;
	}

	return (FALSE);

}



/*****************************************************************************/
/* int MessageBoxResourceV(HWND hwnd,UINT nID,UINT nType, ... )
/*
/* Superset of MessageBox functionality allowing a user to specify a String
/* table loaded string 
/*****************************************************************************/
int MessageBoxResource( HWND hwnd, UINT nID, UINT nType ) {
	static TCHAR szRes[512];
	int		 nRet;
	LoadString( g_hInst, nID, szRes, sizeof(szRes) / sizeof(szRes[0]) );
	nRet = ::MessageBox( hwnd, szRes, g_szAppTitle, nType );
	return (nRet);
}


