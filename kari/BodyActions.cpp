
#include "stdafx.h"
#include "SkeletalViewer.h"
#include "resource.h"
#include <mmsystem.h>

#include <stdio.h>
#include <string>
#include <sstream>

#include <map>
#include <iostream>
#include <vector>

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <math.h>
#define PI 3.1415926535897932384

#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <cstring>

#include <fstream>
#include <Commdlg.h>


std::map<int, map<int, map<int, map<int, int> > > > actionSets;
std::map<int, bool> actionSetsStatus;

void CKARIApp::actionsDetect( bool bBlank, NUI_SKELETON_DATA * pSkel, HWND hWnd, int skeletonNumber ) {

	if(		!checkSkeltonValid( pSkel )
		||	!bodySegmentsMeasure( pSkel )
		) {
		return;
	}

	std::map<int, std::string>	curBodyParts;
	std::map<int, int>			curJoints;
	std::map<int, float>		jointDistances;
	std::map<int, float>		jointAnglesX;
	std::map<int, float>		jointAnglesY;
	
	/**/

	
	bool	go;
	int		distance;
	double	angleX;
	double	angleY;
	float	leftX;
	float	centerX;
	float	rightX;
	int		actionSetNum;

	for( int unsigned i = 0; i < actionsCount; i++ ) {

		go				= true;
		distance		= 0;
		angleX			= 0;
		angleY			= 0;
		leftX			= 0;
		centerX			= 0;
		rightX			= 0;
		actionSetNum	= 0;
		
		curJoints.clear();
		jointDistances.clear();
		curBodyParts.clear();
		
		for( int unsigned i2 = 0; i2 < actionSets[i][AS_REQUIRE].size(); i2++ ) {

			curBodyParts[ curBodyParts.size() ] = s_bodyActions[ actionSets[i][AS_REQUIRE][i2][AS_BODY_ACTION] ];

			switch( actionSets[i][AS_REQUIRE][i2][AS_BODY_ACTION] ) {
				case BA_LEAN_LEFT:				//	lean_left				:	angular body lean left (degrees)
				break;
				case BA_LEAN_RIGHT:				//	lean_right				:	angular body lean right(degrees)
				break;
				case BA_LEAN_FORWARD:			//	lean_forwards			:	angualr body lean forwards (degrees)
				break;
				case BA_LEAN_BACKWARD:			//	lean_backwards			:	angular body lean back (degrees)
				break;
				case BA_TURN_LEFT:				//	turn_left				:	angular amount of left body turn (degrees)
					curJoints[ curJoints.size() ] = NUI_JOINT_SHOULDER_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_ANGLE] ) {
						float&	centerZ		= pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ].z;
						float&	rightZ		= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].z;
						if( centerZ > rightZ ) {
							centerX	= pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ].x;
							rightX	= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].x;
							if( centerX < rightX ) {
								angleX	= atan2( (double)( centerZ - rightZ ), (double)( centerX - rightX ) ) * 180 / PI;
							}
						}
					}
				break;
				case BA_TURN_RIGHT:				//	turn_right				:	angular amount of right body turn(degrees)
					curJoints[ curJoints.size() ] = NUI_JOINT_SHOULDER_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_ANGLE] ) {
						float&	centerZ		= pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ].z;
						float&	leftZ		= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].z;
						if( centerZ > leftZ ) {
							centerX	= pSkel->joints[ NUI_JOINT_SHOULDER_CENTER ].x;
							leftX	= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].x;
							if( centerX > leftX ) {
								angleX	= atan2( (double)( centerZ - leftZ ), (double)( centerX - leftX ) ) * 180 / PI;
							}
						}
					}
				break;
				case BA_HAND_LEFT_FORWARD:		//	left_arm_forwards		:	forward distance from left hand to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	handZ	= pSkel->joints[ NUI_JOINT_HAND_LEFT ].z;
						float&	shouldZ	= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].z;
						jointDistances[ NUI_JOINT_HAND_LEFT ] = ( shouldZ - handZ ) / 0.0254;
							distance	= (int)jointDistances[ NUI_JOINT_HAND_LEFT ];
					}
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_ANGLE] ) {
						float&	handX	= pSkel->joints[ NUI_JOINT_HAND_LEFT ].x;
						float&	handY	= pSkel->joints[ NUI_JOINT_HAND_LEFT ].y;
						float&	handZ	= pSkel->joints[ NUI_JOINT_HAND_LEFT ].z;
						float&	shouldX	= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].x;
						float&	shouldY	= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].y;
						float&	shouldZ	= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].z;
								angleX	= atan2( (double)( shouldZ - handZ ), (double)( shouldX - handX ) ) * 180 / PI;
								angleY	= atan2( (double)( shouldZ - handZ ), (double)( shouldY - handY ) ) * 180 / PI;
						jointAnglesX[ NUI_JOINT_HAND_LEFT ] = angleX;
						jointAnglesY[ NUI_JOINT_HAND_LEFT ] = angleY;
					}
				break;
				case BA_HAND_LEFT_DOWN:			//	left_arm_down			:	downward distance from left hand to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	handY		= pSkel->joints[ NUI_JOINT_HAND_LEFT ].y;
						float&	shouldY		= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].y;
								distance	= (int)( ( shouldY - handY ) / 0.0254 );
					}
				break;
				case BA_HAND_LEFT_UP:			//	left_arm_up				:	upward distance from left hand to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	handY		= pSkel->joints[ NUI_JOINT_HAND_LEFT ].y;
						float&	shouldY		= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].y;
						if( handY > shouldY )
								distance	= (int)( ( handY - shouldY ) / 0.0254 );
					}
				break;
				case BA_HAND_LEFT_OUT:			//	left_arm_out			:	sideways distance from left hand to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	handX		= pSkel->joints[ NUI_JOINT_HAND_LEFT ].x;
						float&	shouldX		= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].x;
						if( handX < shouldX )
							distance		= (int)( ( shouldX - handX ) / 0.0254 );
					}
				break;
				case BA_HAND_LEFT_ACROSS:		//	left_arm_across			:	sideways distance from left hand across body to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_HAND_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	handX		= pSkel->joints[ NUI_JOINT_HAND_LEFT ].x;
						float&	shouldX		= pSkel->joints[ NUI_JOINT_SHOULDER_LEFT ].x;
						if( handX > shouldX )
							distance		= (int)( ( handX - shouldX ) / 0.0254 );
					}
				break;
				case BA_HAND_RIGHT_FORWARD:		//	right_arm_forwards		:	forward distance from right hand to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	handZ		= pSkel->joints[ NUI_JOINT_HAND_RIGHT ].z;
						float&	shouldZ		= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].z;
						jointDistances[ NUI_JOINT_HAND_RIGHT ] = ( shouldZ - handZ ) / 0.0254;
								distance	= (int)jointDistances[ NUI_JOINT_HAND_RIGHT ];
					}
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_ANGLE] ) {
						float&	handX	= pSkel->joints[ NUI_JOINT_HAND_RIGHT ].x;
						float&	handY	= pSkel->joints[ NUI_JOINT_HAND_RIGHT ].y;
						float&	handZ	= pSkel->joints[ NUI_JOINT_HAND_RIGHT ].z;
						float&	shouldX	= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].x;
						float&	shouldY	= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].y;
						float&	shouldZ	= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].z;
								angleX	= atan2( (double)( shouldZ - handZ ), (double)( shouldX - handX ) ) * 180 / PI;
								angleY	= atan2( (double)( shouldZ - handZ ), (double)( shouldY - handY ) ) * 180 / PI;
						jointAnglesX[ NUI_JOINT_HAND_RIGHT ] = angleX;
						jointAnglesY[ NUI_JOINT_HAND_RIGHT ] = angleY;
					}
				break;
				case BA_HAND_RIGHT_DOWN:		//	right_arm_down			:	downward distance from right hand to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	handY		= pSkel->joints[ NUI_JOINT_HAND_RIGHT ].y;
						float&	shouldY		= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].y;
								distance	= (int)( ( shouldY - handY ) / 0.0254 );
					}
				break;
				case BA_HAND_RIGHT_UP:			//	right_arm_up			:	upward distance from right hand to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	handY		= pSkel->joints[ NUI_JOINT_HAND_RIGHT ].y;
						float&	shouldY		= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].y;
						if( handY > shouldY )
							distance		= (int)( ( handY - shouldY ) / 0.0254 );
					}
				break;
				case BA_HAND_RIGHT_OUT:			//	right_arm_out			:	sideways distance from right hand to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	handX		= pSkel->joints[ NUI_JOINT_HAND_RIGHT ].x;
						float&	shouldX		= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].x;
						if( handX > shouldX )
							distance		= (int)( ( handX - shouldX ) / 0.0254 );
					}
				break;
				case BA_HAND_RIGHT_ACROSS:		//	right_arm_across		:	sideways distance from right hand across body to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_HAND_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	handX		= pSkel->joints[ NUI_JOINT_HAND_RIGHT ].x;
						float&	shouldX		= pSkel->joints[ NUI_JOINT_SHOULDER_RIGHT ].x;
						if( handX < shouldX )
							distance		= (int)( ( shouldX - handX ) / 0.0254 );
					}
				break;
				case BA_FOOT_LEFT_FORWARD:		//	left_foot_forwards		:	forward distance from left hip to foot (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	footZ		= pSkel->joints[ NUI_JOINT_FOOT_LEFT ].z;
						float&	hipZ		= pSkel->joints[ NUI_JOINT_HIP_LEFT ].z;
								distance	= (int)( ( hipZ - footZ ) / 0.0254 );
					}
				break;
				case BA_FOOT_LEFT_OUT:			//	left_foot_sideways		:	sideways distance from left hip to foot (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	footX		= pSkel->joints[ NUI_JOINT_FOOT_LEFT ].x;
						float&	hipX		= pSkel->joints[ NUI_JOINT_HIP_LEFT ].x;
						if( footX < hipX )
							distance		= (int)( ( hipX - footX ) / 0.0254 );
					}
				break;
				case BA_FOOT_LEFT_ACROSS:		//	right_arm_across		:	sideways distance from right hand across body to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	footX		= pSkel->joints[ NUI_JOINT_FOOT_LEFT ].x;
						float&	hipX		= pSkel->joints[ NUI_JOINT_HIP_LEFT ].x;
						if( footX > hipX )
							distance		= (int)( ( footX - hipX ) / 0.0254 );
					}
				break;
				case BA_FOOT_LEFT_BACKWARD:		//	left_foot_backwards		:	backwards distance from left hip to foot (inches)
				break;
				case BA_FOOT_LEFT_UP:			//	left_foot_up			:	height of left foot above other foot on ground (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_LEFT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	leftY		= pSkel->joints[ NUI_JOINT_FOOT_LEFT ].y;
						float&	rightY		= pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].y;
						if( leftY > rightY  )
							distance		= (int)( ( leftY - rightY ) / 0.0254 );
					}
				break;
				case BA_FOOT_RIGHT_FORWARD:		//	right_foot_forwards		:	forward distance from right hip to foot (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	footZ		= pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].z;
						float&	hipZ		= pSkel->joints[ NUI_JOINT_HIP_RIGHT ].z;
								distance	= (int)( ( hipZ - footZ ) / 0.0254 );
					}
				break;
				case BA_FOOT_RIGHT_OUT:			//	right_foot_sideways		:	sideways distance from right hip to foot (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	footX		= pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].x;
						float&	hipX		= pSkel->joints[ NUI_JOINT_HIP_RIGHT ].x;
						if( footX > hipX )
							distance		= (int)( ( footX - hipX ) / 0.0254 );
					}
				break;
				case BA_FOOT_RIGHT_ACROSS:		//	right_arm_across		:	sideways distance from right hand across body to shoulder (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	footX		= pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].x;
						float&	hipX		= pSkel->joints[ NUI_JOINT_HIP_RIGHT ].x;
						if( footX < hipX )
							distance		= (int)( ( hipX - footX ) / 0.0254 );
					}
				break;
				case BA_FOOT_RIGHT_BACKWARD:	//	right_foot_backwards	:	backwards distance from right hip to foot (inches)
				break;
				case BA_FOOT_RIGHT_UP:			//	right_foot_up			:	height of right foot above other foot on ground (inches)
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					if( actionSets[i][AS_REQUIRE][i2][AS_USE_DISTANCE] ) {
						float&	leftY		= pSkel->joints[ NUI_JOINT_FOOT_LEFT ].y;
						float&	rightY		= pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].y;
						if( rightY > leftY )
							distance		= (int)( ( rightY - leftY ) / 0.0254 );
					}
				break;
				case BA_JUMP:					//	jump					:	height of both feet above ground (inches)
					/**/
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_LEFT;

					distance = getSkeletonElevation( pSkel );
					
					SetDlgItemInt( m_hWnd, IDC_PERSON_ELEVATION, distance, FALSE );

					/**/
				break;
				case BA_CROUCH:					//	crouch					:	crouch distance: calculated as current height subtracted from standing height (inches)
					
					if( getSkeletonElevation( pSkel ) < 10 ) {

						curJoints[ curJoints.size() ] = NUI_JOINT_HEAD;
						
						float footY;

						footY = pSkel->joints[ NUI_JOINT_FOOT_LEFT ].y < pSkel->joints[ NUI_JOINT_FOOT_RIGHT ].y
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
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_RIGHT;
					curJoints[ curJoints.size() ] = NUI_JOINT_FOOT_LEFT;
				break;
			}

					if( actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MIN]	&& distance	< actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MIN]	)	{	go = false;	break;	}
			else	if( actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MAX]	&& distance	> actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MAX]	)	{	go = false;	break;	}
			else	if( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X]	&& angleX	< actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X]		)	{	go = false;	break;	}
			else	if( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_X]	&& angleX	> actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_X]		)	{	go = false;	break;	}
			else	if( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y]	&& angleY	< actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y]		)	{	go = false;	break;	}
			else	if( actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_Y]	&& angleY	> actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_Y]		)	{	go = false;	break;	}
			
		}

		if( go == true ) {
			
			//NUI_SKELETON_DATA * pSkel
			NUI_SKELETON_DATA * prevSkel = &previousSkeltonFrame.SkeletonData[ skeletonNumber ];

			for( int unsigned i2 = 0; i2 < actionSets[i][AS_EXECUTE].size(); i2++ ) {
				/**/

				if(		actionSets[i][AS_EXECUTE][i2][AS_MOUSE_TRACK] 
					||	actionSets[i][AS_EXECUTE][i2][AS_WINDOW_HOLD] 
						) {
					
					float dDiff		= jointDistances[ curJoints[0] ] - actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MIN];
					float dRange	= actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MAX] - actionSets[i][AS_REQUIRE][i2][AS_DISTANCE_MIN];
					float dStrength	= dDiff / dRange;
					
					// get should point from hand point
					float& shouldX	= pSkel->joints[ curJoints[0] - 3 ].x;
					float& shouldY	= pSkel->joints[ curJoints[0] - 3 ].y;
					float& shouldZ	= pSkel->joints[ curJoints[0] - 3 ].z;
					
					float& handX	= pSkel->joints[ curJoints[0] ].x;
					float& handY	= pSkel->joints[ curJoints[0] ].y;
					float& handZ	= pSkel->joints[ curJoints[0] ].z;
					
					int signed offsetX;
					int signed offsetY;
					
					int xGoal, yGoal;
					float xDiff, yDiff, yRange, xPercent, yPercent, xRange, xAngle, yAngle, prevAngleX, prevAngleY, anglePixelsX, anglePixelsY;
					
					Vector4& prevShould	= prevSkel->joints[ curJoints[0] - 3 ];
					Vector4& prevHand	= prevSkel->joints[ curJoints[0] ];

					switch( actionSets[i][AS_EXECUTE][i2][AS_MOUSE_TRACK] ) {
						case ACV_ABSOLUTE:
							xAngle		= jointAnglesX[ curJoints[0] ];
							yAngle		= jointAnglesY[ curJoints[0] ];
							
							xRange		= actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_X] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X];
							xDiff		= xAngle - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X];
							xPercent	= xDiff / xRange;
							
							yRange		= actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_Y] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y];
							yDiff		= yAngle - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y];
							yPercent	= yDiff / yRange;
							
							xGoal		= screenSizeX * xPercent;
							yGoal		= screenSizeY * ( 1 - yPercent );
							
							SetCursorPos( xGoal, yGoal );
						break;
						case ACV_PUSH:
							offsetX = (int)( ( ( handX - shouldX ) / 0.0254 ) / dStrength );
							offsetY = (int)( ( ( shouldY - handY ) / 0.0254 ) / dStrength );
							if( offsetX != 0 || offsetY != 0 )
								mouseMove( offsetX, offsetY );
						break;
						case ACV_RELATIVE:
							/*/
							setup so that movement range is mapped to detection area and screen size, same as absolute.
							/**/
							
							prevAngleX	= atan2( (double)( prevShould.z - prevHand.z ), (double)( prevShould.x - prevHand.x ) ) * 180 / PI;
							prevAngleY	= atan2( (double)( prevShould.z - prevHand.z ), (double)( prevShould.y - prevHand.y ) ) * 180 / PI;
							
							xRange		= actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_X] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X];
							yRange		= actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_Y] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y];
							
							anglePixelsX = screenSizeX / xRange;
							anglePixelsY = screenSizeY / yRange;
							
							xAngle		= jointAnglesX[ curJoints[0] ];
							yAngle		= jointAnglesY[ curJoints[0] ];

							offsetX		= ( xAngle - prevAngleX ) * anglePixelsX;
							offsetY		= ( prevAngleY - yAngle ) * anglePixelsY;

							if( offsetX != 0 || offsetY != 0 ) {
								m_Log << "dragx : " << xAngle << "\t" << prevAngleX << "\t" << ( xAngle - prevAngleX ) << "\t" << anglePixelsX << "\t" << offsetX << "\n";
								m_Log << "dragy : " << yAngle << "\t" << prevAngleY << "\t" << ( yAngle - prevAngleY ) << "\t" << anglePixelsY << "\t" << offsetY << "\n";
								mouseMove( offsetX, offsetY );
							}
							
						break;
						case ACV_DRAG:
							handX /= 0.0254;
							handY /= 0.0254;
							handX = (int)handX;
							handY = (int)handY;
							if( actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_Y] == NULL ) {
								actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_X] = handX;
								actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_Y] = handY;
							} else {
								offsetX = handX - actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_X];
								offsetY = actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_Y] - handY;
								if( offsetX != 0 || offsetY != 0 ) {
									if( dStrength != 0 ) {
										if( dStrength < .02 )	dStrength = .02;
										if( offsetX != 0 )		offsetX /= dStrength;
										if( offsetY != 0 )		offsetY /= dStrength;
									}
									mouseMove( offsetX, offsetY );
									actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_X] = handX;
									actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_Y] = handY;
								}
							}
						break;
					}

					switch( actionSets[i][AS_EXECUTE][i2][AS_WINDOW_HOLD] ) {
						case ACV_ABSOLUTE:
							xAngle		= jointAnglesX[ curJoints[0] ];
							yAngle		= jointAnglesY[ curJoints[0] ];
							
							xRange		= actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_X] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X];
							xDiff		= xAngle - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X];
							xPercent	= xDiff / xRange;
							
							yRange		= actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_Y] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y];
							yDiff		= yAngle - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y];
							yPercent	= yDiff / yRange;
							
							xGoal		= screenSizeX * xPercent;
							yGoal		= screenSizeY * ( 1 - yPercent );
							
							placeActiveWindow( xGoal, yGoal );							
						break;
						case ACV_PUSH:
							offsetX = (int)( ( ( handX - shouldX ) / 0.0254 ) / dStrength );
							offsetY = (int)( ( ( shouldY - handY ) / 0.0254 ) / dStrength );
							if( offsetX != 0 || offsetY != 0 )
								moveActiveWindow( offsetX, offsetY );
						break;
						case ACV_RELATIVE:
							
							prevAngleX	= atan2( (double)( prevShould.z - prevHand.z ), (double)( prevShould.x - prevHand.x ) ) * 180 / PI;
							prevAngleY	= atan2( (double)( prevShould.z - prevHand.z ), (double)( prevShould.y - prevHand.y ) ) * 180 / PI;
							
							xRange		= actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_X] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_X];
							yRange		= actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MAX_Y] - actionSets[i][AS_REQUIRE][i2][AS_ANGLE_MIN_Y];
							
							anglePixelsX = screenSizeX / xRange;
							anglePixelsY = screenSizeY / yRange;
							
							xAngle		= jointAnglesX[ curJoints[0] ];
							yAngle		= jointAnglesY[ curJoints[0] ];

							offsetX		= ( xAngle - prevAngleX ) * anglePixelsX;
							offsetY		= ( prevAngleY - yAngle ) * anglePixelsY;

							if( offsetX != 0 || offsetY != 0 ) {
								m_Log << "dragx : " << xAngle << "\t" << prevAngleX << "\t" << ( xAngle - prevAngleX ) << "\t" << anglePixelsX << "\t" << offsetX << "\n";
								m_Log << "dragy : " << yAngle << "\t" << prevAngleY << "\t" << ( yAngle - prevAngleY ) << "\t" << anglePixelsY << "\t" << offsetY << "\n";
								moveActiveWindow( offsetX, offsetY );
							}
						break;
						case ACV_DRAG:
							handX /= 0.0254;
							handY /= 0.0254;
							handX = (int)handX;
							handY = (int)handY;
							if( actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_Y] == NULL ) {
								actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_X] = handX;
								actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_Y] = handY;
							} else {
								offsetX = handX - actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_X];
								offsetY = actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_Y] - handY;
								if( offsetX != 0 || offsetY != 0 ) {
									if( dStrength != 0 ) {
										if( dStrength < .02 )	dStrength = .02;
										if( offsetX != 0 )		offsetX /= dStrength;
										if( offsetY != 0 )		offsetY /= dStrength;
									}
									moveActiveWindow( offsetX, offsetY );
									actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_X] = handX;
									actionSets[i][AS_EXECUTE][i2][AS_JOINT_PREV_Y] = handY;
								}
							}
						break;
					}
					
				} else 
				/**/
				if( !actionSetsStatus[i] ) {
					
					m_Log << "on : " << i << "\n";
					for( int cb = 0; cb < curBodyParts.size(); cb++ ) {
						m_Log << "\t" << curBodyParts[ cb ] << "\n";
					}
					m_Log << "\t";
					m_Log << "\td:" << distance;
					m_Log << "\taX:" << angleX;
					m_Log << "\taY:" << angleY;
					m_Log << "\n";

					if( actionSets[i][AS_EXECUTE][i2][AS_KEY_TAP] ) {
						m_Log << "keyTap : " << actionSets[i][AS_EXECUTE][i2][AS_KEY_TAP] << "\n";
						_beginthread( keyTap, 0, (void*)actionSets[i][AS_EXECUTE][i2][AS_KEY_TAP] );
					}
					
					if( actionSets[i][AS_EXECUTE][i2][AS_KEY_HOLD] ) {
						m_Log << "keyPress : " << actionSets[i][AS_EXECUTE][i2][AS_KEY_HOLD] << "\n";
						_beginthread( keyPress, 0, (void*)actionSets[i][AS_EXECUTE][i2][AS_KEY_HOLD] );
					}

					if( actionSets[i][AS_EXECUTE][i2][AS_KEY_PRESS] ) {
						m_Log << "keyPress : " << actionSets[i][AS_EXECUTE][i2][AS_KEY_PRESS] << "\n";
						_beginthread( keyPress, 0, (void*)actionSets[i][AS_EXECUTE][i2][AS_KEY_PRESS] );
					}

					if( actionSets[i][AS_EXECUTE][i2][AS_KEY_RELEASE] ) {
						m_Log << "keyRelease : " << actionSets[i][AS_EXECUTE][i2][AS_KEY_RELEASE] << "\n";
						_beginthread( keyRelease, 0, (void*)actionSets[i][AS_EXECUTE][i2][AS_KEY_RELEASE] );
					}
					
					if( actionSets[i][AS_EXECUTE][i2][AS_MOUSE_TAP] ) {

						int down;
						int up;
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
						int down;
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

			actionSetsStatus[i]	= true;

			for( int unsigned cj = 0; cj < curJoints.size(); cj++ ) {
				g_JointColorPower[ curJoints[ cj ] ] = 1;
			}

		} else if( actionSetsStatus[i] ) {

			actionSetsStatus[i] = false;
			
			m_Log << "off : " << i << "\n";
			for( int cb = 0; cb < curBodyParts.size(); cb++ ) {
				m_Log << "\t" << curBodyParts[ cb ] << "\n";
			}
			m_Log << "\t";
			m_Log << "\td:" << distance;
			m_Log << "\taX:" << angleX;
			m_Log << "\taY:" << angleY;
			m_Log << "\n";

			for( int unsigned i2 = 0; i2 < actionSets[i][AS_EXECUTE].size(); i2++ ) {
				
				if( actionSets[i][AS_EXECUTE][i2][AS_KEY_HOLD] ) {
					m_Log << "keyRelease : " << actionSets[i][AS_EXECUTE][i2][AS_KEY_HOLD] << "\n";
					_beginthread( keyRelease, 0, (void*)actionSets[i][AS_EXECUTE][i2][AS_KEY_HOLD] );
				}

				if( actionSets[i][AS_EXECUTE][i2][AS_MOUSE_HOLD] ) {
					int up;
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