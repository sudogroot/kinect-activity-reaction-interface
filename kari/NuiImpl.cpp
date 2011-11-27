/************************************************************************
*																		*
*	NuiImpl.cpp -- Implementation of CSkeletalViewerApp methods dealing *
*				  with NUI processing								  *
*																		*
* Copyright (c) Microsoft Corp. All rights reserved.					*
*																		*
* This code is licensed under the terms of the						  *
* Microsoft Kinect for Windows SDK (Beta) from Microsoft Research		*
* License Agreement: http://research.microsoft.com/KinectSDK-ToU		*
*																		*
************************************************************************/

#define _CRT_SECURE_NO_WARNING

#include "stdafx.h"

#include <map>
#include <iostream>
#include <vector>
#include <tchar.h>

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <math.h>
#define PI 3.1415926535897932384

#include "SkeletalViewer.h"
#include "resource.h"
#include <mmsystem.h>

#include <stdio.h>
#include <string>
#include <sstream>

#include <windows.h>
#include <process.h>
#include <cstring>

#include <fstream>
#include <Commdlg.h>


#include "ConStream.h"
ConStream m_Log;

static enum bodyActions {
	BA_LEAN_LEFT,				//	lean_left					inches body lean left
	BA_LEAN_RIGHT,				//	lean_right					inches body lean right
	BA_LEAN_FORWARD,			//	lean_forwards				inches body lean forward
	BA_LEAN_BACKWARD,			//	lean_backwards				inches body lean back
	BA_TURN_LEFT,				//	turn_left					angular amount of left body turn (degrees)
	BA_TURN_RIGHT,				//	turn_right					angular amount of right body turn(degrees)
	BA_HAND_LEFT_EXPAND,				//								left hand action. like expand or clench
	BA_HAND_LEFT_FORWARD,		//	left_arm_forwards			forward distance from left hand to shoulder (inches)
	BA_HAND_LEFT_DOWN,			//	left_arm_down				downward distance from left hand to shoulder (inches)
	BA_HAND_LEFT_UP,			//	left_arm_up					upward distance from left hand to shoulder (inches)
	BA_HAND_LEFT_OUT,			//	left_arm_out				sideways distance from left hand to shoulder (inches)
	BA_HAND_LEFT_ACROSS,		//	left_arm_across				sideways distance from left hand across body to shoulder (inches)
	BA_HAND_LEFT_BACKWARD,
	BA_HAND_RIGHT_EXPAND,				//								right hand action. like expand or clench
	BA_HAND_RIGHT_FORWARD,		//	right_arm_forwards			forward distance from right hand to shoulder (inches)
	BA_HAND_RIGHT_DOWN,			//	right_arm_down				downward distance from right hand to shoulder (inches)
	BA_HAND_RIGHT_UP,			//	right_arm_up				upward distance from right hand to shoulder (inches)
	BA_HAND_RIGHT_OUT,			//	right_arm_out				sideways distance from right hand to shoulder (inches)
	BA_HAND_RIGHT_ACROSS,		//	right_arm_across			sideways distance from right hand across body to shoulder (inches)
	BA_HAND_RIGHT_BACKWARD,
	BA_FOOT_LEFT_FORWARD,		//	left_foot_forwards			forward distance from left hip to foot (inches)
	BA_FOOT_LEFT_OUT,			//	left_foot_sideways			sideways distance from left hip to foot (inches)
	BA_FOOT_LEFT_BACKWARD,		//	left_foot_backwards			backwards distance from left hip to foot (inches)
	BA_FOOT_LEFT_UP,			//	left_foot_up				height of left foot above other foot on ground (inches)
	BA_FOOT_LEFT_ACROSS,		//	right foot acro
	BA_FOOT_RIGHT_FORWARD,		//	right_foot_forwards			forward distance from right hip to foot (inches)
	BA_FOOT_RIGHT_OUT,			//	right_foot_sideways			sideways distance from right hip to foot (inches)
	BA_FOOT_RIGHT_BACKWARD,		//	right_foot_backwards		backwards distance from right hip to foot (inches)
	BA_FOOT_RIGHT_UP,			//	right_foot_up				height of right foot above other foot on ground (inches)
	BA_FOOT_RIGHT_ACROSS,		//	right foot acro
	BA_JUMP,					//	jump						height of both feet above ground (inches)
	BA_CROUCH,					//	crouch						crouch distance, calculated as current height subtracted from standing height (inches)
	BA_WALK						//	walk						height of each step above ground when walking in place (inches)
};


static enum bodyParts {
	BP_NECK,
	BP_TORSO_UPPER,
	BP_TORSO_LOWER,
	BP_ARM_UPPER_LEFT,
	BP_ARM_UPPER_RIGHT,
	BP_ARM_FOR_LEFT,
	BP_ARM_FOR_RIGHT,
	BP_SHOULDER_LEFT,
	BP_SHOULDER_RIGHT,
	BP_HIP_LEFT,
	BP_HIP_RIGHT,
	BP_TIBIA_LEFT,
	BP_TIBIA_RIGHT,
	BP_TARSUS_LEFT,
	BP_TARSUS_RIGHT
};

HWND textWindowHwnd;

static std::map<int unsigned, std::string>	s_bodyActions;
static std::map<std::string, int unsigned>	bodyActionStrings;

static enum actionSetParts {
	AS_REQUIRE,
	AS_EXECUTE,
	AS_BODY_ACTION,

	AS_USE_DISTANCE,
	AS_DISTANCE_MIN,
	AS_DISTANCE_MAX,

	AS_USE_ANGLE,
	AS_ANGLE_MIN,
	AS_ANGLE_MAX,
	AS_ANGLE_MIN_X,
	AS_ANGLE_MAX_X,
	AS_ANGLE_MIN_Y,
	AS_ANGLE_MAX_Y,

	AS_KEY_TAP,
	AS_KEY_HOLD,
	AS_KEY_PRESS,
	AS_KEY_RELEASE,

	AS_MOUSE_TRACK,
	AS_MOUSE_TAP,
	AS_MOUSE_HOLD,

	AS_WINDOW_HOLD,
	AS_CONFIG_LOAD,

	AS_WINDOW_SHOW,

	AS_JOINT_PREV_X,
	AS_JOINT_PREV_Y,
	AS_JOINT_PREV_ANGLE_X,
	AS_JOINT_PREV_ANGLE_Y

};

static enum actionCommandValues {
	ACV_MOUSE_LEFT = 1,
	ACV_MOUSE_RIGHT,

	ACV_ABSOLUTE,
	ACV_RELATIVE,
	ACV_DRAG,
	ACV_PUSH,

	ACV_MAXIMIZE,
	ACV_MINIMIZE,
	ACV_RESTORE,

	ACV_HAND_CLENCH,
	ACV_HAND_EXPAND
};


static std::map<std::string, int unsigned> actionPartStrings;
static std::map<std::string, int unsigned> actionCommandStrings;
static std::map<int unsigned, std::string> configFiles;

static std::map<int unsigned, float>						bodyPartLengths;
static std::map<int unsigned, map<int unsigned, float > >	bodyPartLengthFrames;

std::map<
	int unsigned,
	std::map<
		int unsigned,
		std::map<
			int unsigned,
			std::map<
				int unsigned,
				int signed
			>
		>
	>
> actionSets;

std::map<int unsigned, bool> actionSetsStatus;

NUI_SKELETON_FRAME SkeletonFrame;
NUI_SKELETON_FRAME previousSkeltonFrame;

int unsigned personHeight;
int unsigned actionsCount;

long	cameraAngleTest		= 0;
long	cameraAnglePrev		= 0;
float	cameraAngleAverage	= 0;
long	cameraAngleTotal	= 0;
int		cameraAngleCount	= 0;

//static DWORD actionExecuteThreads[];
vector<HANDLE> actionExecuteThreads;



int signed getSkeletonElevation( NUI_SKELETON_DATA * &pSkel ) {
	Vector4& c = SkeletonFrame.vFloorClipPlane;
	Vector4& j = pSkel->joints[ NUI_JOINT_FOOT_LEFT ].y < pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].y
		? pSkel->joints[ NUI_JOINT_FOOT_LEFT ]
		: pSkel->joints[ NUI_JOINT_FOOT_RIGHT ]
	;
	return (int signed)( ( j.x * c.x + j.y * c.y + j.z * c.z + c.w ) / 0.0254 );
}



/*/
t = current time
b = start value
c = change in value
d = duration
/**/
float expoEaseInOut( float t, float b, float c, float d ) {
	if(t==0) return b;
	if(t==d) return b+c;
	if((t/=d/2) < 1) return c/2 * pow(2, 10 * (t - 1)) + b;
	return c/2 * (-pow(2, -10 * --t) + 2) + b;
}

float cubicEaseInOut( float t, float b, float c, float d ) {
	if ((t/=d/2) < 1) return c/2*t*t*t + b;
	return c/2*((t-=2)*t*t + 2) + b;
}

float quadEaseInOut( float t, float b, float c, float d ) {
	if ((t/=d/2) < 1) return ((c/2)*(t*t)) + b;
	return -c/2 * (((t-2)*(--t)) - 1) + b;
}

float circEaseInOut(float t,float b , float c, float d) {
	if ((t/=d/2) < 1) return -c/2 * (sqrt(1 - t*t) - 1) + b;
	return c/2 * (sqrt(1 - t*(t-=2)) + 1) + b;
}

float sineEaseInOut(float t,float b , float c, float d) {
	return -c/2 * (cos(PI*t/d) - 1) + b;
}


COLORREF interpolate( COLORREF c1, COLORREF c2, float normalized_value ) {
	if( normalized_value <= 0.0 ){ return c1; }
	if( normalized_value >= 1.0 ){ return c2; }

	return RGB(
		(int unsigned)((1.0-normalized_value) * ((c1 >> 16)	& 0xFF ) + normalized_value * ((c2 >> 16)	& 0xFF )	),
		(int unsigned)((1.0-normalized_value) * ((c1 >> 8)	& 0xFF ) + normalized_value * ((c2 >> 8	)	& 0xFF )	),
		(int unsigned)((1.0-normalized_value) * ( c1		& 0xFF ) + normalized_value * (	c2			& 0xFF )	)
    );
}


void AppendTextToEditCtrl( HWND hWndEdit, LPCTSTR pszText ) {
   int nLength = GetWindowTextLength( hWndEdit );
   SendMessage( hWndEdit, EM_SETSEL, (WPARAM)nLength, (LPARAM)nLength );
   SendMessage( hWndEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)pszText );
}

void TypeStr( char *lpszString ) {
	char cChar;
	// loops through char
	while( ( cChar =* lpszString++ ) ) {
		// keycode of char
		short vk = VkKeyScan( cChar );
		// hold shift if necessary
		if( ( vk >> 8 ) & 1 ) {
			keybd_event( VK_LSHIFT, 0, 0, 0 );
		}
		// key in
		keybd_event( (unsigned char)vk, 0x1e, 0, 0 );
		Sleep( 100 );
		// key out
		keybd_event( (unsigned char)vk, 0x1e, KEYEVENTF_KEYUP, 0 );
		// release shift if necessary
		if( ( vk >> 8 ) & 1 ) {
			keybd_event( VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0 );
		}
	}
}

void keyTap( WORD vk ) {
	UINT VKCode		= LOBYTE( vk );
	UINT ScanCode	= MapVirtualKey( VKCode, 0 );
	m_Log << "key tap : " << vk << " : " << ScanCode << "\n\n";
	keybd_event( (unsigned char)vk, ScanCode, 0, 0 ); // key in
	Sleep( 100 );
	keybd_event( (unsigned char)vk, ScanCode, KEYEVENTF_KEYUP, 0 );
}

/*/
void keyPress( WORD wVk ) {
	INPUT input[1];
	input[0].type			= INPUT_KEYBOARD;
    input[0].ki.wVk			= wVk;
    input[0].ki.dwFlags		= KEYEVENTF_UNICODE;
    input[0].ki.time		= 0;
    input[0].ki.dwExtraInfo	= 0;
	SendInput( 1, input, sizeof( input ) );
/*/
void keyPress( WORD vk ) {
	UINT VKCode		= LOBYTE( vk );
	UINT ScanCode	= MapVirtualKey( VKCode, 0 );
	m_Log << "key press : " << vk << " : " << ScanCode << "\n\n";
	keybd_event( (unsigned char)vk, ScanCode, 0, 0 );
/**/
}

void keyRelease( WORD vk ) {
	UINT VKCode		= LOBYTE( vk );
	UINT ScanCode	= MapVirtualKey( VKCode, 0 );
	m_Log << "key release : " << vk << " : " << ScanCode << "\n\n";
	keybd_event( (unsigned char)vk, ScanCode, KEYEVENTF_KEYUP, 0 );
}


static const COLORREF g_SkeletonColors[ 6 ] = {
	RGB(	40,		40,		40	),
	RGB(	50,		50,		50	),
	RGB(	60,		60,		60	),
	RGB(	70,		70,		70	),
	RGB(	80,		80,		80	),
	RGB(	90,		90,		90	)
};

static const COLORREF g_JointColorTable[ NUI_JOINT_COUNT ] = {
	RGB( 255, 255,   0 ),	// NUI_JOINT_HIP_CENTER
	RGB( 255, 255,   0 ),	// NUI_JOINT_SPINE
	RGB( 255, 255,   0 ),	// NUI_JOINT_SHOULDER_CENTER
	RGB( 255, 255,   0 ),	// NUI_JOINT_HEAD
	RGB( 255, 255,   0 ),	// NUI_JOINT_SHOULDER_LEFT
	RGB( 255, 255,   0 ),	// NUI_JOINT_ELBOW_LEFT
	RGB( 255, 255,   0 ),	// NUI_JOINT_WRIST_LEFT
	RGB( 255, 255,   0 ),	// NUI_JOINT_HAND_LEFT
	RGB( 255, 255,   0 ),	// NUI_JOINT_SHOULDER_RIGHT
	RGB( 255, 255,   0 ),	// NUI_JOINT_ELBOW_RIGHT
	RGB( 255, 255,   0 ),	// NUI_JOINT_WRIST_RIGHT
	RGB( 255, 255,   0 ),	// NUI_JOINT_HAND_RIGHT
	RGB( 255, 255,   0 ),	// NUI_JOINT_HIP_LEFT
	RGB( 255, 255,   0 ),	// NUI_JOINT_KNEE_LEFT
	RGB( 255, 255,   0 ),	// NUI_JOINT_ANKLE_LEFT
	RGB( 255, 255,   0 ),	// NUI_JOINT_FOOT_LEFT
	RGB( 255, 255,   0 ),	// NUI_JOINT_HIP_RIGHT
	RGB( 255, 255,   0 ),	// NUI_JOINT_KNEE_RIGHT
	RGB( 255, 255,   0 ),	// NUI_JOINT_ANKLE_RIGHT
	RGB( 255, 255,   0 )	// NUI_JOINT_FOOT_RIGHT
};

static float g_JointColorPower[ NUI_JOINT_COUNT ] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void CKARIApp::Nui_Zero() {
	m_hNextDepthFrameEvent	= NULL;
	m_hNextVideoFrameEvent	= NULL;
	m_hNextSkeletonEvent	= NULL;
	m_pDepthStreamHandle	= NULL;
	m_pVideoStreamHandle	= NULL;
	m_hThNuiProcess			= NULL;
	m_hEvNuiProcessStop		= NULL;
	m_SkeletonDC			= NULL;
	m_SkeletonBMP			= NULL;
	m_SkeletonOldObj		= NULL;
	m_PensTotal				= 6;
	m_LastSkeletonFoundTime	= -1;
	m_bScreenBlanked		= false;
	m_FramesTotal			= 0;
	m_LastFPStime			= -1;
	m_LastFramesTotal		= 0;
	ZeroMemory( m_Pen,		sizeof( m_Pen )		);
	ZeroMemory( m_Points,	sizeof( m_Points )	);
}
/*/
bool valueExists( string search, static std::map<int, std::string> list, int &slotNum ) {
	for( int unsigned i = 0; i <= list.size(); i++ ) {
		if( search == list[i] ) {
			slotNum = i;
			return true;
		}
	}
	return false;
}
/**/


//LPWSTR ConvertToLPWSTR( const std::string& s ) {
string ConvertToLPWSTR( const std::string& s ) {
	return s;
	/*/
	LPWSTR ws = new wchar_t[ s.size() + 1 ]; // +1 for zero at the end
	copy( s.begin(), s.end(), ws );
	ws[ s.size() ] = 0; // zero at the end
	return ws;
	/**/
}

void saveCameraAngle() {
	switch( ::NuiCameraElevationGetAngle( &cameraAngleTest ) ) {
		case E_POINTER:
		case E_NUI_DEVICE_NOT_READY:
		break;
		default:
			if( cameraAngleTest != 0 && cameraAngleTest != cameraAnglePrev ) {
				cameraAngleCount++;
				cameraAngleTotal	+=	cameraAngleTest;
				cameraAngleAverage	=	cameraAngleTotal / cameraAngleCount;
				cameraAnglePrev		=	cameraAngleTest;
			}
	}
}

void mouseMove( int unsigned dx, int unsigned dy ) {
	INPUT a = {};
	a.type = INPUT_MOUSE;
	a.mi.dx = dx;
	a.mi.dy = dy;
	a.mi.mouseData = 0;
	a.mi.dwFlags = MOUSEEVENTF_MOVE;
	a.mi.time = 0;
	a.mi.dwExtraInfo = 0;
	::SendInput( 1, &a, sizeof( a ) );
}

void moveActiveWindow( int unsigned x, int unsigned y ) {
	HWND hWnd = GetForegroundWindow();
	RECT rect;
	GetWindowRect( hWnd, &rect );
	SetWindowPos(
		hWnd,
		NULL,
		rect.left + x,
		rect.top + y,
		0,
		0,
		SWP_NOSIZE
	);
}

void placeActiveWindow( int unsigned x, int unsigned y ) {
	SetWindowPos(
		GetForegroundWindow(),
		NULL,
		x,
		y,
		0,
		0,
		SWP_NOSIZE
	);
}

void loadConfig( string configFileName ) {

	int actionSetNum		= -1;
	int actionRequireNum	= -1;
	int actionExecuteNum	= -1;
	int actionMode		= -1;
	int actionType		= -1;

	actionSets.clear();
	actionSetsStatus.clear();

	configFileName = configFileName.substr( 0, configFileName.size() );
	m_Log << "load file: [" << configFileName << "]" << "\n";

	std::ifstream file;
	file.open( configFileName );

	int unsigned command;

	configFiles.clear();
	configFiles[0] = "";

	std::string
		line,
		textOut;

	int unsigned lNum = 0;
	while( std::getline( file, line ) ) {

		lNum++;

		if( line.empty() ) {
			continue;
		}

		size_t startpos	= line.find_first_not_of(" \t\r\n");	// Find the first character position after excluding leading blank space
		size_t endpos	= line.find_last_not_of(" \t\r\n");		// Find the first character position from reverse af

		if( ( string::npos == startpos ) || ( string::npos == endpos ) ) {
			continue;
		}

		line = line.substr( startpos, endpos - startpos + 1 );

		if( line.substr( 0, 1 ) == "#" ) {
			continue;
		}

		if( line.substr( 0, 1 ) == "[" && line.substr( line.length() - 1, 1 ) == "]" ) {

			string sectionName = line.substr( 1, line.length() - 2 );

			if( sectionName == "set" ) {
				actionSetNum++;
				actionRequireNum					= -1;
				actionExecuteNum					= -1;
				AppendTextToEditCtrl( textWindowHwnd, "set\n" );
			} else if( sectionName == "require" ) {
				actionRequireNum++;
				actionMode = AS_REQUIRE;
				AppendTextToEditCtrl( textWindowHwnd, "\trequire\n" );
			} else if( sectionName == "execute" ) {
				actionExecuteNum++;
				actionMode = AS_EXECUTE;
				AppendTextToEditCtrl( textWindowHwnd, "\texecute\n" );
			}

		} else {

			line.erase( 0, line.find_first_not_of("\t ") );
			size_t sepPos = line.find("=");

			std::string key, value;

			key = line.substr( 0, sepPos );
			if( key.find('\t') != line.npos || key.find(' ') != line.npos )
				key.erase( key.find_first_of("\t ") );

			value = line.substr( sepPos + 1 );
			value.erase( 0, value.find_first_not_of("\t ") );
			value.erase( value.find_last_not_of("\t ") + 1 );

			actionType = actionPartStrings[ key ];

			command = 0;

			switch( actionType ) {
				case AS_CONFIG_LOAD:
					//int slotNum;
					//if( !valueExists( value, configFiles, slotNum ) ) {
						configFiles[ configFiles.size() ] = value;
					//}
					command = configFiles.size() - 1;
					textOut = "\t\tconfig load : ";
					textOut += value;
					textOut += "\n";
					AppendTextToEditCtrl( textWindowHwnd, textOut.c_str() );
				break;
				case AS_WINDOW_HOLD:
				case AS_MOUSE_TRACK:
					actionSets[ actionSetNum ][ actionMode ][ actionRequireNum ][ AS_USE_DISTANCE ]	= true;
					actionSets[ actionSetNum ][ actionMode ][ actionRequireNum ][ AS_USE_ANGLE ]	= true;
					command = actionCommandStrings[ value ];
					textOut = "\t\twindow/mouse : ";
					textOut += value;
					textOut += "\n";
					AppendTextToEditCtrl( textWindowHwnd, textOut.c_str() );
				break;
				case AS_BODY_ACTION:
					command = bodyActionStrings[ value ];
					textOut = "\t\tbody action : ";
					textOut += value;
					textOut += "\n";
					AppendTextToEditCtrl( textWindowHwnd, textOut.c_str() );
				break;
				case AS_USE_DISTANCE:
				case AS_DISTANCE_MIN:
				case AS_DISTANCE_MAX:
					actionSets[ actionSetNum ][ actionMode ][ actionRequireNum ][ AS_USE_DISTANCE ]	= true;
					istringstream( value ) >> command;
					textOut = "\t\tdistance : ";
					textOut += value;
					textOut += "\n";
					AppendTextToEditCtrl( textWindowHwnd, textOut.c_str() );
				break;
				case AS_USE_ANGLE:
				case AS_ANGLE_MIN:
				case AS_ANGLE_MAX:
				case AS_ANGLE_MIN_X:
				case AS_ANGLE_MAX_X:
				case AS_ANGLE_MIN_Y:
				case AS_ANGLE_MAX_Y:
					actionSets[ actionSetNum ][ actionMode ][ actionRequireNum ][ AS_USE_ANGLE ]	= true;
					istringstream( value ) >> command;
					textOut = "\t\tangle : ";
					textOut += value;
					textOut += "\n";
					AppendTextToEditCtrl( textWindowHwnd, textOut.c_str() );
				break;
				case AS_KEY_TAP:
				case AS_KEY_HOLD:
				case AS_KEY_PRESS:
				case AS_KEY_RELEASE:

						 if( value == "back" )			command = VK_BACK;
					else if( value == "tab" )			command = VK_TAB;
					else if( value == "return" )		command = VK_RETURN;
					else if( value == "shift" )			command = VK_SHIFT;
					else if( value == "control" )		command = VK_CONTROL;
					else if( value == "menu" )			command = VK_MENU;
					else if( value == "pause" )			command = VK_PAUSE;
					else if( value == "capslock" )		command = VK_CAPITAL;
					else if( value == "escape" )		command = VK_ESCAPE;
					else if( value == "space" )			command = VK_SPACE;
					else if( value == "end" )			command = VK_END;
					else if( value == "home" )			command = VK_HOME;
					else if( value == "arrow_left" )	command = VK_LEFT;
					else if( value == "arrow_up" )		command = VK_UP;
					else if( value == "arrow_right" )	command = VK_RIGHT;
					else if( value == "arrow_down" )	command = VK_DOWN;
					else if( value == "print" )			command = VK_PRINT;
					else if( value == "snapshot" )		command = VK_SNAPSHOT;
					else if( value == "insert" )		command = VK_INSERT;
					else if( value == "delete" )		command = VK_DELETE;
					else if( value == "win_left" )		command = VK_LWIN;
					else if( value == "win_right" )		command = VK_RWIN;
					else if( value == "numpad_0" )		command = VK_NUMPAD0;
					else if( value == "numpad_1" )		command = VK_NUMPAD1;
					else if( value == "numpad_2" )		command = VK_NUMPAD2;
					else if( value == "numpad_3" )		command = VK_NUMPAD3;
					else if( value == "numpad_4" )		command = VK_NUMPAD4;
					else if( value == "numpad_5" )		command = VK_NUMPAD5;
					else if( value == "numpad_6" )		command = VK_NUMPAD6;
					else if( value == "numpad_7" )		command = VK_NUMPAD7;
					else if( value == "numpad_8" )		command = VK_NUMPAD8;
					else if( value == "numpad_9" )		command = VK_NUMPAD9;
					else if( value == "multiply" )		command = VK_MULTIPLY;
					else if( value == "add" )			command = VK_ADD;
					else if( value == "separator" )		command = VK_SEPARATOR;
					else if( value == "subtract" )		command = VK_SUBTRACT;
					else if( value == "decimal" )		command = VK_DECIMAL;
					else if( value == "divide" )		command = VK_DIVIDE;
					else if( value == "fkey_1" )		command = VK_F1;
					else if( value == "fkey_2" )		command = VK_F2;
					else if( value == "fkey_3" )		command = VK_F3;
					else if( value == "fkey_4" )		command = VK_F4;
					else if( value == "fkey_5" )		command = VK_F5;
					else if( value == "fkey_6" )		command = VK_F6;
					else if( value == "fkey_7" )		command = VK_F7;
					else if( value == "fkey_8" )		command = VK_F8;
					else if( value == "fkey_9" )		command = VK_F9;
					else if( value == "fkey_10" )		command = VK_F10;
					else if( value == "fkey_11" )		command = VK_F11;
					else if( value == "fkey_12" )		command = VK_F12;
					else if( value == "numlock" )		command = VK_NUMLOCK;
					else if( value == "scroll" )		command = VK_SCROLL;
					else if( value == "shift_left" )	command = VK_LSHIFT;
					else if( value == "shift_right" )	command = VK_RSHIFT;
					else if( value == "control_left" )	command = VK_LCONTROL;
					else if( value == "control_right" )	command = VK_RCONTROL;
					else if( value == "menu_left" )		command = VK_LMENU;
					else if( value == "menu_right" )	command = VK_RMENU;
					else								command = VkKeyScan( (WCHAR)value[0] );

					textOut = "\t\tkey : ";
					textOut += value;
					textOut += "\n";
					AppendTextToEditCtrl( textWindowHwnd, textOut.c_str() );
				break;
				case AS_MOUSE_TAP:
				case AS_MOUSE_HOLD:
					command = actionCommandStrings[ value ];
					textOut = "\t\tmouse tap/hold : ";
					textOut += value;
					textOut += "\n";
					AppendTextToEditCtrl( textWindowHwnd, textOut.c_str() );
				break;
				case AS_WINDOW_SHOW:
					command = actionCommandStrings[ value ];
				break;
				default:
					m_Log << "\nbad action, line # " << lNum << "\n";
					m_Log << "\t" << line << "\n";
					continue;
			}

			if( actionSetNum < 0 || actionMode < 0 || actionRequireNum < 0 || actionType < 0 ) {
				m_Log << "\nbad action, line # " << lNum << "\n";
			} else {
				switch( actionMode ) {
					case AS_REQUIRE:	actionSets[ actionSetNum ][ actionMode ][ actionRequireNum ][ actionType ]	= command;	break;
					case AS_EXECUTE:	actionSets[ actionSetNum ][ actionMode ][ actionExecuteNum ][ actionType ]	= command;	break;
				}
			}

		}


	}

	file.close();

	actionsCount = actionSetNum + 1;

}



void userInputConfig( HWND & m_hWnd ) {

	/**/
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name

	// Initialize OPENFILENAME
	ZeroMemory( &ofn, sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hwndOwner		= m_hWnd;
	ofn.lpstrFile		= szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0]	= '\0';
	ofn.nMaxFile		= sizeof( szFile );
	ofn.lpstrFilter		= "Config\0*.CFG\0Text\0*.TXT\0All\0*.*\0";
	ofn.nFilterIndex	= 1;
	ofn.lpstrFileTitle	= NULL;
	ofn.nMaxFileTitle	= 0;
	ofn.lpstrInitialDir	= NULL;
	ofn.Flags			= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box.

	if( GetOpenFileName( &ofn ) == true ) {
		loadConfig( ofn.lpstrFile );
	}
	/**/

}


int screenSizeX;
int screenSizeY;


NUI_TRANSFORM_SMOOTH_PARAMETERS mySmooth;

struct testStruct {
	int										distance		;
	std::map<int unsigned, int unsigned>	curBodyParts	;
};

DWORD WINAPI CKARIApp::testThread( void * argsInput ) {
	m_Log << "1";
	testStruct * args = (testStruct*) argsInput;
	m_Log << "2";	
	int distance											= args->distance;
	m_Log << "3";
	std::map<int unsigned, int unsigned>	curBodyParts	= args->curBodyParts;
	m_Log << "4[" << curBodyParts.size() << "]";
	return (0);
}

DWORD WINAPI CKARIApp::testStart( LPVOID pParam ) {

	m_Log << "a";
	
	int unsigned							distance;
	std::map<int unsigned, int unsigned>	curBodyParts;

	m_Log << "b";
	if( 1 ) {
		m_Log << "c";
	
		distance			= 1;
		curBodyParts[ 1 ]	= 2;

		m_Log << "d";
	
		struct testStruct ps = { distance, curBodyParts };
		m_Log << "e";
		CreateThread( NULL, 0, testThread, (void*)&ps, 0, NULL );
//		_beginthread( testThread, 0, (void*)&ps );
		m_Log << "f";
	
	}
	return (0);

}


HRESULT CKARIApp::Nui_Init( LPTSTR commandLineString ) {
	
	m_Log.Open();
	
	mySmooth.fSmoothing = 0.5f;
	mySmooth.fCorrection = 0.0f;
	mySmooth.fPrediction = 5.0f;
	mySmooth.fJitterRadius = 0.02f;
	mySmooth.fMaxDeviationRadius = 0.1f;


	HRESULT				hr;
	RECT				rc;

	screenSizeX = GetSystemMetrics( SM_CXSCREEN );
	screenSizeY = GetSystemMetrics( SM_CYSCREEN );

	textWindowHwnd = CreateWindow(
		TEXT("edit"),
		TEXT(""),
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | WS_EX_RIGHTSCROLLBAR | ES_WANTRETURN | WS_VSCROLL,
		0, 510, 480, 120,
		m_hWnd,
		(HMENU) 1,
		NULL,
		NULL
	);

	HFONT hfffont = CreateFont(
		12,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "Verdana"
	);

	SendMessage( textWindowHwnd, WM_SETFONT, (DWORD)hfffont, 0 );


	actionPartStrings["require"]		= AS_REQUIRE;
	actionPartStrings["execute"]		= AS_EXECUTE;
	actionPartStrings["bodyAction"]		= AS_BODY_ACTION;
	actionPartStrings["useDistance"]	= AS_USE_DISTANCE;
	actionPartStrings["distanceMin"]	= AS_DISTANCE_MIN;
	actionPartStrings["distanceMax"]	= AS_DISTANCE_MAX;
	actionPartStrings["useAngle"]		= AS_USE_ANGLE;
	actionPartStrings["angleMin"]		= AS_ANGLE_MIN;
	actionPartStrings["angleMax"]		= AS_ANGLE_MAX;
	actionPartStrings["angleMinX"]		= AS_ANGLE_MIN_X;
	actionPartStrings["angleMaxX"]		= AS_ANGLE_MAX_X;
	actionPartStrings["angleMinY"]		= AS_ANGLE_MIN_Y;
	actionPartStrings["angleMaxY"]		= AS_ANGLE_MAX_Y;
	actionPartStrings["keyTap"]			= AS_KEY_TAP;
	actionPartStrings["keyHold"]		= AS_KEY_HOLD;
	actionPartStrings["keyPress"]		= AS_KEY_PRESS;
	actionPartStrings["keyRelease"]		= AS_KEY_RELEASE;
	actionPartStrings["mouseTrack"]		= AS_MOUSE_TRACK;
	actionPartStrings["mouseTap"]		= AS_MOUSE_TAP;
	actionPartStrings["mouseHold"]		= AS_MOUSE_HOLD;
	actionPartStrings["windowHold"]		= AS_WINDOW_HOLD;
	actionPartStrings["configLoad"]		= AS_CONFIG_LOAD;
	actionPartStrings["windowShow"]		= AS_WINDOW_SHOW;



	bodyActionStrings["leanLeft"]			=	BA_LEAN_LEFT;
	bodyActionStrings["leanRight"]			=	BA_LEAN_RIGHT;
	bodyActionStrings["leanForward"]		=	BA_LEAN_FORWARD;
	bodyActionStrings["leanBackward"]		=	BA_LEAN_BACKWARD;

	bodyActionStrings["turnLeft"]			=	BA_TURN_LEFT;
	bodyActionStrings["turnRight"]			=	BA_TURN_RIGHT;

	bodyActionStrings["handLeftExpand"]		=	BA_HAND_LEFT_EXPAND;
	bodyActionStrings["handLeftForward"]	=	BA_HAND_LEFT_FORWARD;
	bodyActionStrings["handLeftDown"]		=	BA_HAND_LEFT_DOWN;
	bodyActionStrings["handLeftUp"]			=	BA_HAND_LEFT_UP;
	bodyActionStrings["handLeftOut"]		=	BA_HAND_LEFT_OUT;
	bodyActionStrings["handLeftAcross"]		=	BA_HAND_LEFT_ACROSS;
	bodyActionStrings["handLeftBackward"]	=	BA_HAND_LEFT_BACKWARD;

	bodyActionStrings["handRightExpand"]	=	BA_HAND_RIGHT_EXPAND;
	bodyActionStrings["handRightForward"]	=	BA_HAND_RIGHT_FORWARD;
	bodyActionStrings["handRightDown"]		=	BA_HAND_RIGHT_DOWN;
	bodyActionStrings["handRightUp"]		=	BA_HAND_RIGHT_UP;
	bodyActionStrings["handRightOut"]		=	BA_HAND_RIGHT_OUT;
	bodyActionStrings["handRightAcross"]	=	BA_HAND_RIGHT_ACROSS;
	bodyActionStrings["handRightBackward"]	=	BA_HAND_RIGHT_BACKWARD;

	bodyActionStrings["footLeftForward"]	=	BA_FOOT_LEFT_FORWARD;
	bodyActionStrings["footLeftOut"]		=	BA_FOOT_LEFT_OUT;
	bodyActionStrings["footLeftBackward"]	=	BA_FOOT_LEFT_BACKWARD;
	bodyActionStrings["footLeftUp"]			=	BA_FOOT_LEFT_UP;
	bodyActionStrings["footLeftAcross"]		=	BA_FOOT_LEFT_ACROSS;

	bodyActionStrings["footRightForward"]	=	BA_FOOT_RIGHT_FORWARD;
	bodyActionStrings["footRightOut"]		=	BA_FOOT_RIGHT_OUT;
	bodyActionStrings["footRightBackward"]	=	BA_FOOT_RIGHT_BACKWARD;
	bodyActionStrings["footRightUp"]		=	BA_FOOT_RIGHT_UP;
	bodyActionStrings["footRightAcross"]	=	BA_FOOT_RIGHT_ACROSS;

	bodyActionStrings["jump"]				=	BA_JUMP;
	bodyActionStrings["crouch"]				=	BA_CROUCH;
	bodyActionStrings["walk"]				=	BA_WALK;


	actionCommandStrings["mouseLeft"]		=	ACV_MOUSE_LEFT;
	actionCommandStrings["mouseRight"]		=	ACV_MOUSE_RIGHT;

	actionCommandStrings["absolute"]		=	ACV_ABSOLUTE;
	actionCommandStrings["relative"]		=	ACV_RELATIVE;
	actionCommandStrings["drag"]			=	ACV_DRAG;
	actionCommandStrings["push"]			=	ACV_PUSH;

	actionCommandStrings["maximize"]		=	SW_SHOWMAXIMIZED;
	actionCommandStrings["minimize"]		=	SW_HIDE;
	actionCommandStrings["restore"]			=	SW_SHOWNORMAL;

	actionCommandStrings["handClench"]		=	ACV_HAND_CLENCH;
	actionCommandStrings["handExpand"]		=	ACV_HAND_EXPAND;


	s_bodyActions[ BA_LEAN_LEFT				] = "Lean left";			//	lean_left				angular body lean left (degrees)
	s_bodyActions[ BA_LEAN_RIGHT			] = "Lean right";			//	lean_right				angular body lean right(degrees)
	s_bodyActions[ BA_LEAN_FORWARD			] = "Lean forward";			//	lean_forwards			angualr body lean forwards (degrees)
	s_bodyActions[ BA_LEAN_BACKWARD			] = "Lean backward";		//	lean_backwards			angular body lean back (degrees)
	s_bodyActions[ BA_TURN_LEFT				] = "Turn left";			//	turn_left				angular amount of left body turn (degrees)
	s_bodyActions[ BA_TURN_RIGHT			] = "Turn right";			//	turn_right				angular amount of right body turn(degrees)
	s_bodyActions[ BA_HAND_LEFT_EXPAND		] = "Left hand";			//
	s_bodyActions[ BA_HAND_LEFT_FORWARD		] = "Left hand forward";	//	left_arm_forwards		forward distance from left hand to shoulder (inches)
	s_bodyActions[ BA_HAND_LEFT_DOWN		] = "Left hand down";		//	left_arm_down			downward distance from left hand to shoulder (inches)
	s_bodyActions[ BA_HAND_LEFT_UP			] = "Left hand up";			//	left_arm_up				upward distance from left hand to shoulder (inches)
	s_bodyActions[ BA_HAND_LEFT_OUT			] = "Left hand out";		//	left_arm_out			sideways distance from left hand to shoulder (inches)
	s_bodyActions[ BA_HAND_LEFT_ACROSS		] = "Left hand across";		//	left_arm_across			sideways distance from left hand across body to shoulder (inches)
	s_bodyActions[ BA_HAND_LEFT_BACKWARD	] = "Left hand backward";	//
	s_bodyActions[ BA_HAND_RIGHT_EXPAND		] = "Right hand expand";	//
	s_bodyActions[ BA_HAND_RIGHT_FORWARD	] = "Right hand forward";	//	right_arm_forwards		forward distance from right hand to shoulder (inches)
	s_bodyActions[ BA_HAND_RIGHT_DOWN		] = "Right hand down";		//	right_arm_down			downward distance from right hand to shoulder (inches)
	s_bodyActions[ BA_HAND_RIGHT_UP			] = "Right hand up";		//	right_arm_up			upward distance from right hand to shoulder (inches)
	s_bodyActions[ BA_HAND_RIGHT_OUT		] = "Right hand out";		//	right_arm_out			sideways distance from right hand to shoulder (inches)
	s_bodyActions[ BA_HAND_RIGHT_ACROSS		] = "Right hand across";	//	right_arm_across		sideways distance from right hand across body to shoulder (inches)
	s_bodyActions[ BA_HAND_RIGHT_BACKWARD	] = "Right hand backward";	//
	s_bodyActions[ BA_FOOT_LEFT_FORWARD		] = "Foot left forward";	//	left_foot_forwards		forward distance from left hip to foot (inches)
	s_bodyActions[ BA_FOOT_LEFT_OUT			] = "Foot left out";		//	left_foot_sideways		sideways distance from left hip to foot (inches)
	s_bodyActions[ BA_FOOT_LEFT_BACKWARD	] = "Foot left backward";	//	left_foot_backwards		s_bodyActions[ BAckwards distance from left hip to foot (inches)
	s_bodyActions[ BA_FOOT_LEFT_UP			] = "Foot left up";			//	left_foot_up			height of left foot above other foot on ground (inches)
	s_bodyActions[ BA_FOOT_RIGHT_FORWARD	] = "Foot right forward";	//	right_foot_forwards		forward distance from right hip to foot (inches)
	s_bodyActions[ BA_FOOT_RIGHT_OUT		] = "Foot right out";		//	right_foot_sideways		sideways distance from right hip to foot (inches)
	s_bodyActions[ BA_FOOT_RIGHT_BACKWARD	] = "Foot right backward";	//	right_foot_backwards	s_bodyActions[ BAckwards distance from right hip to foot (inches)
	s_bodyActions[ BA_FOOT_RIGHT_UP			] = "Foot right up";		//	right_foot_up			height of right foot above other foot on ground (inches)
	s_bodyActions[ BA_JUMP					] = "Jump";					//	jump					height of both feet above ground (inches)
	s_bodyActions[ BA_CROUCH				] = "Crouch";				//	crouch					crouch distance, calculated as current height subtracted from standing height (inches)
	s_bodyActions[ BA_WALK					] = "Walk";					//	walk					height of each step above ground when walking in place (inches)


	/*/
	m_Log << "ease : " << sineEaseInOut( .00, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .05, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .10, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .15, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .20, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .25, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .30, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .35, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .40, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .45, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .50, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .55, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .60, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .65, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .70, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .75, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .80, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .85, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .90, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( .95, 1, 60, 1 ) << "\n";
	m_Log << "ease : " << sineEaseInOut( 1.0, 1, 60, 1 ) << "\n";
	/**/

	if( _tcslen( commandLineString ) > 0 ) {
		loadConfig( commandLineString );
	} else {
		userInputConfig( m_hWnd );
	}

	m_hNextDepthFrameEvent	= CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hNextVideoFrameEvent	= CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hNextSkeletonEvent	= CreateEvent( NULL, TRUE, FALSE, NULL );

	GetWindowRect( GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), &rc );
	int width	= rc.right - rc.left;
	int height	= rc.bottom - rc.top;

	HDC hdc = GetDC( GetDlgItem( m_hWnd, IDC_SKELETALVIEW ) );

	m_SkeletonBMP		= CreateCompatibleBitmap( hdc, width, height );
	m_SkeletonDC		= CreateCompatibleDC( hdc );

	::ReleaseDC( GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), hdc );

	m_SkeletonOldObj	= SelectObject( m_SkeletonDC, m_SkeletonBMP );

	/**/
	hr = m_DrawDepth.CreateDevice( GetDlgItem( m_hWnd, IDC_DEPTHVIEWER	) );
	hr = m_DrawDepth.SetVideoType( 320, 240, 320 * 4 );
	/**/

	/*/
	hr = m_DrawVideo.CreateDevice( GetDlgItem( m_hWnd, IDC_VIDEOVIEW	) );
	hr = m_DrawVideo.SetVideoType( 640, 480, 640 * 4 );
	/**/

	hr = NuiInitialize( NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR );
	//hr = NuiInitialize( NUI_INITIALIZE_FLAG_USES_SKELETON );
	hr = NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );

	/*/
	hr = NuiImageStreamOpen( NUI_IMAGE_TYPE_COLOR,					NUI_IMAGE_RESOLUTION_640x480, 0, 2, m_hNextVideoFrameEvent, &m_pVideoStreamHandle );
	/**/
	hr = NuiImageStreamOpen( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,	NUI_IMAGE_RESOLUTION_320x240, 0, 2, m_hNextDepthFrameEvent, &m_pDepthStreamHandle );

	// Start the Nui processing thread
	m_hEvNuiProcessStop	= CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hThNuiProcess		= CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );

	return hr;

}



void CKARIApp::Nui_UnInit( ) {
	::SelectObject( m_SkeletonDC, m_SkeletonOldObj );
	DeleteDC( m_SkeletonDC );
	DeleteObject( m_SkeletonBMP );

	if( m_Pen[0] != NULL ) {
		DeleteObject(m_Pen[0]);
		DeleteObject(m_Pen[1]);
		DeleteObject(m_Pen[2]);
		DeleteObject(m_Pen[3]);
		DeleteObject(m_Pen[4]);
		DeleteObject(m_Pen[5]);
		ZeroMemory( m_Pen, sizeof( m_Pen ) );
	}

	// Stop the Nui processing thread
	if( m_hEvNuiProcessStop != NULL ) {
		// Signal the thread
		SetEvent( m_hEvNuiProcessStop );

		// Wait for thread to stop
		if( m_hThNuiProcess != NULL ) {
			WaitForSingleObject( m_hThNuiProcess, INFINITE );
			CloseHandle( m_hThNuiProcess );
		}
		CloseHandle( m_hEvNuiProcessStop );
	}

	NuiShutdown();
	if( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) ) {
		CloseHandle( m_hNextSkeletonEvent );
		m_hNextSkeletonEvent = NULL;
	}
	if( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) ) {
		CloseHandle( m_hNextDepthFrameEvent );
		m_hNextDepthFrameEvent = NULL;
	}
	if( m_hNextVideoFrameEvent && ( m_hNextVideoFrameEvent != INVALID_HANDLE_VALUE ) ) {
		CloseHandle( m_hNextVideoFrameEvent );
		m_hNextVideoFrameEvent = NULL;
	}
	m_DrawDepth.DestroyDevice( );
	m_DrawVideo.DestroyDevice( );
}



DWORD WINAPI CKARIApp::Nui_ProcessThread( LPVOID pParam ) {

	CKARIApp *pthis = ( CKARIApp * ) pParam;
	HANDLE				hEvents[4];
	int					nEventIdx, t, dt;
	

	// Configure events to be listened on
	hEvents[0] = pthis->m_hEvNuiProcessStop;
	hEvents[1] = pthis->m_hNextDepthFrameEvent;
	hEvents[2] = pthis->m_hNextVideoFrameEvent;
	hEvents[3] = pthis->m_hNextSkeletonEvent;

	// Main thread loop
	while( 1 ) {

		// Wait for an event to be signalled
		nEventIdx = WaitForMultipleObjects(
			sizeof( hEvents ) / sizeof( hEvents[0] ),
			hEvents,
			FALSE,
			100
		);

		// If the stop event, stop looping and exit
		if( nEventIdx == 0 )
			break;
		/**/
		// Perform FPS processing
		t = timeGetTime();
		if( pthis->m_LastFPStime == -1 ) {
			pthis->m_LastFPStime		= t;
			pthis->m_LastFramesTotal	= pthis->m_FramesTotal;
		}

		dt = t - pthis->m_LastFPStime;
		if( dt > 1000 ) {
			pthis->m_LastFPStime		= t;
			int unsigned FrameDelta		= pthis->m_FramesTotal - pthis->m_LastFramesTotal;
			pthis->m_LastFramesTotal	= pthis->m_FramesTotal;
			SetDlgItemInt( pthis->m_hWnd, IDC_FPS, FrameDelta, FALSE );
		}
		/**/
		// Perform skeletal panel blanking
		if( pthis->m_LastSkeletonFoundTime == -1 )
			pthis->m_LastSkeletonFoundTime = t;

		dt = t - pthis->m_LastSkeletonFoundTime;

		if( dt > 250 ) {
			if( !pthis->m_bScreenBlanked ) {
				pthis->Nui_BlankSkeletonScreen( GetDlgItem( pthis->m_hWnd, IDC_SKELETALVIEW ) );
				pthis->m_bScreenBlanked = true;
			}
		}
		/**/

		// Process signal event
		switch( nEventIdx ) {
			case 1:
				pthis->Nui_GotDepthAlert();
				pthis->m_FramesTotal++;
			break;
			case 2:
				pthis->Nui_GotVideoAlert();
			break;
			case 3:

				/*/
	m_Log << "a";
	int unsigned							test_distance;
	std::map<int unsigned, int unsigned>	test_curBodyParts;
	test_distance			= 1;
	test_curBodyParts[ 1 ]	= 2;
	m_Log << "b";
	struct testStruct test_ps = { test_distance, test_curBodyParts };
	m_Log << "c";
	HANDLE test_handle = 0;
	m_Log << "d";
	test_handle = CreateThread( NULL, 0, pthis->Nui_GotSkeletonAlert, (void*)&ps, 0, NULL );
	//test_handle = CreateThread( NULL, 0, testThread, (void*)&test_ps, 0, NULL );
	actionExecuteThreads.push_back( test_handle );
	m_Log << "e[" << actionExecuteThreads.size() << "]";
	vector<HANDLE> tmpHandlers;
	tmpHandlers.clear();
	for( int unsigned cj = 0; cj < actionExecuteThreads.size(); cj++ ) {		
		m_Log << "f";
		DWORD test_exit;
		BOOL test_existResult = GetExitCodeThread( actionExecuteThreads[ cj ], &test_exit );
		m_Log << "g[" << actionExecuteThreads[ cj ] << ":" << test_existResult  << ":" << test_exit << "]" << "]";
		if( actionExecuteThreads[ cj ] != 0 && test_existResult != 0 && test_exit != 0 && test_exit != STILL_ACTIVE ) {
			m_Log << "h";
			CloseHandle( actionExecuteThreads[ cj ] );
			m_Log << "i";			
			//ExitThread(test_exit);
			m_Log << "j";
		} else {
			m_Log << "k";
			tmpHandlers.push_back( actionExecuteThreads[ cj ] );
		}
	}
	actionExecuteThreads.clear();
	actionExecuteThreads = tmpHandlers;
	m_Log << "l[" << actionExecuteThreads.size() << "]\n";
	/**/

				pthis->Nui_GotSkeletonAlert();
			break;
		}


	}

	return (0);
}

void CKARIApp::Nui_GotVideoAlert() {

	const NUI_IMAGE_FRAME * pImageFrame = NULL;

	HRESULT hr = NuiImageStreamGetNextFrame( m_pVideoStreamHandle, 0, &pImageFrame );
	if( FAILED( hr ) ) {
		return;
	}

	NuiImageBuffer * pTexture = pImageFrame->pFrameTexture;
	KINECT_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );
	if( LockedRect.Pitch != 0 ) {
		BYTE * pBuffer = (BYTE*) LockedRect.pBits;
		m_DrawVideo.DrawFrame( (BYTE*) pBuffer );
	} else {
		OutputDebugString( "Buffer length of received texture is bogus\r\n" );
	}

	NuiImageStreamReleaseFrame( m_pVideoStreamHandle, pImageFrame );
}


const NUI_IMAGE_FRAME * pImageFrame_depth = NULL;
NuiImageBuffer * pTexture_depth;


void CKARIApp::Nui_GotDepthAlert() {

	HRESULT hr = NuiImageStreamGetNextFrame( m_pDepthStreamHandle, 0, &pImageFrame_depth );

	if( FAILED( hr ) ) {
		return;
	}

	pTexture_depth = pImageFrame_depth->pFrameTexture;
	/**/
	KINECT_LOCKED_RECT LockedRect;

	pTexture_depth->LockRect( 0, &LockedRect, NULL, 0 );

	if( LockedRect.Pitch != 0 ) {

		BYTE * pBuffer = (BYTE*) LockedRect.pBits;

		// draw the bits to the bitmap
		RGBQUAD * rgbrun	= m_rgbWk;
		USHORT * pBufferRun	= (USHORT*) pBuffer;

		for( int unsigned y = 0; y < 240; y++ ) {
			for( int unsigned x = 0; x < 320; x++ ) {
				RGBQUAD quad = Nui_ShortToQuad_Depth( *pBufferRun );
				*rgbrun = quad;
				pBufferRun++;
				rgbrun++;
			}
		}

		m_DrawDepth.DrawFrame( (BYTE*) m_rgbWk );

	} else {
		OutputDebugString( "Buffer length of received texture is bogus\r\n" );
	}
	/**/
	NuiImageStreamReleaseFrame( m_pDepthStreamHandle, pImageFrame_depth );

}


bool CKARIApp::checkAction_handOpen( int unsigned jointId ) {

	// need to store values into an array and compare against in time to see if snap-open happened

	KINECT_LOCKED_RECT LockedRect;

	pTexture_depth->LockRect( 0, &LockedRect, NULL, 0 );

	if( LockedRect.Pitch != 0 ) {

		float	rightDepth		= 0;
		float	rightDepthMax	= 0;
		bool	found			= false;

		for( int unsigned i = 0; i < NUI_SKELETON_COUNT; i++ ) {
			if( previousSkeltonFrame.SkeletonData[i].trackingState == NUI_SKELETON_TRACKED ) {
				NUI_SKELETON_DATA * prevSkel = &previousSkeltonFrame.SkeletonData[ i ];
				rightDepth		= prevSkel->joints[ jointId ].z;
				rightDepthMax	= rightDepth + ( 0.0254 * 3 );
				found = true;
				break;
			}
		}

		if( !found || rightDepth == 0 ) {
			return false;
		}

		BYTE * pBuffer		= (BYTE*) LockedRect.pBits;
		USHORT * pBufferRun	= (USHORT*) pBuffer;

		rightDepthMax *= 1000;

		int unsigned boxInnerPixels			= floor( ( 1.75 / rightDepth ) * 10 );
		int unsigned boxOuterPixels			= boxInnerPixels * 2;

		int unsigned handOuterDepth			= 0;
		int unsigned handOuterDepthPixels	= 0;
		int unsigned handOuterDepthAvg		= 0;

		int unsigned handInnerDepth			= 0;
		int unsigned handInnerDepthPixels	= 0;
		int unsigned handInnerDepthAvg		= 0;

		int unsigned handInnerBoxTop		= m_Points[ jointId ].x - boxInnerPixels;
		int unsigned handInnerBoxBottom		= m_Points[ jointId ].x + boxInnerPixels;
		int unsigned handInnerBoxLeft		= m_Points[ jointId ].y - boxInnerPixels;
		int unsigned handInnerBoxRight		= m_Points[ jointId ].y + boxInnerPixels;

		int unsigned handOuterBoxTop		= m_Points[ jointId ].x - boxOuterPixels;
		int unsigned handOuterBoxBottom		= m_Points[ jointId ].x + boxOuterPixels;
		int unsigned handOuterBoxLeft		= m_Points[ jointId ].y - boxOuterPixels;
		int unsigned handOuterBoxRight		= m_Points[ jointId ].y + boxOuterPixels;

		float handRangeClose				= rightDepthMax;
		float handRangeFar					= rightDepthMax;

		/*/
		int unsigned yStart	= 0;
		int unsigned yEnd	= 240;
		int unsigned xStart	= 0;
		int unsigned xEnd	= 320;
		/*/
		int unsigned yStart	= 0;//handOuterBoxTop;
		int unsigned yEnd	= handOuterBoxBottom;
		int unsigned xStart	= 0;//handOuterBoxLeft;
		int unsigned xEnd	= 320;//handOuterBoxRight;
		/**/

		//m_Log << "buff : " << pTexture_depth->BufferLen() << "\t" << handInnerBoxTop << "\t" << handInnerBoxBottom << "\t" << handInnerBoxLeft << "\t" << handInnerBoxRight << "\t" << rightDepth << "\n";

		//pBufferRun += ( yStart * 320 );

		int unsigned cycles = 0;
		for( int unsigned y = yStart; y < yEnd; y++ ) {
			//pBufferRun += xStart;
			for( int unsigned x = xStart; x < xEnd; x++ ) {
				cycles++;
				USHORT RealDepth	= ( ( (USHORT)*pBufferRun & 0xfff8 ) >> 3 ); // * 0.001;
				//USHORT Player		= (USHORT)*pBufferRun & 7;
				pBufferRun++;
				if( x > handInnerBoxTop && x < handInnerBoxBottom && y > handInnerBoxLeft && y < handInnerBoxRight ) {
					if(	RealDepth < rightDepthMax ) {
						if( RealDepth < handRangeClose ) {
							handRangeClose = RealDepth;
						}
						handInnerDepthPixels++;
						handInnerDepth		+=	RealDepth;
						handInnerDepthAvg	=	handInnerDepth / handInnerDepthPixels;
					}
				} else if( x > handOuterBoxTop && x < handOuterBoxBottom && y > handOuterBoxLeft && y < handOuterBoxRight ) {
					if(	RealDepth < rightDepthMax ) {
						if( RealDepth < handRangeClose ) {
							handRangeClose = RealDepth;
						}
						handOuterDepthPixels++;
						handOuterDepth		+=	RealDepth;
						handOuterDepthAvg	=	handOuterDepth / handOuterDepthPixels;
					}
				}
				/**/
			}
			//pBufferRun += ( 320 - xEnd );
		}
		int unsigned innerBoxSize = ( ( handInnerBoxBottom - handInnerBoxTop ) * ( handInnerBoxRight - handInnerBoxLeft ) );
		if( handInnerDepthPixels > ( innerBoxSize * .7 ) ) {
			m_Log << "hand valid";
		} else {
			m_Log << "hand invalid";
		}
		m_Log << "\t" << innerBoxSize << "\t" << handInnerDepthPixels << "\t" << handOuterDepthPixels << "\t" << (int)( rightDepthMax - handRangeClose ) << "\n";
	}

	//NuiImageStreamReleaseFrame( m_pDepthStreamHandle, pImageFrame_depth );

	return false;

}



void CKARIApp::Nui_BlankSkeletonScreen( HWND hWnd ) {
	HDC hdc = GetDC( hWnd );
	RECT rct;
	GetClientRect( hWnd, &rct );
	int unsigned width	= rct.right;
	int unsigned height	= rct.bottom;
	PatBlt( hdc, 0, 0, width, height, BLACKNESS );
	ReleaseDC( hWnd, hdc );
}


void CKARIApp::Nui_DrawSkeletonSegment( NUI_SKELETON_DATA * pSkel, int unsigned numJoints, ... ) {
	va_list vl;
	va_start( vl, numJoints );
	POINT segmentPositions[ NUI_JOINT_COUNT ];

	for( int unsigned iJoint = 0; iJoint < numJoints; iJoint++ ) {
		NUI_JOINT_INDEX jointIndex = va_arg( vl, NUI_JOINT_INDEX );
		segmentPositions[iJoint].x = m_Points[jointIndex].x;
		segmentPositions[iJoint].y = m_Points[jointIndex].y;
	}

	Polyline( m_SkeletonDC, segmentPositions, numJoints );

	va_end( vl );
}

float pointsDistance( Vector4 p1, Vector4 p2 ) {
	float dx = p1.x - p2.x;
	float dy = p1.y - p2.y;
	float dz = p1.z - p2.z;
	return sqrt( dx*dx + dy*dy + dz*dz );
}

void setBodyPartLength( NUI_SKELETON_DATA * pSkel, int unsigned bodyPart, int unsigned joint1, int unsigned joint2 ) {
	if(		NULL				== bodyPartLengths[ bodyPart ]
		&&	NUI_JOINT_TRACKED	== pSkel->jointsTrackingState[ joint1 ]
		&&	NUI_JOINT_TRACKED	== pSkel->jointsTrackingState[ joint2 ]
		) {
		float dist = pointsDistance( pSkel->joints[ joint1 ], pSkel->joints[ joint2 ] );
		if( dist > ( 0.0254 * 5 ) ) {
			bodyPartLengths[ bodyPart ] = dist;
		}
	}
}

void CKARIApp::Nui_DrawSkeleton( bool bBlank, NUI_SKELETON_DATA * pSkel, HWND hWnd, int unsigned skeletonNumber ) {

	HGDIOBJ hOldObj = SelectObject( m_SkeletonDC, m_Pen[ skeletonNumber % m_PensTotal ] );

    RECT rct;
    GetClientRect( hWnd, &rct );
    int unsigned
		width	= rct.right,
		height	= rct.bottom;

    if( m_Pen[0] == NULL ) {
		m_Pen[0] = CreatePen( PS_SOLID, width / 100, g_SkeletonColors[ 0 ] );
		m_Pen[1] = CreatePen( PS_SOLID, width / 100, g_SkeletonColors[ 1 ] );
		m_Pen[2] = CreatePen( PS_SOLID, width / 100, g_SkeletonColors[ 2 ] );
		m_Pen[3] = CreatePen( PS_SOLID, width / 100, g_SkeletonColors[ 3 ] );
		m_Pen[4] = CreatePen( PS_SOLID, width / 100, g_SkeletonColors[ 4 ] );
		m_Pen[5] = CreatePen( PS_SOLID, width / 100, g_SkeletonColors[ 5 ] );
    }
	/*/
    if( bBlank ) {
		PatBlt( m_SkeletonDC, 0, 0, width, height, BLACKNESS );
    }
	/**/

	actionsDetect( false, pSkel, hWnd, skeletonNumber );

	//scaling up to image coordinate
	int unsigned
			scaleX = width,
			scaleY = height;

	float	fx = 0,
			fy = 0;

	for( int unsigned i = 0; i < NUI_JOINT_COUNT; i++ ) {
		NuiTransformSkeletonToDepthImageF( pSkel->joints[i], &fx, &fy );
		/**/
		m_Points[i].x = (int unsigned)( fx * 320 );
		m_Points[i].y = (int unsigned)( fy * 240 );
		/*/
		m_Points[i].x = (int)( fx * scaleX );
		m_Points[i].y = (int)( fy * scaleY );

		m_Points[i].x = (int)( fx * scaleX + 0.5f );
		m_Points[i].y = (int)( fy * scaleY + 0.5f );
		/**/
	}


	SelectObject( m_SkeletonDC, m_Pen[ skeletonNumber % m_PensTotal ] );


	/**/
	Nui_DrawSkeletonSegment( pSkel, 4, NUI_JOINT_HIP_CENTER,		NUI_JOINT_SPINE,			NUI_JOINT_SHOULDER_CENTER,	NUI_JOINT_HEAD									);
	Nui_DrawSkeletonSegment( pSkel, 5, NUI_JOINT_SHOULDER_CENTER,	NUI_JOINT_SHOULDER_LEFT,	NUI_JOINT_ELBOW_LEFT,		NUI_JOINT_WRIST_LEFT,	NUI_JOINT_HAND_LEFT		);
	Nui_DrawSkeletonSegment( pSkel, 5, NUI_JOINT_SHOULDER_CENTER,	NUI_JOINT_SHOULDER_RIGHT,	NUI_JOINT_ELBOW_RIGHT,		NUI_JOINT_WRIST_RIGHT,	NUI_JOINT_HAND_RIGHT	);
	Nui_DrawSkeletonSegment( pSkel, 5, NUI_JOINT_HIP_CENTER,		NUI_JOINT_HIP_LEFT,			NUI_JOINT_KNEE_LEFT,		NUI_JOINT_ANKLE_LEFT,	NUI_JOINT_FOOT_LEFT		);
	Nui_DrawSkeletonSegment( pSkel, 5, NUI_JOINT_HIP_CENTER,		NUI_JOINT_HIP_RIGHT,		NUI_JOINT_KNEE_RIGHT,		NUI_JOINT_ANKLE_RIGHT,	NUI_JOINT_FOOT_RIGHT	);
	/**/


	int unsigned r, g, b;
	float	intensity,
			greyPercent;
	COLORREF color;


	// Draw the joints in a different color

	int penSize;

	for( int unsigned i = 0; i < NUI_JOINT_COUNT; i++ ) {

		if( g_JointColorPower[ i ] > 0 ) {

			penSize					=	g_JointColorPower[ i ] == 1 ? 9 : 8;
			greyPercent				=	1 - g_JointColorPower[ i ];
			g_JointColorPower[ i ]	-=	.1;

			/*/
			r = 255;
			g = 255;
			b = 0;
			/**/
			/*/
			r = ( g_JointColorTable[i] >> 16 ) & 0xFF;
			g = ( g_JointColorTable[i] >> 8 ) & 0xFF;
			b = g_JointColorTable[i] & 0xFF;
			intensity = 0.3 * r + 0.59 * g + 0.11 * b;
			r = intensity * greyPercent + r * ( 1 - greyPercent );
			g = intensity * greyPercent + g * ( 1 - greyPercent );
			b = intensity * greyPercent + b * ( 1 - greyPercent );
			/**/
			/*/
			color = RGB( r, g, b );
			/**/

			color = interpolate( g_JointColorTable[i], g_SkeletonColors[ skeletonNumber ], greyPercent );


		} else {

			color	= g_SkeletonColors[ skeletonNumber ];
			penSize	= 3;

		}

		HPEN hJointPen;

		hJointPen	= CreatePen( PS_SOLID, penSize, color );
		hOldObj		= SelectObject( m_SkeletonDC, hJointPen );

		MoveToEx(	m_SkeletonDC, m_Points[i].x, m_Points[i].y, NULL	);
		LineTo(		m_SkeletonDC, m_Points[i].x, m_Points[i].y			);

		SelectObject( m_SkeletonDC, hOldObj );

		DeleteObject( hJointPen );

	}



}

bool CKARIApp::bodySegmentsMeasure( NUI_SKELETON_DATA * pSkel ) {

	if( bodyPartLengths.size() == 15 ) {


		float segment;
		float segmentDiff;

		/**/
		segment		= pointsDistance( pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ], pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ] );
		segmentDiff	= bodyPartLengths[ BP_SHOULDER_LEFT ] - segment;
		if( segmentDiff < -0.0254 || segmentDiff > 0.0254 ) {
			pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].x = pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ].x - bodyPartLengths[ BP_SHOULDER_LEFT ];
			//pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].y = pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].y;
			//pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].z = pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].z;
			//m_Log << "shoulder X : " << segment << "\t" << segmentDiff << "\t" << pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ].x <<  "\t" << bodyPartLengths[ BP_SHOULDER_LEFT ] << "\n";
		}


		segment		= pointsDistance( pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ], pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ] );
		segmentDiff	= bodyPartLengths[ BP_SHOULDER_RIGHT ] - segment;
		if( segmentDiff < -0.0254 || segmentDiff > 0.0254 ) {
			pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].x = pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ].x + bodyPartLengths[ BP_SHOULDER_RIGHT ];
		}
		/**/

		return true;

	} else
	if(
		(		pSkel->joints[ NUI_JOINT_ELBOW_RIGHT ].y	< pSkel->joints[ NUI_JOINT_WRIST_RIGHT ].y
			&&	pSkel->joints[ NUI_JOINT_ELBOW_LEFT ].y		< pSkel->joints[ NUI_JOINT_WRIST_LEFT ].y
			&&	pSkel->joints[ NUI_JOINT_ELBOW_RIGHT ].y	< pSkel->joints[ NUI_JOINT_WRIST_RIGHT ].y
		)
		&&
		(		pSkel->joints[ NUI_JOINT_HIP_RIGHT ].y		> pSkel->joints[ NUI_JOINT_KNEE_RIGHT ].y
			&&	pSkel->joints[ NUI_JOINT_HIP_LEFT ].y		> pSkel->joints[ NUI_JOINT_KNEE_LEFT ].y
		)
		&&
		(		pSkel->joints[ NUI_JOINT_KNEE_RIGHT ].y		> pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].y
			&&	pSkel->joints[ NUI_JOINT_KNEE_LEFT ].y		> pSkel->joints[ NUI_JOINT_FOOT_LEFT ].y
		)
		&&
		(		pSkel->joints[ NUI_JOINT_HIP_RIGHT ].x		< pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].x
			&&	pSkel->joints[ NUI_JOINT_HIP_LEFT ].x		> pSkel->joints[ NUI_JOINT_FOOT_LEFT ].x
		)
		&&
		(		pSkel->joints[ NUI_JOINT_ELBOW_RIGHT ].x	> pSkel->joints[ NUI_JOINT_WRIST_RIGHT ].x
			&&	pSkel->joints[ NUI_JOINT_ELBOW_LEFT ].x		< pSkel->joints[ NUI_JOINT_WRIST_LEFT ].x
		)

	) {

		RECT localLabel = {0, 0, 300, 20};
		PAINTSTRUCT localPs;
		HDC localHandle = BeginPaint(m_hWnd, &localPs);
		DrawText(m_SkeletonDC, "Calibrating", -1, &localLabel, DT_CENTER);
		EndPaint(m_hWnd, &localPs);




		m_Log << "calibration pos detected\n";

		setBodyPartLength( pSkel, BP_NECK,				NUI_JOINT_HEAD,				NUI_JOINT_SHOULDER_CENTER	);
		setBodyPartLength( pSkel, BP_TORSO_UPPER,		NUI_JOINT_SPINE,			NUI_JOINT_SHOULDER_CENTER	);
		setBodyPartLength( pSkel, BP_TORSO_LOWER,		NUI_JOINT_SPINE,			NUI_JOINT_HIP_CENTER		);
		setBodyPartLength( pSkel, BP_ARM_UPPER_LEFT,	NUI_JOINT_SHOULDER_LEFT,	NUI_JOINT_ELBOW_LEFT		);
		setBodyPartLength( pSkel, BP_ARM_UPPER_RIGHT,	NUI_JOINT_SHOULDER_RIGHT,	NUI_JOINT_ELBOW_RIGHT		);
		setBodyPartLength( pSkel, BP_ARM_FOR_LEFT,		NUI_JOINT_ELBOW_LEFT ,		NUI_JOINT_WRIST_LEFT		);
		setBodyPartLength( pSkel, BP_ARM_FOR_RIGHT,		NUI_JOINT_ELBOW_RIGHT,		NUI_JOINT_WRIST_RIGHT		);
		setBodyPartLength( pSkel, BP_SHOULDER_LEFT,		NUI_JOINT_SHOULDER_LEFT,	NUI_JOINT_SHOULDER_CENTER	);
		setBodyPartLength( pSkel, BP_SHOULDER_RIGHT,	NUI_JOINT_TRACKED,			NUI_JOINT_SHOULDER_RIGHT	);
		setBodyPartLength( pSkel, BP_HIP_LEFT,			NUI_JOINT_HIP_LEFT ,		NUI_JOINT_KNEE_LEFT			);
		setBodyPartLength( pSkel, BP_HIP_RIGHT,			NUI_JOINT_HIP_RIGHT ,		NUI_JOINT_KNEE_RIGHT		);
		setBodyPartLength( pSkel, BP_TIBIA_LEFT,		NUI_JOINT_ANKLE_LEFT ,		NUI_JOINT_KNEE_LEFT			);
		setBodyPartLength( pSkel, BP_TIBIA_RIGHT,		NUI_JOINT_ANKLE_RIGHT,		NUI_JOINT_KNEE_RIGHT		);
		setBodyPartLength( pSkel, BP_TARSUS_LEFT,		NUI_JOINT_ANKLE_LEFT ,		NUI_JOINT_FOOT_LEFT			);
		setBodyPartLength( pSkel, BP_TARSUS_RIGHT,		NUI_JOINT_ANKLE_RIGHT,		NUI_JOINT_FOOT_RIGHT		);

		if( bodyPartLengths.size() == 15 ) {

			bodyPartLengthFrames[ bodyPartLengthFrames.size() ] = bodyPartLengths;

			if( bodyPartLengthFrames.size() > 1 ) {
				static std::map<int unsigned, map<int unsigned, float > > validBodyFrames;
				for( int unsigned i = 0; i < bodyPartLengthFrames.size(); i++ ) {
					bool valid = true;
					for( int unsigned bp = 0; bp < 15; bp++ ) {
						float diff = bodyPartLengths[ bp ] - bodyPartLengthFrames[ i ][ bp ];
						if( diff < -0.000254 || diff > 0.000254 ) {
							valid = false;
							break;
						}
					}
					if( valid ) {
						validBodyFrames[ validBodyFrames.size() ] = bodyPartLengthFrames[ i ];
					}
				}
				bodyPartLengthFrames = validBodyFrames;
			}


			if( bodyPartLengthFrames.size() > 10 ) {

				/**/
				int checkHeight = ( pSkel->joints[ NUI_JOINT_HEAD ].y - pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].y ) / 0.0254;
				if( checkHeight > personHeight ) {
					personHeight = checkHeight;
					SetDlgItemInt( m_hWnd, IDC_PERSON_HEIGHT, personHeight, FALSE );
				}
				/*/
				static std::map<int, std::map<int, int> > lengths;

				lengths[0][0] = NUI_JOINT_FOOT_RIGHT;
				lengths[0][1] = NUI_JOINT_ANKLE_RIGHT;
				lengths[1][0] = NUI_JOINT_ANKLE_RIGHT;
				lengths[1][1] = NUI_JOINT_KNEE_RIGHT;
				lengths[2][0] = NUI_JOINT_KNEE_RIGHT;
				lengths[2][1] = NUI_JOINT_HIP_RIGHT;

				lengths[3][0] = NUI_JOINT_HIP_CENTER;
				lengths[3][1] = NUI_JOINT_SPINE;
				lengths[4][0] = NUI_JOINT_SPINE;
				lengths[4][1] = NUI_JOINT_SHOULDER_CENTER;
				lengths[5][0] = NUI_JOINT_SHOULDER_CENTER;
				lengths[5][1] = NUI_JOINT_HEAD;

				personHeight = 0;
				for( int unsigned i = 0; i < 6; i++ ) {
					Vector4& p = pSkel->joints[ lengths[i][0] ];
					Vector4& v = pSkel->joints[ lengths[i][1] ];
					personHeight += sqrt( (p.x-v.x) * (p.x-v.x) + (p.y-v.y) * (p.y-v.y) + (p.z-v.z) * (p.z-v.z) ) / 0.0254;
				}
				/**/

				m_Log << "body lengths recorded\n";

				for( int unsigned i = 0; i < bodyPartLengths.size(); i++ ) {
					m_Log << "\t" << i << " : " << bodyPartLengths[ i ] << "\n";
				}

				return true;

			} else {

				bodyPartLengths.clear();

			}

		}

	}

	return false;

}

bool CKARIApp::checkSkeletonValid( NUI_SKELETON_DATA * pSkel ) {

	if(		pSkel->joints[ NUI_JOINT_HEAD ].y				< pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ].y
		||	pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ].y	< pSkel->joints[ NUI_JOINT_SPINE ].y
		||	pSkel->joints[ NUI_JOINT_SPINE ].y				< pSkel->joints[ NUI_JOINT_HIP_CENTER ].y

		||	pSkel->joints[ NUI_JOINT_HIP_LEFT ].y			< pSkel->joints[ NUI_JOINT_KNEE_LEFT ].y
		||	pSkel->joints[ NUI_JOINT_KNEE_LEFT ].y			< pSkel->joints[ NUI_JOINT_ANKLE_LEFT ].y
		||	pSkel->joints[ NUI_JOINT_ANKLE_LEFT ].y			< pSkel->joints[ NUI_JOINT_FOOT_LEFT ].y

		||	pSkel->joints[ NUI_JOINT_HIP_RIGHT ].y			< pSkel->joints[ NUI_JOINT_KNEE_RIGHT ].y
		||	pSkel->joints[ NUI_JOINT_KNEE_RIGHT ].y			< pSkel->joints[ NUI_JOINT_ANKLE_RIGHT ].y
		||	pSkel->joints[ NUI_JOINT_ANKLE_RIGHT ].y		< pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].y
		) {
		return false;
	}
	return true;

}

struct actionExecuteOnStruct {
	int signed								i				;
	NUI_SKELETON_DATA *						pSkel			;
	NUI_SKELETON_DATA *						prevSkel		;
	int										distance		;
	int										angleX			;
	int										angleY			;
	std::map<int unsigned, int unsigned>	curBodyParts	;
	std::map<int signed, float>				jointDistances	;
	std::map<int unsigned, int unsigned>	curJoints		;
	std::map<int signed, float>				jointAnglesX	;
	std::map<int signed, float>				jointAnglesY	;
};

		bool IsThreadAlive(const HANDLE hThread, bool& bAlive )
{
 // Read thread's exit code.
 DWORD dwExitCode = 0;
 if( GetExitCodeThread(hThread, &dwExitCode))
 {
 // if return code is STILL_ACTIVE,
 // then thread is live.
 bAlive = (dwExitCode == STILL_ACTIVE);
 return true;
 }

 // Check failed.
 return false;
}

DWORD WINAPI CKARIApp::actionExecuteOn( void * argsInput ) {
//void actionExecuteOn( void * argsInput ) {

	actionExecuteOnStruct * args = (actionExecuteOnStruct*) argsInput;
	
	int signed i											= args->i;					m_Log << "[21]";
	NUI_SKELETON_DATA * pSkel								= args->pSkel;				m_Log << "[22]";
	NUI_SKELETON_DATA * prevSkel							= args->prevSkel;			m_Log << "[23]";
	int distance											= args->distance;			m_Log << "[24]";
	int angleX												= args->angleX;				m_Log << "[25]";
	int angleY												= args->angleY;				m_Log << "[26]";
	std::map<int unsigned, int unsigned>	curBodyParts	= args->curBodyParts;		m_Log << "[27]";
	std::map<int signed, float>				jointDistances	= args->jointDistances;		m_Log << "[28]";
	std::map<int unsigned, int unsigned>	curJoints		= args->curJoints;			m_Log << "[29]";
	std::map<int signed, float>				jointAnglesX	= args->jointAnglesX;		m_Log << "[291]";
	std::map<int signed, float>				jointAnglesY	= args->jointAnglesY;		m_Log << "[292]";
		
	m_Log << "3";

	for( int unsigned i2 = 0; i2 < actionSets[i][AS_EXECUTE].size(); i2++ ) {
		
	m_Log << "4[" << i << ":" << i2 << "]";
	/*/
	m_Log << "as1[" << actionSets[i].size() << "]";
	m_Log << "as2[" << actionSets[i][AS_EXECUTE].size() << "]";
	m_Log << "as3[" << actionSets[i][AS_EXECUTE][i2].size() << "]";
	m_Log << "as4[" << actionSets[i][AS_EXECUTE][i2][AS_MOUSE_TRACK] << "]";
	m_Log << "as5[" << actionSets[i][AS_EXECUTE][i2][AS_WINDOW_HOLD] << "]";
	/**/
	
		int mouseCommandMode =
			actionSets[i][AS_EXECUTE][i2][AS_MOUSE_TRACK]
				? actionSets[i][AS_EXECUTE][i2][AS_MOUSE_TRACK]
				: actionSets[i][AS_EXECUTE][i2][AS_WINDOW_HOLD]
		;
			
	m_Log << "4.1[" << mouseCommandMode << "]";

		if( mouseCommandMode ) {
			
	m_Log << "4.2[" << mouseCommandMode << "]";

			float dDiff		= jointDistances[ curJoints[0] ] - actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MIN];
			float dRange	= actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MAX] - actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MIN];
			float dStrength	= dDiff / dRange;

			// get should point from hand point
			Vector4&	should	= pSkel->joints[ curJoints[0] - 3 ];
			Vector4&	hand	= pSkel->joints[ curJoints[0] ];

			int signed offsetX;
			int signed offsetY;

			int unsigned xGoal, yGoal;
			float xDiff, yDiff, xPercent, yPercent, xAngle, yAngle, prevAngleX, prevAngleY, anglePixelsX, anglePixelsY;

			Vector4& prevShould	= prevSkel->joints[ curJoints[0] - 3 ];
			Vector4& prevHand	= prevSkel->joints[ curJoints[0] ];

			switch( mouseCommandMode ) {
				case ACV_ABSOLUTE:
					xAngle		= jointAnglesX[ curJoints[0] ];
					yAngle		= jointAnglesY[ curJoints[0] ];

					xDiff		= xAngle - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X];
					xPercent	= xDiff / ( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_X] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X] );

					yDiff		= yAngle - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y];
					yPercent	= yDiff / ( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_Y] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y] );

					xGoal		= screenSizeX * xPercent;
					yGoal		= screenSizeY * ( 1 - yPercent );
				break;
				case ACV_PUSH:
					offsetX = (int signed)( ( ( hand.x - should.x ) / 0.0254 ) / dStrength );
					offsetY = (int signed)( ( ( should.y - hand.y ) / 0.0254 ) / dStrength );
				break;
				case ACV_RELATIVE:
					/*/
					setup so that movement range is mapped to detection area and screen size, same as absolute.
					/**/
					prevAngleX		= atan2( (double)( prevShould.z - prevHand.z ), (double)( prevShould.x - prevHand.x ) ) * 180 / PI;
					prevAngleY		= atan2( (double)( prevShould.z - prevHand.z ), (double)( prevShould.y - prevHand.y ) ) * 180 / PI;

					anglePixelsX	= screenSizeX / ( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_X] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X] );
					anglePixelsY	= screenSizeY / ( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_Y] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y] );

					xAngle			= jointAnglesX[ curJoints[0] ];
					yAngle			= jointAnglesY[ curJoints[0] ];

					offsetX			= ( xAngle - prevAngleX ) * anglePixelsX;
					offsetY			= ( prevAngleY - yAngle ) * anglePixelsY;
				break;
				case ACV_DRAG:
					offsetX = sineEaseInOut( dStrength, 1, 60, 1 ) * ( ( hand.x - prevSkel->joints[ curJoints[0] ].x ) / 0.0254 );
					offsetY = sineEaseInOut( dStrength, 1, 60, 1 ) * ( ( prevSkel->joints[ curJoints[0] ].y - hand.y ) / 0.0254 );
				break;
			}



			switch( actionSets[i][AS_EXECUTE][i2][AS_MOUSE_TRACK] ) {
				case ACV_ABSOLUTE:										SetCursorPos( xGoal, yGoal );			break;
				case ACV_PUSH:		if( offsetX != 0 || offsetY != 0 )	mouseMove( offsetX, offsetY );			break;
				case ACV_RELATIVE:	if( offsetX != 0 || offsetY != 0 )	mouseMove( offsetX, offsetY );			break;
				case ACV_DRAG:		if( offsetX != 0 || offsetY != 0 )	mouseMove( offsetX, offsetY );			break;
			}

			switch( actionSets[i][AS_EXECUTE][i2][AS_WINDOW_HOLD] ) {
				case ACV_ABSOLUTE:										placeActiveWindow( xGoal, yGoal );		break;
				case ACV_PUSH:		if( offsetX != 0 || offsetY != 0 )	moveActiveWindow( offsetX, offsetY );	break;
				case ACV_RELATIVE:	if( offsetX != 0 || offsetY != 0 )	moveActiveWindow( offsetX, offsetY );	break;
				case ACV_DRAG:		if( offsetX != 0 || offsetY != 0 )	moveActiveWindow( offsetX, offsetY );	break;
			}


		} else
		/**/
		if( !actionSetsStatus[i] ) {
			
	m_Log << "5";
			m_Log << "on : " << i << "\n";
			for( int unsigned cb = 0; cb < curBodyParts.size(); cb++ ) {
				m_Log << "\t" << s_bodyActions[ curBodyParts[ cb ] ] << "\n";
			}
			m_Log << "\t";
			m_Log << "\td:" << distance << " : " << actionSets[0][AS_REQUIRE][0][AS_DISTANCE_MIN];
			m_Log << "\taX:" << angleX;
			m_Log << "\taY:" << angleY;
			m_Log << "\n";

			if( actionSets[i][AS_EXECUTE][i2][AS_KEY_TAP] ) {
				//_beginthread( keyTap, 0, (void*)actionSets[i][AS_EXECUTE][i2][AS_KEY_TAP] );
				keyTap( actionSets[i][AS_EXECUTE][i2][AS_KEY_TAP] );
			}

			if( actionSets[i][AS_EXECUTE][i2][AS_KEY_HOLD] ) {
				keyPress( actionSets[i][AS_EXECUTE][i2][AS_KEY_HOLD] );
			}

			if( actionSets[i][AS_EXECUTE][i2][AS_KEY_PRESS] ) {
				keyPress( actionSets[i][AS_EXECUTE][i2][AS_KEY_PRESS] );
			}

			if( actionSets[i][AS_EXECUTE][i2][AS_KEY_RELEASE] ) {
				keyRelease( actionSets[i][AS_EXECUTE][i2][AS_KEY_RELEASE] );
			}

			if( actionSets[i][AS_EXECUTE][i2][AS_MOUSE_TAP] ) {

				int unsigned down;
				int unsigned up;
				switch( actionSets[i][AS_EXECUTE][i2][AS_MOUSE_TAP] ) {
					case ACV_MOUSE_LEFT:
						down	= MOUSEEVENTF_LEFTDOWN;
						up		= MOUSEEVENTF_LEFTUP;
					break;
					case ACV_MOUSE_RIGHT:
						down	= MOUSEEVENTF_RIGHTDOWN;
						up		= MOUSEEVENTF_RIGHTUP;
					break;
					default:
						continue;
				}

				INPUT *buffer = new INPUT[2];

				buffer->type = INPUT_MOUSE;
				buffer->mi.mouseData = 0;
				buffer->mi.dwFlags = down;
				buffer->mi.time = 0;
				buffer->mi.dwExtraInfo = 0;

				(buffer+1)->type = INPUT_MOUSE;
				(buffer+1)->mi.mouseData = 0;
				(buffer+1)->mi.dwFlags = up;
				(buffer+1)->mi.time = 0;
				(buffer+1)->mi.dwExtraInfo = 0;

				SendInput( 2, buffer, sizeof( buffer ) );

			}

			if( actionSets[i][AS_EXECUTE][i2][AS_MOUSE_HOLD] ) {
				int unsigned down;
				switch( actionSets[i][AS_EXECUTE][i2][AS_MOUSE_HOLD] ) {
					case ACV_MOUSE_LEFT:	down	= MOUSEEVENTF_LEFTDOWN;		break;
					case ACV_MOUSE_RIGHT:	down	= MOUSEEVENTF_RIGHTDOWN;	break;
					default:
						continue;
				}
				INPUT input;
				input.type = INPUT_MOUSE;
				input.mi.mouseData = 0;
				input.mi.dwFlags = down;
				input.mi.time = 0;
				input.mi.dwExtraInfo = 0;
				SendInput( 1, &input, sizeof( input ) );
			}

		}

		if( actionSets[i][AS_EXECUTE][i2][AS_CONFIG_LOAD] ) {
			loadConfig( configFiles[ actionSets[i][AS_EXECUTE][i2][AS_CONFIG_LOAD] ] );
		}

		if( actionSets[i][AS_EXECUTE][i2][AS_WINDOW_SHOW] ) {
			HWND activeWindow = GetForegroundWindow();
			ShowWindow( activeWindow, actionSets[i][AS_EXECUTE][i2][AS_WINDOW_SHOW] );
		}

	}

	return (0);
}


void CKARIApp::actionsDetect( bool bBlank, NUI_SKELETON_DATA * pSkel, HWND hWnd, int unsigned skeletonNumber ) {


	if(		!checkSkeletonValid( pSkel )
		||	!bodySegmentsMeasure( pSkel )
		) {
		return;
	}

	std::map<int unsigned, int unsigned>	curBodyParts;
	std::map<int unsigned, int unsigned>	curJoints;
	std::map<int signed, float>				jointDistances;
	std::map<int signed, float>				jointAnglesX;
	std::map<int signed, float>				jointAnglesY;

	/**/


	bool			go;
	bool			useToggle;
	bool			toggle;
	bool			notTracked;
	bool			bypassStatus;
	bool			currentSetStatus;
	int signed		distance;
	double			angleX;
	double			angleY;
	float			leftX;
	float			centerX;
	float			rightX;
	int unsigned	actionSetNum;

	for( int unsigned i = 0; i < actionsCount; i++ ) {

		useToggle			= false;
		toggle				= false;
		notTracked			= false;
		bypassStatus		= false;
		currentSetStatus	= true;
		distance			= 0;
		angleX				= 0;
		angleY				= 0;
		leftX				= 0;
		centerX				= 0;
		rightX				= 0;
		actionSetNum		= 0;
		/*/
		curJoints.clear();
		jointDistances.clear();
		curBodyParts.clear();
		/**/
		for( int unsigned i2 = 0; i2 < actionSets[i][AS_REQUIRE].size(); i2++ ) {

			curBodyParts[ curBodyParts.size() ] = actionSets[i][AS_REQUIRE][i2][AS_BODY_ACTION];

			switch( actionSets[i][AS_REQUIRE][i2][AS_BODY_ACTION] ) {
				case BA_LEAN_LEFT:				//	lean_left				:	inches lean left
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HEAD;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						jointDistances[ NUI_JOINT_HEAD ] = ( pSkel->joints[ NUI_JOINT_HIP_CENTER ].x - pSkel->joints[ NUI_JOINT_HEAD ].x ) / 0.0254;
						distance = (int)jointDistances[ NUI_JOINT_HEAD ];
					}
				break;
				case BA_LEAN_RIGHT:				//	lean_right				:	inches lean right
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HEAD;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						jointDistances[ NUI_JOINT_HEAD ] = ( pSkel->joints[ NUI_JOINT_HEAD ].x - pSkel->joints[ NUI_JOINT_HIP_CENTER ].x ) / 0.0254;
						distance = (int)jointDistances[ NUI_JOINT_HEAD ];
					}
				break;
				case BA_LEAN_FORWARD:			//	lean_forwards			:	inches body lean forward
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HEAD;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						jointDistances[ NUI_JOINT_HEAD ] = ( pSkel->joints[ NUI_JOINT_HIP_CENTER ].z - pSkel->joints[ NUI_JOINT_HEAD ].z ) / 0.0254;
						distance = (int)jointDistances[ NUI_JOINT_HEAD ];
					}
				break;
				case BA_LEAN_BACKWARD:			//	lean_backwards			:	inches body lean back
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_SHOULDER_CENTER;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						jointDistances[ NUI_JOINT_HEAD ] = ( pSkel->joints[ NUI_JOINT_HEAD ].z - pSkel->joints[ NUI_JOINT_HIP_CENTER ].z ) / 0.0254;
						distance = (int)jointDistances[ NUI_JOINT_HEAD ];
					}
				break;
				case BA_TURN_LEFT:				//	turn_left				:	angular amount of left body turn (degrees)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_SHOULDER_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_ANGLE] ) {
						Vector4&	center		= pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ];
						Vector4&	right		= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ];
						if( center.z > right.z &&  center.x < right.x ) {
							angleX	= atan2( (double)( center.z - right.z ), (double)( right.x - center.x ) ) * 180 / PI;
						}
					}
				break;
				case BA_TURN_RIGHT:				//	turn_right				:	angular amount of right body turn(degrees)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_SHOULDER_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_ANGLE] ) {
						Vector4&	center		= pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ];
						Vector4&	left		= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ];
						if( center.z > left.z && center.x > left.x ) {
							angleX	= atan2( (double)( center.z - left.z ), (double)( center.x - left.x ) ) * 180 / PI;
						}
					}
				break;
				case BA_HAND_LEFT_EXPAND:
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					useToggle	= true;
					toggle		= checkAction_handOpen( NUI_JOINT_HAND_LEFT );
				break;
				case BA_HAND_LEFT_FORWARD:		//	left_arm_forwards		:	forward distance from left hand to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						jointDistances[ NUI_JOINT_HAND_LEFT ] = ( pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].z - pSkel->joints[ NUI_JOINT_HAND_LEFT ].z ) / 0.0254;
						distance = (int)jointDistances[ NUI_JOINT_HAND_LEFT ];
					}
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_ANGLE] ) {
						Vector4&	hand	= pSkel->joints[ NUI_JOINT_HAND_LEFT ];
						Vector4&	should	= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ];
						jointAnglesX[ NUI_JOINT_HAND_LEFT ] = angleX = atan2( (double)( should.z - hand.z ), (double)( should.x - hand.x ) ) * 180 / PI;
						jointAnglesY[ NUI_JOINT_HAND_LEFT ] = angleY = atan2( (double)( should.z - hand.z ), (double)( should.y - hand.y ) ) * 180 / PI;
					}
				break;
				case BA_HAND_LEFT_DOWN:			//	left_arm_down			:	downward distance from left hand to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].y - pSkel->joints[ NUI_JOINT_HAND_LEFT ].y ) / 0.0254 );
					}
				break;
				case BA_HAND_LEFT_UP:			//	left_arm_up				:	upward distance from left hand to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_HAND_LEFT ].y - pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].y ) / 0.0254 );
					}
				break;
				case BA_HAND_LEFT_OUT:			//	left_arm_out			:	sideways distance from left hand to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].x - pSkel->joints[ NUI_JOINT_HAND_LEFT ].x ) / 0.0254 );
					}
				break;
				case BA_HAND_LEFT_ACROSS:		//	left_arm_across			:	sideways distance from left hand across body to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_HAND_LEFT ].x - pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].x ) / 0.0254 );
					}
				break;
				case BA_HAND_LEFT_BACKWARD:
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_HAND_LEFT ].z - pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].z ) / 0.0254 );
					}
				break;
				case BA_HAND_RIGHT_EXPAND:
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					useToggle	= true;
					toggle		= checkAction_handOpen( NUI_JOINT_HAND_RIGHT );
				break;
				case BA_HAND_RIGHT_FORWARD:		//	right_arm_forwards		:	forward distance from right hand to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						jointDistances[ NUI_JOINT_HAND_RIGHT ] = ( pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].z - pSkel->joints[ NUI_JOINT_HAND_RIGHT ].z ) / 0.0254;
						distance = (int)jointDistances[ NUI_JOINT_HAND_RIGHT ];
					}
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_ANGLE] ) {
						Vector4&	hand	= pSkel->joints[ NUI_JOINT_HAND_RIGHT ];
						Vector4&	should	= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ];
						jointAnglesX[ NUI_JOINT_HAND_RIGHT ] = angleX = atan2( (double)( should.z - hand.z ), (double)( should.x - hand.x ) ) * 180 / PI;
						jointAnglesY[ NUI_JOINT_HAND_RIGHT ] = angleY = atan2( (double)( should.z - hand.z ), (double)( should.y - hand.y ) ) * 180 / PI;
					}
				break;
				case BA_HAND_RIGHT_DOWN:		//	right_arm_down			:	downward distance from right hand to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].y - pSkel->joints[ NUI_JOINT_HAND_RIGHT ].y ) / 0.0254 );
					}
				break;
				case BA_HAND_RIGHT_UP:			//	right_arm_up			:	upward distance from right hand to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_HAND_RIGHT ].y - pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].y ) / 0.0254 );
					}
				break;
				case BA_HAND_RIGHT_OUT:			//	right_arm_out			:	sideways distance from right hand to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_HAND_RIGHT ].x - pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].x ) / 0.0254 );
					}
				break;
				case BA_HAND_RIGHT_ACROSS:		//	right_arm_across		:	sideways distance from right hand across body to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].x - pSkel->joints[ NUI_JOINT_HAND_RIGHT ].x ) / 0.0254 );
					}
				break;
				case BA_HAND_RIGHT_BACKWARD:
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_HAND_RIGHT ].z - pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].z ) / 0.0254 );
					}
				break;
				case BA_FOOT_LEFT_FORWARD:		//	left_foot_forwards		:	forward distance from left hip to foot (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_HIP_LEFT ].z - pSkel->joints[ NUI_JOINT_FOOT_LEFT ].z ) / 0.0254 );
					}
				break;
				case BA_FOOT_LEFT_OUT:			//	left_foot_sideways		:	sideways distance from left hip to foot (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_HIP_LEFT ].x - pSkel->joints[ NUI_JOINT_FOOT_LEFT ].x ) / 0.0254 );
					}
				break;
				case BA_FOOT_LEFT_ACROSS:		//	right_arm_across		:	sideways distance from right hand across body to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_FOOT_LEFT ].x - pSkel->joints[ NUI_JOINT_HIP_LEFT ].x ) / 0.0254 );
					}
				break;
				case BA_FOOT_LEFT_BACKWARD:		//	left_foot_backwards		:	backwards distance from left hip to foot (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_FOOT_LEFT ].z - pSkel->joints[ NUI_JOINT_HIP_LEFT ].z ) / 0.0254 );
					}
				break;
				case BA_FOOT_LEFT_UP:			//	left_foot_up			:	height of left foot above other foot on ground (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_FOOT_LEFT ].y - pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].y ) / 0.0254 );
					}
				break;
				case BA_FOOT_RIGHT_FORWARD:		//	right_foot_forwards		:	forward distance from right hip to foot (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_HIP_RIGHT ].z - pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].z ) / 0.0254 );
					}
				break;
				case BA_FOOT_RIGHT_OUT:			//	right_foot_sideways		:	sideways distance from right hip to foot (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].x - pSkel->joints[ NUI_JOINT_HIP_RIGHT ].x ) / 0.0254 );
					}
				break;
				case BA_FOOT_RIGHT_ACROSS:		//	right_arm_across		:	sideways distance from right hand across body to shoulder (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_HIP_RIGHT ].x - pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].x ) / 0.0254 );
					}
				break;
				case BA_FOOT_RIGHT_BACKWARD:	//	right_foot_backwards	:	backwards distance from right hip to foot (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].z - pSkel->joints[ NUI_JOINT_HIP_RIGHT ].z ) / 0.0254 );
					}
				break;
				case BA_FOOT_RIGHT_UP:			//	right_foot_up			:	height of right foot above other foot on ground (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						distance = (int)( ( pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].y - pSkel->joints[ NUI_JOINT_FOOT_LEFT ].y ) / 0.0254 );
					}
				break;
				case BA_JUMP:					//	jump					:	height of both feet above ground (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_LEFT;

					distance = getSkeletonElevation( pSkel );
					if( distance > 0 ) {
						SetDlgItemInt( m_hWnd, IDC_PERSON_ELEVATION, distance, FALSE );
					}
				break;
				case BA_CROUCH:					//	crouch					:	crouch distance: calculated as current height subtracted from standing height (inches)

					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_HEAD;

					int signed offGround;

					offGround = getSkeletonElevation( pSkel );

					if( offGround < 0 ) {

						bypassStatus = true;

					} else
					if( offGround < 10 ) {

						float& footY = pSkel->joints[ NUI_JOINT_FOOT_LEFT ].y < pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].y
							? pSkel->joints[ NUI_JOINT_FOOT_LEFT ].y
							: pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].y
						;

						int curHeight = ( pSkel->joints[ NUI_JOINT_HEAD ].y - footY ) / 0.0254;

						if( personHeight > curHeight ) {
							distance = personHeight - curHeight;
						}

					}

				break;
				case BA_WALK:					//	walk					:	height of each step above ground when walking in place (inches)
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					curJoints[ (int unsigned)curJoints.size() ] = NUI_JOINT_FOOT_LEFT;
				break;
			}

					if( bypassStatus == true																							)	{	currentSetStatus = actionSetsStatus[ i ];	break;	}
			else	if( actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MIN]	&& distance	< actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MIN]	)	{	currentSetStatus = false;							}
			else	if( actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MAX]	&& distance	> actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MAX]	)	{	currentSetStatus = false;							}
			else	if( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X]	&& angleX	< actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X]		)	{	currentSetStatus = false;							}
			else	if( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_X]	&& angleX	> actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_X]		)	{	currentSetStatus = false;							}
			else	if( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y]	&& angleY	< actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y]		)	{	currentSetStatus = false;							}
			else	if( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_Y]	&& angleY	> actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_Y]		)	{	currentSetStatus = false;							}
			else	if( useToggle										&& !toggle														)	{	currentSetStatus = false;							}

		}


		if( currentSetStatus == true ) {

			//_beginthread( keyTap, 0, (void*)actionSets[i][AS_EXECUTE][i2][AS_KEY_TAP] );
			//NUI_SKELETON_DATA * pSkel
			NUI_SKELETON_DATA * prevSkel = &previousSkeltonFrame.SkeletonData[ skeletonNumber ];
			
			struct actionExecuteOnStruct ps = {
				i,
				pSkel,
				prevSkel,
				distance,
				angleX,
				angleY,
				curBodyParts,
				jointDistances,
				curJoints,
				jointAnglesX,
				jointAnglesY
			};
			m_Log << "\n";
			for( int unsigned cb = 0; cb < ps.curBodyParts.size(); cb++ ) {
				m_Log << "\t" << s_bodyActions[ ps.curBodyParts[ cb ] ] << "\n";
			}
			//_beginthread( actionExecuteOn, 0, (void*)&ps );
			
		
	/*/
	m_Log << "a";
	int unsigned							test_distance;
	std::map<int unsigned, int unsigned>	test_curBodyParts;
	test_distance			= 1;
	test_curBodyParts[ 1 ]	= 2;
	m_Log << "b";
	struct testStruct test_ps = { test_distance, test_curBodyParts };
	m_Log << "c";
	HANDLE test_handle = 0;
	m_Log << "d";
	test_handle = CreateThread( NULL, 0, testThread, (void*)&test_ps, 0, NULL );
	m_Log << "e";
	WaitForSingleObject( test_handle, INFINITE );
	m_Log << "f";
	DWORD test_exit;
	if( ::GetExitCodeThread( test_handle, &test_exit ) != 0 && test_exit != 0 ) {
		m_Log << "g";
		m_Log << "[thread:" << test_exit << "]";		
		ExitThread(test_exit);
		m_Log << "h";
	}
	m_Log << "i\n";
	CloseHandle( test_handle );
	/**/

	actionExecuteOn( (void*)&ps );
	/*/
	m_Log << "a";
	int unsigned							test_distance;
	std::map<int unsigned, int unsigned>	test_curBodyParts;
	test_distance			= 1;
	test_curBodyParts[ 1 ]	= 2;
	m_Log << "b";
	struct testStruct test_ps = { test_distance, test_curBodyParts };
	m_Log << "c";
	HANDLE test_handle = 0;
	m_Log << "d";
	test_handle = CreateThread( NULL, 0, actionExecuteOn, (void*)&ps, 0, NULL );
	//test_handle = CreateThread( NULL, 0, testThread, (void*)&test_ps, 0, NULL );
	actionExecuteThreads.push_back( test_handle );
	m_Log << "e[" << actionExecuteThreads.size() << "]";
	vector<HANDLE> tmpHandlers;
	tmpHandlers.clear();
	for( int unsigned cj = 0; cj < actionExecuteThreads.size(); cj++ ) {		
		m_Log << "f";
		DWORD test_exit;
		BOOL test_existResult = GetExitCodeThread( actionExecuteThreads[ cj ], &test_exit );
		m_Log << "g[" << actionExecuteThreads[ cj ] << ":" << test_existResult  << ":" << test_exit << "]" << "]";
		if( actionExecuteThreads[ cj ] != 0 && test_existResult != 0 && test_exit != 0 && test_exit != STILL_ACTIVE ) {
			m_Log << "h";
			CloseHandle( actionExecuteThreads[ cj ] );
			m_Log << "i";			
			//ExitThread(test_exit);
			m_Log << "j";
		} else {
			m_Log << "k";
			tmpHandlers.push_back( actionExecuteThreads[ cj ] );
		}
	}
	actionExecuteThreads.clear();
	actionExecuteThreads = tmpHandlers;
	m_Log << "l[" << actionExecuteThreads.size() << "]\n";
	/**/


			/**/
			//actionExecuteOn( i, pSkel, prevSkel, distance, angleX, angleY, curBodyParts, jointDistances, curJoints, jointAnglesX, jointAnglesY );

			actionSetsStatus[i]	= true;

			for( int unsigned cj = 0; cj < curJoints.size(); cj++ ) {
				g_JointColorPower[ curJoints[ cj ] ] = 1;
			}

		} else if( actionSetsStatus[i] ) {

			actionSetsStatus[i] = false;

			m_Log << "off : " << i << "\n";
			for( int unsigned cb = 0; cb < curBodyParts.size(); cb++ ) {
				m_Log << "\t" << s_bodyActions[ curBodyParts[ cb ] ] << "\n";
			}
			m_Log << "\t";
			m_Log << "\td:" << distance;
			m_Log << "\taX:" << angleX;
			m_Log << "\taY:" << angleY;
			m_Log << "\n";

			for( int unsigned i2 = 0; i2 < actionSets[i][AS_EXECUTE].size(); i2++ ) {

				if( actionSets[i][AS_EXECUTE][i2][AS_KEY_HOLD] ) {
					keyRelease( actionSets[i][AS_EXECUTE][i2][AS_KEY_HOLD] );
				}

				if( actionSets[i][AS_EXECUTE][i2][AS_MOUSE_HOLD] ) {
					int unsigned up;
					switch( actionSets[i][AS_EXECUTE][i2][AS_MOUSE_HOLD] ) {
						case ACV_MOUSE_LEFT:	up	= MOUSEEVENTF_LEFTUP;	break;
						case ACV_MOUSE_RIGHT:	up	= MOUSEEVENTF_RIGHTUP;	break;
						default:
							continue;
					}
					INPUT input;
					input.type = INPUT_MOUSE;
					input.mi.mouseData = 0;
					input.mi.dwFlags = up;
					input.mi.time = 0;
					input.mi.dwExtraInfo = 0;
					SendInput( 1, &input, sizeof( input ) );
				}
				if(		actionSets[i][AS_EXECUTE][i2][AS_MOUSE_TRACK]
					||	actionSets[i][AS_EXECUTE][i2][AS_WINDOW_HOLD]
					) {
					actionSets[i][AS_EXECUTE][i2].erase(AS_JOINT_PREV_X);
					actionSets[i][AS_EXECUTE][i2].erase(AS_JOINT_PREV_Y);
				}
			}

		}

	}
	
	return;

}

void CKARIApp::Nui_DoDoubleBuffer( HWND hWnd, HDC hDC ) {
    RECT rct;
    GetClientRect( hWnd, &rct );
    HDC hdc = GetDC( hWnd );
    BitBlt( hdc, 0, 0, rct.right, rct.bottom, hDC, 0, 0, SRCCOPY );
    ReleaseDC( hWnd, hdc );
}

void CKARIApp::Nui_GotSkeletonAlert() {
	

	HRESULT hr = NuiSkeletonGetNextFrame( 0, &SkeletonFrame );

	/**/
	//::NuiCameraElevationSetAngle( -24 );
	//::NuiCameraElevationSetAngle( -9 );
	//::NuiCameraElevationSetAngle( -4 );
	/**/

//	if( !cameraAngleCount ) {
	

		saveCameraAngle();

		SetDlgItemInt(
			m_hWnd,
			IDC_CAMERA_W,
			SkeletonFrame.vFloorClipPlane.w ? SkeletonFrame.vFloorClipPlane.w / 0.0254 : 0,
			FALSE
		);

		SetDlgItemInt(
			m_hWnd,
			IDC_CAMERA_ANGLE,
			/*/
			cameraAngleTest,
			/*/
			cameraAngleAverage > 0 ? cameraAngleAverage : -cameraAngleAverage,
			/**/
			FALSE
		);

//	}
		
	

	//smooth out the skeleton data
	NuiTransformSmooth( &SkeletonFrame, &mySmooth );
	//NuiTransformSmooth( &SkeletonFrame, NULL );


	// draw each skeleton color according to the slot within they are found.

	bool bBlank = true;

    RECT rct;
    GetClientRect( m_hWnd, &rct );
    int unsigned width	= rct.right;
    int unsigned height	= rct.bottom;
	PatBlt( m_SkeletonDC, 0, 0, width, height, BLACKNESS );
	
	for( int unsigned i = 0; i < NUI_SKELETON_COUNT; i++ ) {
		if( SkeletonFrame.SkeletonData[i].trackingState == NUI_SKELETON_TRACKED ) {
			Nui_DrawSkeleton(
				bBlank,
				&SkeletonFrame.SkeletonData[i],
				GetDlgItem( m_hWnd, IDC_SKELETALVIEW ),
				i
			);
			bBlank = false;
		}
	}

	previousSkeltonFrame = SkeletonFrame;

	if( !bBlank ) {
		// we found a skeleton, re-start the timer
		m_bScreenBlanked		= false;
		m_LastSkeletonFoundTime	= -1;
	}

	Nui_DoDoubleBuffer( GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), m_SkeletonDC );

}



RGBQUAD CKARIApp::Nui_ShortToQuad_Depth( USHORT s ) {

	USHORT RealDepth	= ( s & 0xfff8 ) >> 3;
	USHORT Player		= s & 7;

	RGBQUAD q;

	// transform 13-bit depth information into an 8-bit intensity appropriate
	// for display (we disregard information in most significant bit)
	BYTE l = 255 - (BYTE)( 256 * RealDepth / 0x0fff );


	q.rgbRed = q.rgbBlue = q.rgbGreen = 0;

	switch( Player ) {
		case 0:
			q.rgbRed	= l / 2;
			q.rgbBlue	= l / 2;
			q.rgbGreen	= l / 2;
		break;
		case 1:
			q.rgbRed	= l;
		break;
		case 2:
			q.rgbGreen	= l;
		break;
		case 3:
			q.rgbRed	= l / 4;
			q.rgbGreen	= l;
			q.rgbBlue	= l;
		break;
		case 4:
			q.rgbRed	= l;
			q.rgbGreen	= l;
			q.rgbBlue	= l / 4;
		break;
		case 5:
			q.rgbRed	= l;
			q.rgbGreen	= l / 4;
			q.rgbBlue	= l;
		break;
		case 6:
			q.rgbRed	= l / 2;
			q.rgbGreen	= l / 2;
			q.rgbBlue	= l;
		break;
		case 7:
			q.rgbRed	= 255 - ( l / 2 );
			q.rgbGreen	= 255 - ( l / 2 );
			q.rgbBlue	= 255 - ( l / 2 );
		break;
	}

	return q;

}
