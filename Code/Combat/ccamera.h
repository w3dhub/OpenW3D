/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/ccamera.h                             $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/10/01 2:08p                                              $*
 *                                                                                             *
 *                    $Revision:: 62                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef CCAMERA_H
#define CCAMERA_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef CAMERA_H
	#include "camera.h"
#endif

#ifndef WWSTRING_H
	#include "wwstring.h"
#endif

class CCameraProfileClass;
class Listener3DClass;

/*
** CCameraClass (Commando Camera Class)
*/
class CCameraClass : public CameraClass
{
public:
	// Constructor & Destructor
	CCameraClass();
	virtual ~CCameraClass();

	// Get all the profiles loaded
	static	void	Init( void );
	static	void	Shutdown( void );

	// Save and Load
	virtual	bool	Save( ChunkSaveClass &csave ) override;
	virtual	bool	Load( ChunkLoadClass &cload ) override;

	// Function to update the camera's parameters each frame.
	virtual void	Update();

	bool				Is_In_Cinematic( void )					{ return HostModel != NULL; }

	bool				Is_Valid( void )							{ return IsValid; }

	const Vector2 &Get_Camera_Target_2D_Offset()			{ return CameraTarget2DOffset; }

	void				Set_Anchor_Position( Vector3 pos );
	void				Force_Look( const Vector3 & target );

	// Profile access
	void				Use_Profile( const char * name );
	void				Use_Default_Profile( void );
	void				Set_Profile_Height( float height );
	void				Set_Profile_Distance( float distance );
	float				Get_Profile_Zoom( void );

	void				Force_Heading( float heading )		{ Heading = heading; }
	float				Get_Heading( void )						{ return Heading; }
	void				Set_Tilt( float tilt )					{ Tilt = tilt; }
	float				Get_Tilt( void )							{ return Tilt; }

	void				Set_Lerp_Time( float time );
	bool				Is_Lerping( void )						{ return LerpTimeTotal != 0; }

	void				Set_Host_Model( RenderObjClass * host );
	bool				Is_Using_Host_Model( void )			{ return (HostModel != NULL); }

	void				Enable_2D_Targeting( bool on_off )	{ Enable2DTargeting = on_off; }
	bool				Is_2D_Targeting( void )					{ return Enable2DTargeting; }

	void				Enable_Weapon_Help( bool on_off )	{ EnableWeaponHelp = on_off; }
	bool				Is_Weapon_Help_Enabled( void )		{ return EnableWeaponHelp; }

	// Sniper Mode
	bool				Draw_Sniper( void );
	void				Cinematic_Sniper_Control( bool enabled, float zoom );
	void				Set_Is_Star_Sniping( bool yes );
	bool				Is_Star_Sniping( void )					{ return IsStarSniping; }
	float				Get_Sniper_Zoom( void )					{ return SniperZoom; }
	void				Set_Sniper_Distance( float dist );
	float				Get_Sniper_Distance( void )			{ return SniperDistance; }

	Vector3			Get_First_Person_Offset_Tweak( void );
	void				Reset_First_Person_Offset_Tweak( void );

	// Snap Shot Mode
	void				Set_Snap_Shot_Mode( bool mode )		{ SnapShotMode = mode ? SNAPSHOT_ON : SNAPSHOT_OFF; }
	bool				Is_Snap_Shot_Mode( void )				{ return SnapShotMode == SNAPSHOT_ON; }

protected:
	
	// Camera Host Model
	RenderObjClass	 *		HostModel;

	// Camera Anchor Position
	Vector3					AnchorPosition;
	bool						IsValid;

	// Current orientation of the camera
	float						Tilt;
	float						Heading;
	float						DistanceFraction;			

	bool						Enable2DTargeting;
	float						LagPersistTimer;
	bool						DisableLag;

	// WeaponHelp
	bool						EnableWeaponHelp;
	float						WeaponHelpTimer;
	int						WeaponHelpTargetID;	// Should probably be changed to a ObjRef

	Vector3					StarTargetingPosition;
	Vector2					CameraTarget2DOffset;

	// Linear Interpolation
	float						LerpTimeTotal;
	float						LerpTimeRemaining;
	Vector3					LastAnchorPosition;
	float						LastHeading;

	// Parameter profile
	CCameraProfileClass * CurrentProfile;
	CCameraProfileClass * LastProfile;
	CCameraProfileClass * DefaultProfile;
	StringClass				CurrentProfileName;
	StringClass				LastProfileName;
	StringClass				DefaultProfileName;

	float						NearClipPlane;
	float						FarClipPlane;

	// Sniper Mode
	bool						IsStarSniping;
	bool						WasStarSniping;
	float						SniperZoom;
	float						SniperDistance;
	Listener3DClass *		SniperListener;
	bool						CinematicSnipingEnabled;
	float						CinematicSnipingDesiredZoom;

	enum {
		SNAPSHOT_OFF,
		SNAPSHOT_ON,
		SNAPSHOT_PROGRESS,
	};
	int						SnapShotMode;

	/*
	**
	*/
	Vector3					Get_Camera_Pos( const Vector3 & offset, 
													float distance, Vector3 * intermediate_pos );
	void						Use_Host_Model( void );
	void						Handle_Input();	

	// Targeting
	bool				Determine_Targeting_Position( void );	
	void				Apply_Weapon_Help( void );
	void				Update_Sniper_Listener_Pos( void );
	void				Handle_Snap_Shot_Mode( void );
};


#endif		// CCAMERA_H
