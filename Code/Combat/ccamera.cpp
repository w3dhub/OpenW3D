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
 *                     $Archive:: /Commando/Code/Combat/ccamera.cpp                           $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/14/02 10:44a                                              $*
 *                                                                                             *
 *                    $Revision:: 162                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "ccamera.h"
#include "timemgr.h"
#include "physicalgameobj.h"
#include "physcoltest.h"
#include "pscene.h"
#include "combat.h"
#include "input.h"
#include "debug.h"
#include "assets.h"
#include "slist.h"
#include <string.h>
#include "vector3.h"
#include "phys.h"
#include "vehicle.h"
#include "soldier.h"
#include "weapons.h"
#include "gameobjmanager.h"
#include "htree.h"
#include "hanim.h"
#include "hudinfo.h"
#include "Listener.h"
#include "WWAudio.h"
#include "SoundScene.h"
#include "wwprofile.h"
#include "diaglog.h"
#include "gametype.h"
#include "wwphysids.h"
#include "buildingaggregate.h"
#include "persistfactory.h"


#define MIN_FOV				0.02f
#define MAX_FOV				2.6f

#define CCAMERA_NEARZ						0.26f				// near clip plane distance
#define CCAMERA_FARZ							300.0f			// far clip plane distance
#define CCAMERA_SHRINK_NEARZ_DIST		0.5f				// third person distance below which we start shrinking nearz
#define CCAMERA_MIN_NEARZ					0.2f				// how small nearz gets when we're up against a wall


Vector3	FirstPersonOffsetTweak( 0, 0, 0 );

/*
** Speed at which the camera pushes back out to its default position
** after a collision.  1.0 = goes all the way in 1 second
*/
const float CAMERA_UNWIND_SPEED = 1.0f;

/*
** Camera Profiles	
*/
//SList<CCameraProfileClass>	  				ProfileList;
HashTemplateClass<StringClass, CCameraProfileClass*> ProfileHash;

#define	CAMERAS_INI_FILENAME					"CAMERAS.INI"
#define	SECTION_PROFILE_LIST					"Profile_List"
#define	ENTRY_NAME								"Name"
#define	ENTRY_FOV								"FOV"
#define	ENTRY_HEIGHT							"Height"
#define	ENTRY_VIEWTILT							"ViewTilt"
#define	ENTRY_TRANSLATIONTILT				"TranslationTilt"
#define	ENTRY_DISTANCE							"Distance"
#define	ENTRY_LAG_UP							"LagUp"
#define	ENTRY_LAG_LEFT							"LagLeft"
#define	ENTRY_LAG_FORWARD						"LagForward"
#define	ENTRY_TILTTWEAK						"TiltTweak"

/*
**
*/
class CCameraProfileClass 
{
public:
	static void Init( void );
	static void	Shutdown( void );

	void			Set_Zoom( float amount );
	float			Get_Zoom( void );

	void			Set_Height( float height )			{ Height = height; }
	float			Get_Height( void )					{ return Height; }

	void			Set_Distance( float distance )	{ Distance = distance; }
	float			Get_Distance( void )					{ return Distance; }

protected:
	CCameraProfileClass( void );
	~CCameraProfileClass( void );
	const CCameraProfileClass & operator = ( const CCameraProfileClass & );

	void	Lerp( const CCameraProfileClass & a, const CCameraProfileClass & b, float lerp );

//	StringClass	Name;
	float			FOV;						// field of view for the camera
	float			Height;					// height above the origin of "focus" object
	float			ViewTilt;				// default tilt of the camera
	float			TiltTweak;				// default tilt tweak of the camera
	float			TranslationTilt;		// tilt of translation vector for the camera (off of the z axis)
	float			Distance;				// how far back the camera wants to be normally
	Vector3		Lag;						// the camera lag

	static bool _ProfilesInitted;

	static		CCameraProfileClass	*	Find( const char * name );
	friend		CCameraClass;
};

bool CCameraProfileClass::_ProfilesInitted = false;

/*
**
*/
CCameraProfileClass::CCameraProfileClass( void ) : 
	FOV( DEG_TO_RADF( 65.0f ) ),
	Height( 1.95f ),
	ViewTilt( DEG_TO_RADF(20.0f) ),
	TiltTweak( 0.6f ),
	TranslationTilt( DEG_TO_RADF( 19.9f ) ),
	Distance( 3.1f ),
	Lag( 0,0,0 )	
{
}

CCameraProfileClass::~CCameraProfileClass( void )
{
}

void	CCameraProfileClass::Set_Zoom( float amount )
{
	amount = WWMath::Sqrt(amount);
	amount = WWMath::Clamp( (1-amount), 0, 1 );
	FOV = MIN_FOV + ((0.8f - MIN_FOV) * amount);
}

float CCameraProfileClass::Get_Zoom( void )
{
	// TSS - by my reckoning this is the actual zoom factor
	return 0.8 / FOV;
}

#define	Get_Camera_Profile_Radians( v, e )			\
	profile->v = DEG_TO_RADF( camerasINI->Get_Float(	section_name, e, RAD_TO_DEGF( profile->v ) ) )
#define	Get_Camera_Profile_Float( v, e )			\
	profile->v = camerasINI->Get_Float(	section_name, e, profile->v )
//#define	Get_Camera_Profile_String( v, e )			\
//	profile->v = camerasINI->Get_String( section_name, e, profile->v )

void CCameraProfileClass::Init( void )
{	
	if (_ProfilesInitted) {
		CCameraProfileClass::Shutdown();
	}

	INIClass	* camerasINI = Get_INI( CAMERAS_INI_FILENAME );
	if (camerasINI != NULL) {
		WWASSERT( camerasINI && camerasINI->Section_Count() > 0 );

		int count, entry;
		count =  camerasINI->Entry_Count( SECTION_PROFILE_LIST );
		for ( entry = 0; entry < count; entry++ )	{
			StringClass	section_name(0,true);
			camerasINI->Get_String(	section_name, SECTION_PROFILE_LIST,
													camerasINI->Get_Entry( SECTION_PROFILE_LIST, entry) );

			CCameraProfileClass * profile = new CCameraProfileClass();
			WWASSERT( profile );
			StringClass name(true);
			camerasINI->Get_String( name, section_name, ENTRY_NAME, name );
//			Get_Camera_Profile_String(		name,					ENTRY_NAME					);
			Get_Camera_Profile_Radians(	FOV,					ENTRY_FOV					);
			Get_Camera_Profile_Float(		Height,				ENTRY_HEIGHT				);
			Get_Camera_Profile_Radians(	ViewTilt,			ENTRY_VIEWTILT				);
			Get_Camera_Profile_Float(		TiltTweak,			ENTRY_TILTTWEAK			);
			Get_Camera_Profile_Radians(	TranslationTilt,	ENTRY_TRANSLATIONTILT	);
			Get_Camera_Profile_Float(		Distance,			ENTRY_DISTANCE				);

			Get_Camera_Profile_Float(		Lag.Y,				ENTRY_LAG_UP				);
			Get_Camera_Profile_Float(		Lag.X,				ENTRY_LAG_LEFT				);
			Get_Camera_Profile_Float(		Lag.Z,				ENTRY_LAG_FORWARD			);

//			ProfileList.Add_Tail( profile );
			// Convert to lower case
			_strlwr(name.Peek_Buffer());
			ProfileHash.Insert(name,profile);
		}

		Release_INI( camerasINI );
	} else {
		Debug_Say(("CCameraProfileClass::Init - Unable to load %s\n", CAMERAS_INI_FILENAME));
	}	
	_ProfilesInitted = true;
}

void	CCameraProfileClass::Shutdown( void )
{
//	while( ProfileList.Get_Count() ) {
//		delete	ProfileList.Remove_Head();
//	}
	HashTemplateIterator<StringClass,CCameraProfileClass*> ite(ProfileHash);
	for (ite.First();!ite.Is_Done();ite.Next()) {
		CCameraProfileClass * profile=ite.Peek_Value();
		delete profile;
	}
	ProfileHash.Remove_All();
	_ProfilesInitted = false;
}

const CCameraProfileClass & CCameraProfileClass::operator = ( const CCameraProfileClass & src )
{
//	Name					= src.Name;
	FOV					= src.FOV;
	Height				= src.Height;
	ViewTilt				= src.ViewTilt;
	TiltTweak			= src.TiltTweak;
	TranslationTilt	= src.TranslationTilt;
	Distance				= src.Distance;
	Lag					= src.Lag;
	return *this;
}

float	RadianLerp( float a, float b, float lerp ) 
{
	float diff = b - a;					// diff should be -180 .. 180
	diff = WWMath::Wrap( diff, DEG_TO_RADF( -180 ), DEG_TO_RADF( 180 ) );
//	diff += DEG_TO_RAD( 360 ) * WWMath::Sign( -diff );		// Take the long way
	return a + diff * lerp;
}

void	CCameraProfileClass::Lerp( const CCameraProfileClass & a, const CCameraProfileClass & b, float lerp )
{
	*this = a;		// Start by taking A
	FOV				= WWMath::Lerp( a.FOV,					b.FOV,					lerp );
	Height			= WWMath::Lerp( a.Height, 				b.Height,				lerp );
	ViewTilt			= WWMath::Lerp( a.ViewTilt,			b.ViewTilt,				lerp );
	TiltTweak		= WWMath::Lerp( a.TiltTweak,			b.TiltTweak, 			lerp );
	TranslationTilt= WWMath::Lerp( a.TranslationTilt,	b.TranslationTilt,	lerp );
	Distance			= WWMath::Lerp( a.Distance,			b.Distance,				lerp );
}

CCameraProfileClass	*	CCameraProfileClass::Find( const char * name )
{
	// Compare lower case strings
	char tmp[256];
	strncpy(tmp,name,sizeof(tmp));
	
	_strlwr(tmp);
	StringClass tmp_string(tmp,true);
	CCameraProfileClass* profile = ProfileHash.Get(tmp_string);
	return profile;

//	SLNode<CCameraProfileClass> *node;
//	for (	node = ProfileList.Head(); node; node = node->Next()) {
//		if ( !stricmp( node->Data()->Name, name ) ) {
//			return node->Data();
//		}
//	}
//	return NULL;
}


/*
** CCameraClass 
*/
CCameraClass::CCameraClass() : 
	CameraClass(),
	HostModel( NULL ),
	AnchorPosition(0,0,0),
	IsValid( false ),
	Tilt( 0 ),
	Heading( 0 ),
	DistanceFraction(1.0f),
	Enable2DTargeting( false ),
	EnableWeaponHelp( false ),
	CameraTarget2DOffset( 0.5f, 0.5f ),
	LerpTimeTotal( 0.0f ),
	LerpTimeRemaining( 0.0f ),
	LastAnchorPosition(0,0,0),
	LastHeading( 0 ),
	CurrentProfile( NULL ),
	LastProfile( NULL ),
	DefaultProfile( NULL ),
	NearClipPlane( CCAMERA_NEARZ ),
	FarClipPlane( CCAMERA_FARZ ),
	IsStarSniping( false ),
	WasStarSniping( false ),
	CinematicSnipingEnabled( false ),
	CinematicSnipingDesiredZoom( 0 ),
	SniperZoom( 0 ),
	SniperDistance( 0 ),
	SniperListener( NULL ),
	SnapShotMode( SNAPSHOT_OFF ),
	WeaponHelpTimer( 0 ),
	WeaponHelpTargetID( 0 ),
	LagPersistTimer( 0 ),
	DisableLag( false )
{
	Set_Clip_Planes( NearClipPlane, FarClipPlane );
	Set_View_Plane( DEG_TO_RAD( 90.0 ) );

	SniperListener = new Listener3DClass;

	DefaultProfile = CCameraProfileClass::Find( "default" );
	DefaultProfileName="default";
	Use_Default_Profile(); 
}

CCameraClass::~CCameraClass(void)
{
	Set_Host_Model( NULL );

	//
	//	Remove the sniper listener (if necessary) from the world
	//
	if (	SniperListener != NULL && WWAudioClass::Get_Instance() != NULL &&
			WWAudioClass::Get_Instance()->Get_Sound_Scene() != NULL)
	{
		if ( WWAudioClass::Get_Instance()->Get_Sound_Scene()->Peek_2nd_Listener() != NULL ) {
			WWAudioClass::Get_Instance()->Get_Sound_Scene ()->Set_2nd_Listener( NULL );
		}
	}
	REF_PTR_RELEASE (SniperListener);
}

/*
**
*/
void CCameraClass::Init( void )
{	
	CCameraProfileClass::Init();
}

void	CCameraClass::Shutdown( void )
{
	CCameraProfileClass::Shutdown();
}

/*
** Save and Load
*/
enum	{
	XXXCHUNKID_PARENT							=	416012036,
	CHUNKID_VARIABLES,

	MICROCHUNKID_HOST_MODEL					=	1,				
	MICROCHUNKID_ANCHOR_POSITION,			
	MICROCHUNKID_IS_VALID,					
	MICROCHUNKID_TILT,						
	MICROCHUNKID_HEADING,					
	MICROCHUNKID_DISTANCE_FRACTION,		 
	MICROCHUNKID_ENABLE_2D_TARGETING,	
	MICROCHUNKID_ENABLE_WEAPON_HELP,		
	MICROCHUNKID_WEAPON_HELP_TIMER,		
	MICROCHUNKID_WEAPON_HELP_TARGET_ID,	
	MICROCHUNKID_STAR_TARGETING_POSITION,
	MICROCHUNKID_CAMERA_TARGET_2D_OFFSET,
	MICROCHUNKID_LERP_TIME_TOTAL,			
	MICROCHUNKID_LERP_TIME_REMAINING,	
	MICROCHUNKID_LAST_ANCHOR_POSITION,	
	MICROCHUNKID_LAST_HEADING,				
	MICROCHUNKID_CURRENT_PROFILE,			
	MICROCHUNKID_LAST_PROFILE,				
	MICROCHUNKID_DEFAULT_PROFILE,			
	MICROCHUNKID_NEAR_CLIP_PLANE,			
	MICROCHUNKID_FAR_CLIP_PLANE,			
	MICROCHUNKID_IS_STAR_SNIPING,				
	MICROCHUNKID_WAS_STAR_SNIPING,				
	MICROCHUNKID_SNIPER_ZOOM,				
	MICROCHUNKID_SNIPER_DISTANCE,			
	XXXMICROCHUNKID_IS_SNIPER_LOCKED,		
	XXXMICROCHUNKID_SNIPER_TARGET_NAME,		
	XXXMICROCHUNKID_SNIPER_LISTENER,			
	MICROCHUNKID_SNAP_SHOT_MODE,			
	MICROCHUNKID_CINEMATIC_SNIPING_ENABLED,				
	MICROCHUNKID_CINEMATIC_SNIPING_DESIRED_ZOOM,				
};

//------------------------------------------------------------------------------------
bool	CCameraClass::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_HOST_MODEL,					HostModel );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ANCHOR_POSITION,			AnchorPosition );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_VALID,					IsValid );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_TILT,							Tilt );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_HEADING,						Heading );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DISTANCE_FRACTION,		DistanceFraction );			
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ENABLE_2D_TARGETING,		Enable2DTargeting );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ENABLE_WEAPON_HELP,		EnableWeaponHelp );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_WEAPON_HELP_TIMER,		WeaponHelpTimer );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_WEAPON_HELP_TARGET_ID,	WeaponHelpTargetID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_STAR_TARGETING_POSITION,StarTargetingPosition );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CAMERA_TARGET_2D_OFFSET,CameraTarget2DOffset );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LERP_TIME_TOTAL,			LerpTimeTotal );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LERP_TIME_REMAINING,		LerpTimeRemaining );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LAST_ANCHOR_POSITION,	LastAnchorPosition );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_LAST_HEADING,				LastHeading );
		if ( CurrentProfile != NULL ) {
//			StringClass name = CurrentProfile->Name;
			WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_CURRENT_PROFILE,	CurrentProfileName );
		}
		if ( LastProfile != NULL ) {
//			StringClass name = LastProfile->Name;
			WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_LAST_PROFILE,	LastProfileName );
		}
		if ( DefaultProfile != NULL ) {
//			StringClass name = DefaultProfile->Name;
			WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEFAULT_PROFILE,	DefaultProfileName );
		}

// (gth) don't save and load NearClipPlane and FarClipPlane
//		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_NEAR_CLIP_PLANE,			NearClipPlane );
//		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_FAR_CLIP_PLANE,			FarClipPlane );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_IS_STAR_SNIPING,			IsStarSniping );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_WAS_STAR_SNIPING,			WasStarSniping );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CINEMATIC_SNIPING_ENABLED,	CinematicSnipingEnabled );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CINEMATIC_SNIPING_DESIRED_ZOOM, CinematicSnipingDesiredZoom );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SNIPER_ZOOM,				SniperZoom );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SNIPER_DISTANCE,			SniperDistance );
//		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SNIPER_LISTENER,			SniperListener );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SNAP_SHOT_MODE,			SnapShotMode );
	csave.End_Chunk();

	return true;
}

//------------------------------------------------------------------------------------
bool	CCameraClass::Load( ChunkLoadClass &cload )
{
	StringClass	current_name;
	StringClass	last_name;
	StringClass	default_name;
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK_PTR( cload, MICROCHUNKID_HOST_MODEL,					HostModel );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ANCHOR_POSITION,			AnchorPosition );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_VALID,						IsValid );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_TILT,							Tilt );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_HEADING,						Heading );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DISTANCE_FRACTION,			DistanceFraction );			
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ENABLE_2D_TARGETING,		Enable2DTargeting );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_ENABLE_WEAPON_HELP,		EnableWeaponHelp );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_WEAPON_HELP_TIMER,			WeaponHelpTimer );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_WEAPON_HELP_TARGET_ID,	WeaponHelpTargetID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_STAR_TARGETING_POSITION,	StarTargetingPosition );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CAMERA_TARGET_2D_OFFSET,	CameraTarget2DOffset );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LERP_TIME_TOTAL,			LerpTimeTotal );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LERP_TIME_REMAINING,		LerpTimeRemaining );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LAST_ANCHOR_POSITION,		LastAnchorPosition );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_LAST_HEADING,				LastHeading );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_CURRENT_PROFILE,current_name );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_LAST_PROFILE,	last_name );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEFAULT_PROFILE,default_name );
// (gth) don't save and load NearClipPlane and FarClipPlane
//						READ_MICRO_CHUNK( cload, MICROCHUNKID_NEAR_CLIP_PLANE,			NearClipPlane );
//						READ_MICRO_CHUNK( cload, MICROCHUNKID_FAR_CLIP_PLANE,				FarClipPlane );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_IS_STAR_SNIPING,			IsStarSniping );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_WAS_STAR_SNIPING,			WasStarSniping );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CINEMATIC_SNIPING_ENABLED,	CinematicSnipingEnabled );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CINEMATIC_SNIPING_DESIRED_ZOOM, CinematicSnipingDesiredZoom );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SNIPER_ZOOM,					SniperZoom );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SNIPER_DISTANCE,			SniperDistance );
//						READ_MICRO_CHUNK( cload, MICROCHUNKID_SNIPER_LISTENER,			SniperListener );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SNAP_SHOT_MODE,				SnapShotMode );

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	if ( !current_name.Is_Empty() ) {
		CurrentProfile = CCameraProfileClass::Find( current_name );
		CurrentProfileName=current_name;
	}

	if ( !last_name.Is_Empty() ) {
		LastProfile = CCameraProfileClass::Find( last_name );
		LastProfileName=last_name;
	}

	if ( !default_name.Is_Empty() ) {
		DefaultProfile = CCameraProfileClass::Find( default_name );
		DefaultProfileName=default_name;
	}

	if ( HostModel != NULL ) {
		REQUEST_REF_COUNTED_POINTER_REMAP ((RefCountClass **)&HostModel);

	}

	return true;
}


/*
**
*/
void	CCameraClass::Set_Host_Model( RenderObjClass * host )
{
	DisableLag = true;

	static	StringClass		SavedProfileName;

	// If dropping host 
	if ( HostModel != NULL && host == NULL ) {
		if ( SavedProfileName.Is_Empty() ) {
			Use_Default_Profile();
		} else {
			Use_Profile( SavedProfileName );		// Return to saved profile
		}
	} 

	// If getting host 
	if ( HostModel == NULL && host != NULL ) {
		SavedProfileName = CurrentProfileName;
		Use_Profile( "Cinematic" );
//		CurrentProfile->Set_Zoom( 0 );
		CurrentProfile->FOV = DEG_TO_RADF( 75.0f );
		CinematicSnipingEnabled = false;
	}

	if ( HostModel ) {
		HostModel->Release_Ref();
		IsValid = false;
	}
	HostModel = host;
	if ( HostModel ) {
		HostModel->Add_Ref();
		IsValid = true;
	}

	// If clearing host, force facing to match the star
	if ( host == NULL && COMBAT_STAR != NULL ) {
		Force_Heading( COMBAT_STAR->Get_Facing() );
	}

	// When the camera is controlled, activeate cinematic freeze
	GameObjManager::Activate_Cinematic_Freeze( host != NULL );

}

void	Convert_World_To_Camera( Matrix3D * tm ) 
{
	tm->Rotate_Y( DEG_TO_RADF(-90.0) );
	tm->Rotate_Z( DEG_TO_RADF(-90.0) );
}

void	CCameraClass::Use_Host_Model( void ) 
{
	WWASSERT( HostModel );
	Matrix3D tm = HostModel->Get_Bone_Transform( "CAMERA" );
	Set_Transform( tm );

	if ( !CinematicSnipingEnabled ) {
		CurrentProfile->FOV = DEG_TO_RADF( 75.0f );
	}
	Set_View_Plane( CurrentProfile->FOV );

#ifdef ATI_DEMO_HACK
	static GameObjReference DemoFocusObject;

	/*
	** Focus change key pressed:
	*/
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_RESET)) {
		if (DemoFocusObject.Get_Ptr() == NULL) {

			/*
			** Count the soldiers in the level (ignoring the human controlled commando)
			*/
			int soldier_count = 0;
			SLNode<SmartGameObj> * smart_objnode;
			for (smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); smart_objnode; smart_objnode = smart_objnode->Next()) {
				SmartGameObj * obj = smart_objnode->Data();
				if ((obj->As_SoldierGameObj() != NULL) && (obj->Is_Human_Controlled() == false)) {
					soldier_count++;
				}
			}

			/*
			** Pick a random soldier to watch
			*/
			int random_soldier = rand() % soldier_count;
			soldier_count = 0;

			for (smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); smart_objnode; smart_objnode = smart_objnode->Next()) {
				SmartGameObj * obj = smart_objnode->Data();
				if ((obj->As_SoldierGameObj() != NULL) && (obj->Is_Human_Controlled() == false)) {
					if (soldier_count == random_soldier) {
						DemoFocusObject = obj;
						break;
					}
					soldier_count++;
				}
			}
		} else {

			/*
			** Toggle out of demo focus mode
			*/
			DemoFocusObject = NULL;

		}
	}

	/*
	** Position the camera on this soldier
	*/
	if (DemoFocusObject.Get_Ptr() != NULL) {
		Vector3 position;
		DemoFocusObject.Get_Ptr()->Get_Position(&position);
		Matrix3D camera_tm;
		camera_tm.Look_At(position + Vector3(2.0f,0.0f,1.75f),position + Vector3(0.0f,0.0f,1.0f),0.0f);
		Set_Transform(camera_tm);
	}
#endif
}

/*
**
*/
void	CCameraClass::Set_Anchor_Position( Vector3 pos )	
{ 
	AnchorPosition = pos; 

	// If this is the first anchor point, don't lag the camera
	if ( !IsValid ) {
		LastAnchorPosition = pos; 
		IsValid = true;
	}
}

/*
**
*/
void	Ignore_Star_And_Vehicle( void ) 
{
	if (COMBAT_STAR != NULL) {

		COMBAT_STAR->Peek_Physical_Object()->Inc_Ignore_Counter();
		VehicleGameObj * vehicle = COMBAT_STAR->Get_Profile_Vehicle();
		if ( vehicle ) {
			vehicle->Peek_Physical_Object()->Inc_Ignore_Counter();
		}
	}
}

void	Unignore_Star_And_Vehicle( void ) 
{
	if (COMBAT_STAR != NULL) {

		COMBAT_STAR->Peek_Physical_Object()->Dec_Ignore_Counter();
		VehicleGameObj * vehicle = COMBAT_STAR->Get_Profile_Vehicle();
		if ( vehicle ) {
			vehicle->Peek_Physical_Object()->Dec_Ignore_Counter();
		}
	}
}

/*
**
*/
void CCameraClass::Update()
{
	Handle_Input();

	WWASSERT( CurrentProfile );

	if ( SnapShotMode != SNAPSHOT_OFF ) {
		Handle_Snap_Shot_Mode();
		return;
	}

	if ( Is_Using_Host_Model()	) {	// if using a host model, update the
		Use_Host_Model();				// camera from it
		return;
	}

	Vector3	anchor_position = AnchorPosition;
	float		camera_heading  =	Heading;


	CCameraProfileClass	profile;

	bool interpolating = false;

	if ( LerpTimeTotal ) {
		interpolating = true;

		float frame_time = TimeManager::Get_Frame_Seconds();
		float lerp = WWMath::Clamp( LerpTimeRemaining / LerpTimeTotal, 0, 1 );

		LerpTimeRemaining -= frame_time;
		if ( LerpTimeRemaining <= 0 ) {
			LerpTimeRemaining = 0.0f;
			LerpTimeTotal = 0.0f;

			// Set up last, so out Lag code doesn't use the old
			LastAnchorPosition = anchor_position;
			LastHeading	= Heading;	
		}
		profile = *CurrentProfile;

		if ( LastProfile ) {
			profile.Lerp( *CurrentProfile, *LastProfile, lerp );
		}

		anchor_position = ::Lerp( anchor_position, LastAnchorPosition, lerp );

		camera_heading	 =	RadianLerp( camera_heading, LastHeading, lerp );

	} else {
		profile = *CurrentProfile;
		LastProfile = CurrentProfile;
		LastProfileName=CurrentProfileName;

		if ( profile.Lag.Length() > 0 ) {
			// This is an attempt to not lag when in an elevator
			bool lag_ok = false;
			if ( COMBAT_STAR == NULL || COMBAT_STAR->Is_Airborne() ) {
				lag_ok = true;
				LagPersistTimer = 1;  // Persist the lag for 1 second
			} else {
				if ( LagPersistTimer > 0 ) {
					lag_ok = true;
					LagPersistTimer -= TimeManager::Get_Frame_Seconds();
//					Debug_Say(( "Lag Persist\n" ));
				}
			}
			if ( DisableLag ) {
				lag_ok = false;
				DisableLag = false;
			}

			if ( lag_ok ) {
				// Get position local to the camera
				Vector3 local_last;
				Vector3 local_current;
				Matrix3D tm = Get_Transform();
				Matrix3D::Inverse_Transform_Vector( tm, LastAnchorPosition, &local_last );
				Matrix3D::Inverse_Transform_Vector( tm, anchor_position, &local_current );
				// Lerp				// ( left/right, up/down, forward/back )
				Vector3	lerp = profile.Lag * WWMath::Clamp( LagPersistTimer, 0, 1);	//( 0, 0.5f, 0.2f );	//		( y, z, x )
				lerp.X = ::pow( lerp.X, 10 * TimeManager::Get_Frame_Seconds() );
				lerp.Y = ::pow( lerp.Y, 10 * TimeManager::Get_Frame_Seconds() );
				lerp.Z = ::pow( lerp.Z, 10 * TimeManager::Get_Frame_Seconds() );
				lerp = Vector3( 1,1,1 ) - lerp;
				local_current.X = local_last.X + ( local_current.X - local_last.X ) * lerp.X;
				local_current.Y = local_last.Y + ( local_current.Y - local_last.Y ) * lerp.Y;
				local_current.Z = local_last.Z + ( local_current.Z - local_last.Z ) * lerp.Z;
				Matrix3D::Transform_Vector( tm, local_current, &anchor_position );

				Vector3 diff = anchor_position - LastAnchorPosition;
				if ( diff.Length() < 5.0f ) {		// Don't lerp over long distances
					anchor_position = ::Lerp( anchor_position, LastAnchorPosition, 0.25f );
				} else {
					LagPersistTimer = 0;
					DisableLag = true;
				}

			}
		}

		LastAnchorPosition = anchor_position;
		LastHeading	= Heading;
	}

	Set_View_Plane( profile.FOV );	// Apply Zoom


	// Calculate the Camera Transform
	Matrix3D	tm(1);													// Setup base position
	tm.Translate( anchor_position );
	tm.Translate( Vector3( 0, 0, profile.Height ) );

	Convert_World_To_Camera( &tm );								// Setup orientation
	tm.Rotate_Y( camera_heading );									// Apply rotations
	tm.Rotate_X( -profile.ViewTilt - Tilt);

	// Only do this when the profile has a distance value
	if ( profile.Distance != 0 ) {
		// Translate along Z so that our near clip plane is behind the head
		float nearz,farz;
		const float HEAD_RADIUS = 0.2f;								// HEAD_RADIUS should be renamed and become part of the profile?
		Get_Clip_Planes(nearz,farz);
		tm.Translate_Z(nearz + HEAD_RADIUS);						
	}

	Vector3 intermediate_pos = tm.Get_Translation();		// Save base position
	Matrix3D intermediate_tm = tm;								// Save base transform
	

	// Generate a translation path for the camera which is 'tilt' off of the z-axis
	Vector3 camera_move(0,0,profile.Distance);
	camera_move.Rotate_X(-profile.TranslationTilt);
	camera_move.Rotate_X( -(WWMath::Max( -Tilt * profile.TiltTweak, 0 )) );
	tm.Translate(camera_move);										// Pull back
	Vector3 end_pos = tm.Get_Translation();					// Save the end position


	// Sweep the view plane back until it hits something
	if ( profile.Distance != 0 ) {
		WWPROFILE( "Sweep" );

		// (gth) FIXME!
		// Sort of a hack here, trying to make the camera not collide with the star
		// Really this should make sure we don't collide with whatever the camera is starting inside
		Ignore_Star_And_Vehicle();

		// Collide the bounding box of the near clip plane
		// Have to Set_Transform so that the camera can calculate the box for us
		CastResultStruct res;				
		Set_Transform(intermediate_tm);
		Set_Clip_Planes( NearClipPlane,FarClipPlane );
		OBBoxClass box = Get_Near_Clip_Bounding_Box();
		PhysOBBoxCollisionTestClass boxtest(	box,
															end_pos - intermediate_pos,
															&res,
															DEFAULT_COLLISION_GROUP,
															COLLISION_TYPE_CAMERA);
		PhysicsSceneClass::Get_Instance()->Cast_OBBox(boxtest);

		// Solve the problem of the camera when getting out of the car
		if ( res.StartBad && interpolating && boxtest.CollidedPhysObj ) {
			// ignore what we hit and do it again
			PhysClass * hit = boxtest.CollidedPhysObj;
			hit->Inc_Ignore_Counter();
			res.Reset();
			PhysicsSceneClass::Get_Instance()->Cast_OBBox(boxtest);
			hit->Dec_Ignore_Counter();
		}

		// Move the camera to the collision if needed.
		if (res.Fraction < DistanceFraction) {
			DistanceFraction = res.Fraction;				// always pull camera in if a collision occured
		}
		if (res.Fraction > DistanceFraction) {
			float dt = TimeManager::Get_Frame_Seconds();
			DistanceFraction += WWMath::Min(res.Fraction-DistanceFraction,CAMERA_UNWIND_SPEED * dt);
		}

		if (DistanceFraction < 1.0f) {
			tm.Set_Translation(intermediate_pos + DistanceFraction * (end_pos - intermediate_pos));
			end_pos = tm.Get_Translation();
		}

		// When Physics_Debug is on, draw the camera collision box in its start point
#ifdef WWDEBUG	
		PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
		if (scene && scene->Is_Debug_Display_Enabled()) {
			scene->Add_Debug_OBBox(box,Vector3(1,0,0));
		}
#endif

		// Now put the star back to his original 'ignore' state
		Unignore_Star_And_Vehicle();

	} else {
		WWPROFILE( "No Sweep" );

		// This is a camera which doesn't translate back.  Just check its near clip plane for intersection
		// with the world and if it does intersect, pull the near clip plane in to its minimum.
		Ignore_Star_And_Vehicle();

		// Collide the bounding box of the near clip plane
		// Have to Set_Transform so that the camera can calculate the box for us
		CastResultStruct res;				
		Set_Transform(tm);
		Set_Clip_Planes(NearClipPlane,FarClipPlane);
		OBBoxClass box = Get_Near_Clip_Bounding_Box();

		Vector3 null_vec(0.0f,0.0f,0.0f);
		PhysOBBoxCollisionTestClass boxtest(	box,
															null_vec,
															&res,
															DEFAULT_COLLISION_GROUP,
															COLLISION_TYPE_CAMERA);
		PhysicsSceneClass::Get_Instance()->Cast_OBBox(boxtest);

		// Set the near clip plane depending on whether the default near clip plane
		// intersected any geometry
		if (res.StartBad) {
			Set_Clip_Planes(CCAMERA_MIN_NEARZ,FarClipPlane);
		} else {
			Set_Clip_Planes(NearClipPlane,FarClipPlane);
		}

		Unignore_Star_And_Vehicle();
	}

	Set_Transform( tm );						// Set our new transform

//	Debug_Say(( "Camera at %1.2f, %1.2f, %1.2f\n", tm.Get_Translation().X, tm.Get_Translation().Y, tm.Get_Translation().Z ));

	// First, set the aiming point to where the camera is looking
	if ( Determine_Targeting_Position() == false ) {
		WWPROFILE( "Help" );
		// Then, modify the aiming point for weapon help, if not on a target
		Apply_Weapon_Help();
	}

	// Lastly, pass the aiming point to the star
	bool is_star_determining_target = CombatManager::Is_Star_Determining_Target();
	if ( COMBAT_STAR && is_star_determining_target ) {			// and tell the star what we are looking at...

#if 0
		// This didn't work with X key....
		// Make sure it is forward in star space
		Matrix3D star_tm = COMBAT_STAR->Get_Transform();
		Vector3	star_space_pos;
		Matrix3D::Inverse_Transform_Vector( star_tm, StarTargetingPosition, &star_space_pos );
		if ( star_space_pos.X <= 0 ) {
			star_space_pos.X = MAX( star_space_pos.X, 1 );
		}
		Debug_Say(( "SSP %f %f %f\n", star_space_pos.X, star_space_pos.Y, star_space_pos.Z ));
		Matrix3D::Transform_Vector( star_tm, star_space_pos, &StarTargetingPosition );
#endif

		COMBAT_STAR->Set_Targeting( StarTargetingPosition ); 
	}

}

/*
** Determine_Targeting_Position finds the World position and object the player is looking/pointing at
** The details get stored in HUDInfo
*/
bool	CCameraClass::Determine_Targeting_Position( void )
{
	WWPROFILE( "Determining Targeting" );
	bool	looking_at_object = false;

	Matrix3D tm = Get_Transform();
	/*
	**
	*/
	SoldierGameObj * star = COMBAT_STAR;
	bool is_star_determining_target = CombatManager::Is_Star_Determining_Target();
	if ( star && is_star_determining_target ) {			// and tell the star what we are looking at...

		// Now cast to determine what we are looking/pointing at...
		Vector3 cast_start = tm.Get_Translation();

		float range = 100;
		if ( star && star->Get_Weapon() ) {
			range = star->Get_Weapon()->Get_Range();
		}
		if ( star && star->Get_Vehicle() && star->Get_Vehicle()->Get_Weapon() ) {
			range = star->Get_Vehicle()->Get_Weapon()->Get_Range();
		}
		range += CurrentProfile->Distance;

		Vector2 view_plane;
		view_plane.X = CameraTarget2DOffset.X;
		view_plane.Y = CameraTarget2DOffset.Y;
		Vector3 world_pos;
		COMBAT_CAMERA->Un_Project( world_pos, view_plane );

		Vector3 cast_end = world_pos - cast_start;
		cast_end.Normalize();
		cast_end *= range;
		cast_end += cast_start;

		LineSegClass ray;

		if ( !Enable2DTargeting ) {
			// start the ray in front of the character + 0.5m
			float move_forward_dist = DistanceFraction * CurrentProfile->Distance + 0.5f;
			cast_start += move_forward_dist * -(tm.Get_Z_Vector());
			ray.Set( cast_start, cast_end );			
			
		} else {
			// for 2D, start at the camera
			ray.Set( cast_start, cast_end );
		}

		// trim off Near Clip from Beginning
		float start_fraction = NearClipPlane / ray.Get_Length();
		ray.Compute_Point( start_fraction, &cast_start );
		ray.Set( cast_start, cast_end );

		CastResultStruct result;
		PhysRayCollisionTestClass raytest(ray, &result, 
			BULLET_COLLISION_GROUP, COLLISION_TYPE_PROJECTILE);
		WWASSERT(COMBAT_SCENE != NULL);

		Ignore_Star_And_Vehicle();
{ WWPROFILE( "Cast Ray" );
		COMBAT_SCENE->Cast_Ray( raytest );
}
		Unignore_Star_And_Vehicle();

		// Determine the Camera Target Point and object
		ray.Compute_Point( raytest.Result->Fraction, &StarTargetingPosition );
	
		//
		// (gth) "hack" to prevent the target point from ever ending up behind the player
		// this sometimes happens when looking up at an extreme angle in third person
		//
		if (COMBAT_STAR != NULL) {
			Vector3 player_pos(0,0,0);

			if (COMBAT_STAR->Get_Profile_Vehicle() != NULL) {
				COMBAT_STAR->Get_Profile_Vehicle()->Get_Position(&player_pos);
			} else {
				COMBAT_STAR->Get_Position(&player_pos);
			}


			float player_dx = player_pos.X - cast_start.X;
			float player_dy = player_pos.Y - cast_start.Y;
			float target_dx = StarTargetingPosition.X - cast_start.X;
			float target_dy = StarTargetingPosition.Y - cast_start.Y;

			float xy_dist_to_player2 = player_dx * player_dx + player_dy * player_dy;
			float xy_dist_to_target2 = target_dx * target_dx + target_dy * target_dy;

			if (xy_dist_to_target2 < xy_dist_to_player2) {
				
				// Move the point along the raycast until until
				// target is in front of player.
				float ray_parameter = 1.1f * WWMath::Sqrt(xy_dist_to_player2) / WWMath::Sqrt(xy_dist_to_target2);
				StarTargetingPosition = ray.Get_P0() + ray.Get_Dir() * ray_parameter;
			}
		}

		//Debug_Say(( "Targeting %f %f %f\n", StarTargetingPosition.X, StarTargetingPosition.Y, StarTargetingPosition.Z ));

		if ( raytest.CollidedPhysObj != NULL && raytest.CollidedPhysObj->Get_Observer() != NULL ) {
			DamageableGameObj * obj = ((CombatPhysObserverClass *)raytest.CollidedPhysObj->Get_Observer())->As_DamageableGameObj();

			bool MCT = false;

			if ( obj->As_BuildingGameObj() ) {
				// Check for MCT collision
				if (raytest.CollidedPhysObj->Get_Factory().Chunk_ID() == PHYSICS_CHUNKID_BUILDINGAGGREGATE) {
					if (((BuildingAggregateClass *)raytest.CollidedPhysObj)->Is_MCT()) {
						MCT = true;
					}
				}
			}

			if ( obj->Is_Targetable() == false ) {
				obj = NULL;
			}

			// if stealthed and enemy, it's not targetable
			if ( obj && obj->As_SmartGameObj() && obj->As_SmartGameObj()->Is_Stealthed() && 
					COMBAT_STAR && obj->Is_Enemy( COMBAT_STAR ) ) {
				obj = NULL;
			}

			looking_at_object = ( obj != NULL );
			HUDInfo::Set_Info_Object( obj, MCT );		// Update the HUDInfo Info Object
			HUDInfo::Set_Weapon_Target_Object( obj );
		}

		if ( raytest.CollidedPhysObj ) {
			Set_Sniper_Distance( raytest.Result->Fraction * ray.Get_Length() );
		} else {
			Set_Sniper_Distance( 0 );
		}

	}
	return looking_at_object;
}

/*
** Apply_Weapon_Help : Use weapon Help to update the StarTargetingPosition );
** Should only be called if not already on a target object
** We will only do the search periodically, so we will save the last valid target, 
** and track him until we search again.
*/
#define	WEAPON_HELP_CHEAT	0

void	CCameraClass::Apply_Weapon_Help( void )
{
	WWPROFILE( "Weapon Help" );
	WeaponHelpTimer -= TimeManager::Get_Frame_Seconds();
	if ( WeaponHelpTimer <= 0 ) {

#if WEAPON_HELP_CHEAT
		WeaponHelpTimer = 0.01f;
#else
		WeaponHelpTimer = 0.5f;	// Re-check every 1/2 second
#endif

		WeaponHelpTargetID = 0;		// Asuume we find nothing

		SmartGameObj * star = COMBAT_STAR;

//		if ( star && EnableWeaponHelp )	{
#if WEAPON_HELP_CHEAT
		if ( star )	{ 
#else
		if ( star && CombatManager::Get_Difficulty_Level() == 0 && IS_MISSION )	{	// if on easy difficulty
#endif

			// Make sure dot is off
#if WEAPON_HELP_CHEAT
			if ( !CombatManager::Is_Hit_Reticle_Enabled() ) {
#else
			if ( CombatManager::Is_Hit_Reticle_Enabled() ) {
#endif
				CombatManager::Toggle_Hit_Reticle_Enabled();
			}

			float weapon_range = 100;
			if ( star->Get_Weapon() ) {
				weapon_range = star->Get_Weapon()->Get_Range();
			}		
			Vector3	star_pos;
			star->Get_Position( &star_pos );

			SmartGameObj * best_obj = NULL;
#if WEAPON_HELP_CHEAT
			float best_distance = 10;
#else
			float best_distance = 0.1f;
#endif

			// Find a target object near the camera target vector
			SLNode<SmartGameObj> * smart_objnode;
			for (smart_objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); smart_objnode; smart_objnode = smart_objnode->Next()) {
				SmartGameObj * obj = smart_objnode->Data();

				if ( obj == COMBAT_STAR ) {
					continue;
				}

#if WEAPON_HELP_CHEAT
				if ( !obj->Is_Enemy( COMBAT_STAR ) ) {
					continue;
				}
#endif

				Vector3 pos = obj->Get_Bullseye_Position();

				float range = (pos - star_pos).Length();
				if ( range > weapon_range ) {
					continue;
				}

				Vector3 twoDpos;

				if ( Project( twoDpos, pos ) == CameraClass::INSIDE_FRUSTUM ) {

					twoDpos.X -= CameraTarget2DOffset.X;
					twoDpos.Y -= CameraTarget2DOffset.Y;
					twoDpos.Z = 0;

					float distance = twoDpos.Length();

					if ( distance < best_distance ) {

						// This is the best target so far, cast a Ray and see if it is a possible target

						// We are seeing if the camera can see him, not if the gun can hit him...
						Vector3 cast_start = Get_Transform().Get_Translation();
						Vector3 cast_end = pos;
						LineSegClass ray;
						ray.Set( cast_start, cast_end );			

	#if 0
						// trim off Near Clip from Beginning
						float start_fraction = NearClipPlane / ray.Get_Length();
						ray.Compute_Point( start_fraction, &cast_start );
						ray.Set( cast_start, cast_end );
	#endif

						CastResultStruct result;
						PhysRayCollisionTestClass raytest(ray, &result, 
							BULLET_COLLISION_GROUP, COLLISION_TYPE_PROJECTILE);
						WWASSERT(COMBAT_SCENE != NULL);

						Ignore_Star_And_Vehicle();
	{ WWPROFILE( "Cast Ray" );
						COMBAT_SCENE->Cast_Ray( raytest );
	}
						Unignore_Star_And_Vehicle();

						//Debug_Say(( "Targeting %f %f %f\n", StarTargetingPosition.X, StarTargetingPosition.Y, StarTargetingPosition.Z ));

						if ( raytest.CollidedPhysObj != NULL && raytest.CollidedPhysObj->Get_Observer() != NULL && 
							 ((CombatPhysObserverClass *)raytest.CollidedPhysObj->Get_Observer())->As_PhysicalGameObj() ) {
							best_distance = distance;
							best_obj = obj;
						} else {
							// If this is the Info Target, forget him!
							if ( obj == HUDInfo::Get_Info_Object() ) {
								HUDInfo::Set_Info_Object( NULL ); 
							}

						}
					}

				}
  			}

			if ( best_obj != NULL ) {
				// remember this guy
				WeaponHelpTargetID = best_obj->Get_ID();
				StarTargetingPosition = best_obj->Get_Bullseye_Position();
				HUDInfo::Set_Info_Object( best_obj );			// Update HUD Info Object
			}
		}
	} else {

		// If we had a target
		if ( WeaponHelpTargetID != 0 ) {
			SmartGameObj * target = GameObjManager::Find_SmartGameObj( WeaponHelpTargetID );
			// and we still have him
			if ( target != NULL ) {
				// update the targting pos
				StarTargetingPosition = target->Get_Bullseye_Position();
			} else {
				// else, forget about him
				WeaponHelpTargetID = 0;
			}
		}
	}

}

void	CCameraClass::Use_Profile( const char * name )
{
	if ( name ) {
		// quick reject if the same
		if ( CurrentProfile && !stricmp( CurrentProfileName, name ) ) {
			return;
		}

		CCameraProfileClass *profile = CCameraProfileClass::Find( name );
		if ( profile ) {
			CurrentProfile = profile;
			CurrentProfileName=name;
		}
	} else {
		CurrentProfile = NULL;
	}
}

void	CCameraClass::Use_Default_Profile()
{
	WWASSERT( DefaultProfile );
	CurrentProfile = DefaultProfile;
	CurrentProfileName=DefaultProfileName;
}

void	CCameraClass::Set_Profile_Height( float height )
{
	CurrentProfile->Set_Height( height );
}

void	CCameraClass::Set_Profile_Distance( float distance )
{
	CurrentProfile->Set_Distance( distance );
}

float	CCameraClass::Get_Profile_Zoom( void )
{
	return CurrentProfile->Get_Zoom();
}

/*
**
*/
void	CCameraClass::Handle_Input( void )
{
	#define TILT_ADJUST			(float)DEG_TO_RAD(30.0f)
	#define MAX_TILT				(float)DEG_TO_RAD(80.0f)
	#define MIN_TILT				(float)DEG_TO_RAD(-30.0f)
	
	#define DISTANCE_ADJUST		5.0f
	#define MIN_DISTANCE			-5.0f
	#define MAX_DISTANCE			200.0f

	#define FOV_ADJUST			1.0f

	#define HEIGHT_ADJUST		0.5f
	#define MIN_HEIGHT			-3.0f
	#define MAX_HEIGHT			4.0f

	#define FP_TWEAK_RATE		0.1f;

	float dt = TimeManager::Get_Frame_Real_Seconds();

	// Hold F9 to control the camera
	assert( CurrentProfile != NULL );

#ifdef WWDEBUG
	if ( CombatManager::Is_First_Person() ) {

		Vector3	tweak(0,0,0);

		tweak.X	 =	( Input::Get_State( INPUT_FUNCTION_CAMERA_TRANSTILT_INC ) ? 1 : 0 ) + 
						( Input::Get_State( INPUT_FUNCTION_CAMERA_TRANSTILT_DEC ) ? -1 : 0 );

		tweak.Y	 =	( Input::Get_State( INPUT_FUNCTION_DEBUG_FAR_CLIP_IN ) ? -1 : 0 ) + 
						( Input::Get_State( INPUT_FUNCTION_DEBUG_FAR_CLIP_OUT ) ? 1 : 0 );

		tweak.Z	 =	( Input::Get_State( INPUT_FUNCTION_CAMERA_DIST_INC ) ? -1 : 0 ) + 
						( Input::Get_State( INPUT_FUNCTION_CAMERA_DIST_DEC ) ? 1 : 0 );

		tweak *= dt * FP_TWEAK_RATE;
		if ( tweak.Length() ) {
			FirstPersonOffsetTweak += tweak;
			Vector3 offset( 0,0,0 );
			if ( COMBAT_STAR != NULL && COMBAT_STAR->Get_Weapon() != NULL ) {
				offset = COMBAT_STAR->Get_Weapon()->Get_First_Person_Model_Offset();
			}
			offset += FirstPersonOffsetTweak;
			Debug_Say(( "First Person Offset ( %1.3f, %1.3f, %1.3f )\n", offset.X, offset.Y, offset.Z ));
//			Debug_Say(( "First Person Tweak ( %1.3f, %1.3f, %1.3f )\n", FirstPersonOffsetTweak.X, FirstPersonOffsetTweak.Y, FirstPersonOffsetTweak.Z ));
		}

	} else {

		float	tilt_amount =	( Input::Get_State( INPUT_FUNCTION_CAMERA_TRANSTILT_INC ) ? 1 : 0 ) + 
									( Input::Get_State( INPUT_FUNCTION_CAMERA_TRANSTILT_DEC ) ? -1 : 0 );
		CurrentProfile->TranslationTilt += TILT_ADJUST * tilt_amount * dt;
		CurrentProfile->TranslationTilt = WWMath::Clamp( CurrentProfile->TranslationTilt, MIN_TILT, MAX_TILT );

		float	view_tilt_amount =	( Input::Get_State( INPUT_FUNCTION_CAMERA_VIEWTILT_INC ) ? 1 : 0 ) + 
									( Input::Get_State( INPUT_FUNCTION_CAMERA_VIEWTILT_DEC ) ? -1 : 0 );
		CurrentProfile->ViewTilt += TILT_ADJUST * view_tilt_amount * dt;
		CurrentProfile->ViewTilt = WWMath::Clamp( CurrentProfile->ViewTilt, MIN_TILT, MAX_TILT );

		float	distance_amount =	( Input::Get_State( INPUT_FUNCTION_CAMERA_DIST_INC ) ? 1 : 0 ) + 
										( Input::Get_State( INPUT_FUNCTION_CAMERA_DIST_DEC ) ? -1 : 0 );
		CurrentProfile->Distance += DISTANCE_ADJUST * distance_amount * dt;
		CurrentProfile->Distance = WWMath::Clamp( CurrentProfile->Distance, MIN_DISTANCE, MAX_DISTANCE );

		float	fov_amount =	( Input::Get_State( INPUT_FUNCTION_CAMERA_FOV_INC ) ? 1 : 0 ) + 
									( Input::Get_State( INPUT_FUNCTION_CAMERA_FOV_DEC ) ? -1 : 0 );
		CurrentProfile->FOV += FOV_ADJUST * fov_amount * dt;
		CurrentProfile->FOV = WWMath::Clamp( CurrentProfile->FOV, MIN_FOV, MAX_FOV );

		float	height_amount =	( Input::Get_State( INPUT_FUNCTION_CAMERA_HEIGHT_INC ) ? 1 : 0 ) + 
										( Input::Get_State( INPUT_FUNCTION_CAMERA_HEIGHT_DEC ) ? -1 : 0 );
		CurrentProfile->Height += HEIGHT_ADJUST * height_amount * dt;
		CurrentProfile->Height = WWMath::Clamp( CurrentProfile->Height, MIN_HEIGHT, MAX_HEIGHT );

		if ( ( tilt_amount != 0 ) || (view_tilt_amount != 0) || ( distance_amount != 0 ) ||
			  ( fov_amount != 0 ) || ( height_amount != 0 ) ) 
		{
		
			Debug_Say(( " Dist: %3.1f Height: %3.1f FOV %3.1f TransTilt: %3.1f ViewTilt: %3.1f\n",
				CurrentProfile->Distance,
				CurrentProfile->Height,
				RAD_TO_DEG( CurrentProfile->FOV ), 
				RAD_TO_DEG( CurrentProfile->TranslationTilt ),
				RAD_TO_DEG( CurrentProfile->ViewTilt )
				));
		}

		if ( Input::Get_State( INPUT_FUNCTION_DEBUG_FAR_CLIP_IN ) || 
			 Input::Get_State( INPUT_FUNCTION_DEBUG_FAR_CLIP_OUT ) ) {
			float fog_start, fog_stop;
			COMBAT_SCENE->Get_Fog_Range( &fog_start, &fog_stop );
			if ( Input::Get_State( INPUT_FUNCTION_DEBUG_FAR_CLIP_OUT ) ) {
				FarClipPlane *= 1.1f;
				fog_start *= 1.1f;
			} else {
				FarClipPlane /= 1.1f;
				fog_start /= 1.1f;
			}
			COMBAT_SCENE->Set_Fog_Range( fog_start, FarClipPlane );
			Debug_Say(( "FarClipPlane %f\n", FarClipPlane ));
			Set_Clip_Planes( NearClipPlane, FarClipPlane );
		} 

	}
#endif

	if ( CinematicSnipingEnabled ) {

		float zoom_change = CinematicSnipingDesiredZoom - SniperZoom;
		float max = TimeManager::Get_Frame_Seconds();
		zoom_change = WWMath::Clamp( zoom_change, -max, max );
		SniperZoom += zoom_change;
		CurrentProfile->Set_Zoom( SniperZoom );
	}

#ifdef WWDEBUG
	if ( Input::Get_State( INPUT_FUNCTION_TOGGLE_SNAP_SHOT_MODE ) ) {
		Set_Snap_Shot_Mode( Is_Snap_Shot_Mode() == SNAPSHOT_OFF );
		Debug_Say(( "Snap Shot Mode %s\n", Is_Snap_Shot_Mode() ? "ON" : "OFF" ));
	}
#endif

	if ( COMBAT_STAR != NULL && COMBAT_STAR->Is_Control_Enabled() == false ) {
		// Don't move camera if control is disabled
		return;
	}

	// if Snipinging Changed, update tilt to match old target
	if ( WasStarSniping != IsStarSniping ) {
		WasStarSniping = IsStarSniping;

		SmartGameObj * star = COMBAT_STAR;
		if ( star ) {
			Vector3 target_pos = star->Get_Targeting_Pos();
			Vector3	camera_pos = AnchorPosition;
			camera_pos.Z += CurrentProfile->Height;
			camera_pos -= target_pos;
			float drop = camera_pos.Z;
			float dist = camera_pos.Length();
			Tilt = WWMath::Asin( drop/dist );
			Tilt -= CurrentProfile->ViewTilt;
		}
	}

	// Sniper Control
	float sniping_scale = 1;
	if ( !Is_Using_Host_Model() && Is_Star_Sniping() ) {

		float amount = Input::Get_Amount( INPUT_FUNCTION_ZOOM_IN ) - 
						   Input::Get_Amount( INPUT_FUNCTION_ZOOM_OUT );
		SniperZoom += amount * TimeManager::Get_Frame_Seconds();
		SniperZoom = WWMath::Clamp( SniperZoom, 0, 1 );

		// Scale weapon tilt and turn from Zoom
		float	sniper_scale = 0.3f;
		float scale = 1 - SniperZoom * (1 - sniper_scale);
//		Debug_Say(( "scale %f\n", scale ));
		sniping_scale = scale * scale;

		// Set Zoom
		CurrentProfile->Set_Zoom( SniperZoom );

		// Update Zooming Sound.....
	}


	//	Adjust Tilt
	float tilt_amount =	Input::Get_Amount( INPUT_FUNCTION_WEAPON_UP ) - 
						Input::Get_Amount( INPUT_FUNCTION_WEAPON_DOWN );

	//	Adjust Facing
	float	turn_amount =	Input::Get_Amount( INPUT_FUNCTION_WEAPON_LEFT ) - 
								Input::Get_Amount( INPUT_FUNCTION_WEAPON_RIGHT );

	if ( Input::Get_State( INPUT_FUNCTION_TURN_AROUND ) ) {
//		turn_amount += DEG_TO_RADF( 180.0F ) / dt;
		Heading = Heading + DEG_TO_RADF( 180.0F );
		if ( Heading > DEG_TO_RADF( 360.0F ) ) {
			Heading -= DEG_TO_RADF( 360.0F );
		}

		if ( COMBAT_STAR ) {
			Vector3 pos;
			COMBAT_STAR->Get_Position( &pos );
			DIAG_LOG(( "SPUS", "%1.2f; %1.2f; %1.2f", pos.X, pos.Y, pos.Z ));
		}

	}

	if ( Enable2DTargeting ) {
		// Adjust Targeting Offset

		if ( !( Input::Get_Mouse_2D_Invert() ^ Input::Get_Mouse_Invert() ) ) {
			tilt_amount = -tilt_amount;
		}

		CameraTarget2DOffset.X += -turn_amount * dt;
		CameraTarget2DOffset.Y += -tilt_amount * dt;

#define	_2D_TARGETING_LIMIT	0.85f
		CameraTarget2DOffset.X = WWMath::Clamp( CameraTarget2DOffset.X, -_2D_TARGETING_LIMIT, _2D_TARGETING_LIMIT );
		CameraTarget2DOffset.Y = WWMath::Clamp( CameraTarget2DOffset.Y, -_2D_TARGETING_LIMIT, _2D_TARGETING_LIMIT );
	} else {
		Tilt += -tilt_amount * dt * sniping_scale;
		Heading += turn_amount * dt * sniping_scale;

		CameraTarget2DOffset.X = 0.0f;
		CameraTarget2DOffset.Y = 0.0f;
	}

	// Clamp the tilt to this arbitrary value
	Tilt = WWMath::Clamp( Tilt, DEG_TO_RADF( -80 ), DEG_TO_RADF( 80 ) );

	// if soldier tries to turn, turn the camera
	float	soldier_turn = 0;
	if ( COMBAT_STAR != NULL && COMBAT_STAR->Get_Vehicle() == NULL ) {
		// only do this if the soldier is not in a vehicle
		soldier_turn =	Input::Get_Amount( INPUT_FUNCTION_TURN_LEFT ) - 
							Input::Get_Amount( INPUT_FUNCTION_TURN_RIGHT );
	}

	Heading += soldier_turn * dt * sniping_scale;
}

void CCameraClass::Force_Look( const Vector3 & target ) 
{
	Vector3 diff = Get_Transform().Get_Translation();
	diff -= target;

	Heading = WWMath::Atan2( -diff.Y, -diff.X );

	float drop = diff.Z;
	float dist = diff.Length();
	Tilt = WWMath::Asin( drop/dist );
	Tilt -= CurrentProfile->ViewTilt;
}

void CCameraClass::Set_Lerp_Time( float time )
{
	LerpTimeTotal = time;
	LerpTimeRemaining = time;
}

Vector3	CCameraClass::Get_First_Person_Offset_Tweak( void )
{
	return FirstPersonOffsetTweak;
}

void	CCameraClass::Reset_First_Person_Offset_Tweak( void )
{
	FirstPersonOffsetTweak = Vector3(0,0,0);
}

bool	CCameraClass::Draw_Sniper( void )
{
	if ( Is_Using_Host_Model() ) {
		return CinematicSnipingEnabled;
	} else {
		return IsStarSniping;
	}
}

void	CCameraClass::Cinematic_Sniper_Control( bool enabled, float zoom )
{
	Debug_Say(( "Camera Sniper Control %d, %f \n", enabled, zoom ));
	CinematicSnipingEnabled = enabled;
	CinematicSnipingDesiredZoom = zoom;
}

void	CCameraClass::Set_Is_Star_Sniping( bool onoff )
{
	if (IsStarSniping != onoff) {

		IsStarSniping = onoff;

		//
		//	Add or remove the sniper listener from the world
		//
		if ( SniperListener != NULL && WWAudioClass::Get_Instance() != NULL ) {
			if ( WWAudioClass::Get_Instance()->Get_Sound_Scene()->Peek_2nd_Listener() != NULL ) {
				WWAudioClass::Get_Instance()->Get_Sound_Scene()->Set_2nd_Listener( NULL );
			} else {
				WWAudioClass::Get_Instance()->Get_Sound_Scene()->Set_2nd_Listener( SniperListener );

				//
				//	Update the listener's position in the world
				//
				Update_Sniper_Listener_Pos();
			}
		}

		if ( COMBAT_STAR ) {
			Vector3 pos;
			COMBAT_STAR->Get_Position( &pos );
			int ammo = 0;
			if ( COMBAT_STAR->Get_Weapon() ) {
				ammo = COMBAT_STAR->Get_Weapon()->Get_Total_Rounds();
			}
			if ( IsStarSniping ) {
				DIAG_LOG(( "SNEN", "%1.2f; %1.2f; %1.2f; %d ", pos.X, pos.Y, pos.Z, ammo ));
			} else {
				DIAG_LOG(( "SNEX", "%1.2f; %1.2f; %1.2f; %d ", pos.X, pos.Y, pos.Z, ammo ));
			}
		}

	}

	return ;
}


void CCameraClass::Update_Sniper_Listener_Pos( void )
{
	if ( SniperListener != NULL ) {
		
		//
		//	Get the camera's transform
		//
		Matrix3D tm = Get_Transform();

		//
		//	Calculate what point in world space the sniper is zoomed into
		//
		float dist = ::tan (1.45F + ((1.5672F - 1.45F) * SniperZoom));	
		dist = min (dist, SniperDistance);
		Vector3 pos = tm.Get_Translation() - (tm.Get_Z_Vector() * dist);

		//
		//	Update the sniper's position
		//
		SniperListener->Set_Transform( Matrix3D( pos ) );
	}

	return ;
}


void CCameraClass::Set_Sniper_Distance( float dist )
{
	//
	//	If the distance has changed, update the sniper's listener
	//
	if ( SniperDistance != dist ) {
		SniperDistance = dist;
		Update_Sniper_Listener_Pos();
	}	

	return ;
}

/*
**
*/
void	CCameraClass::Handle_Snap_Shot_Mode( void )
{
	float dt = TimeManager::Get_Frame_Real_Seconds();

	static float tilt = 0;
	static float turn = 0;
	float tilt_amount =	Input::Get_Amount( INPUT_FUNCTION_WEAPON_UP ) - 
								Input::Get_Amount( INPUT_FUNCTION_WEAPON_DOWN );

	float	turn_amount =	Input::Get_Amount( INPUT_FUNCTION_WEAPON_LEFT ) - 
								Input::Get_Amount( INPUT_FUNCTION_WEAPON_RIGHT );

	if (turn_amount == 0) {
		turn_amount =	Input::Get_Amount( INPUT_FUNCTION_TURN_LEFT ) - 
							Input::Get_Amount( INPUT_FUNCTION_TURN_RIGHT );
	}
	
	tilt += tilt_amount * dt;
	turn += turn_amount * dt;

	Matrix3D tm = Get_Transform();
	Vector3 pos = tm.Get_Translation();

	tm.Make_Identity();
	tm.Set_Translation( pos );
	tm.Rotate_Z( turn );
	tm.Rotate_X( tilt );

	float forward = Input::Get_Amount( INPUT_FUNCTION_MOVE_FORWARD ) - 
						Input::Get_Amount( INPUT_FUNCTION_MOVE_BACKWARD );
	float left = Input::Get_Amount( INPUT_FUNCTION_MOVE_LEFT ) - 
						Input::Get_Amount( INPUT_FUNCTION_MOVE_RIGHT );
	float up = Input::Get_Amount( INPUT_FUNCTION_MOVE_UP ) - 
						Input::Get_Amount( INPUT_FUNCTION_MOVE_DOWN );

	SnapShotMode = Input::Get_State( INPUT_FUNCTION_SNAP_SHOT_ADVANCE ) ? SNAPSHOT_PROGRESS : SNAPSHOT_ON;

	dt *= 4;
	tm.Translate( Vector3( -left * dt, up * dt, -forward * dt ) );

	Set_Transform( tm );
}

