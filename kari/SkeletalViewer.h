/************************************************************************
*																		*
*	SkeletalViewer.h -- Declares of CSkeletalViewerApp class			*
*																		*
* Copyright (c) Microsoft Corp. All rights reserved.					*
*																		*
* This code is licensed under the terms of the						  *
* Microsoft Kinect for Windows SDK (Beta) from Microsoft Research		*
* License Agreement: http://research.microsoft.com/KinectSDK-ToU		*
*																		*
************************************************************************/

#pragma once

#include "resource.h"
#include "MSR_NuiApi.h"
#include "DrawDevice.h"

#define SZ_APPDLG_WINDOW_CLASS		_T("KinectActivityReactionInterfaceAppDlgWndClass")


class CKARIApp {

public:
	HRESULT	Nui_Init( LPTSTR commandLineString );
	void	Nui_UnInit( );
	void	Nui_GotDepthAlert( );
	bool	checkAction_handOpen( int unsigned jointId );
	void	Nui_GotVideoAlert( );
	void	Nui_GotSkeletonAlert( );
	void	Nui_Zero();
	void	Nui_BlankSkeletonScreen( HWND hWnd );
	void	Nui_DoDoubleBuffer( HWND hWnd, HDC hDC );
	void	Nui_DrawSkeleton( bool bBlank, NUI_SKELETON_DATA * pSkel, HWND hWnd, int unsigned skeletonNumber );
	void	actionsDetect( bool bBlank, NUI_SKELETON_DATA * pSkel, HWND hWnd, int unsigned skeletonNumber );
	bool	bodySegmentsMeasure( NUI_SKELETON_DATA * pSkel );
	bool	checkSkeletonValid( NUI_SKELETON_DATA * pSkel );
	void	Nui_DrawSkeletonSegment( NUI_SKELETON_DATA * pSkel, int unsigned numJoints, ... );
	
	static	DWORD WINAPI actionExecuteOn(void * argsInput);
	static	DWORD WINAPI testThread(void * argsInput);
	static	DWORD WINAPI testStart(LPVOID pParam);

	RGBQUAD	Nui_ShortToQuad_Depth( USHORT s );

	static	LONG CALLBACK	WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HWND	m_hWnd;

private:
	
	static DWORD WINAPI Nui_ProcessThread(LPVOID pParam);

	// NUI and draw stuff
	DrawDevice			m_DrawDepth;
	DrawDevice			m_DrawVideo;

	// thread handling
	HANDLE				m_hThNuiProcess;
	HANDLE				m_hEvNuiProcessStop;

	HANDLE				m_hNextDepthFrameEvent;
	HANDLE				m_hNextVideoFrameEvent;
	HANDLE				m_hNextSkeletonEvent;
	HANDLE				m_pDepthStreamHandle;
	HANDLE				m_pVideoStreamHandle;
	HFONT				m_hFontFPS;
	HPEN				m_Pen[6];
	HDC					m_SkeletonDC;
	HBITMAP				m_SkeletonBMP;
	HGDIOBJ				m_SkeletonOldObj;
	int unsigned		m_PensTotal;
	POINT				m_Points[NUI_JOINT_COUNT];
	RGBQUAD				m_rgbWk[320*240];
	int					m_LastSkeletonFoundTime;
	bool				m_bScreenBlanked;
	int unsigned		m_FramesTotal;
	int unsigned		m_LastFPStime;
	int unsigned		m_LastFramesTotal;

};

int MessageBoxResource( HWND hwnd, UINT nID, UINT nType );

