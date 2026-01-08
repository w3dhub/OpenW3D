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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/combat.cpp                            $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/17/02 10:25a                                              $*
 *                                                                                             *
 *                    $Revision:: 265                                                        $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "combat.h"
#include "ccamera.h"
#include "gameobjmanager.h"
#include "input.h"
#include "weaponmanager.h"
#include "wwprofile.h"
#include "scriptman.h"
#include "pscene.h"
#include "damage.h"
#include "ww3d.h"
#include "assets.h"
#include "timemgr.h"
#include "soldier.h"
#include "SoundScene.h"
#include "weapons.h"
#include "vehicle.h"
#include "bones.h"
#include "humanstate.h"
#include "surfaceeffects.h"
#include "debug.h"
#include "backgroundmgr.h"
#include "cover.h"
#include "spawn.h"
#include "vehiclephys.h"
#include "building.h"
#include "objectives.h"
#include "conversationmgr.h"
#include "bullet.h"
#include "dazzle.h"
#include "messagewindow.h"
#include "hudinfo.h"
#include "WeatherMgr.h"
#include "thread.h"
#include "savegame.h"
#include "assetdep.h"
#include "saveloadstatus.h"
#include <stdio.h>
#include "SoundEnvironment.h"
#include "weaponview.h"
#include "hud.h"
#include "mapmgr.h"
#include "gametype.h"
#include "staticnetworkobject.h"
#include "diaglog.h"
#include "combatdazzle.h"
#include "ffactorylist.h"
#include "definitionfactorymgr.h"
#include "definitionfactory.h"
#include "globalsettings.h"
#include "dx8wrapper.h"
#include "Except.h"
#include "cheatmgr.h"
#include "systeminfolog.h"
#include "assetstatus.h"
#include "wwmemlog.h"
#include "unitcoordinationzonemgr.h"
#include "FastAllocator.h"
#include "screenfademanager.h"
#include "animatedsoundmgr.h"
#include "render2dsentence.h"
#include "stylemgr.h"
#include "translatedb.h"
#include "string_ids.h"


const int DEFAULT_MAX_SHADOWS = 4;


/*
**
*/
CCameraClass				*		CombatManager::MainCamera			= NULL;
SimpleSceneClass			*		CombatManager::BackgroundScene	= NULL;
SoundEnvironmentClass	*		CombatManager::SoundEnvironment	= NULL;
DazzleLayerClass			*		CombatManager::DazzleLayer			= NULL;
GameObjReference					CombatManager::TheStar;
bool									CombatManager::IsStarDeterminingTarget = true;
//bool									CombatManager::IAmServer			= true;
//bool									CombatManager::IAmClient			= true;
bool									CombatManager::IAmServer			= false;
bool									CombatManager::IAmClient			= false;
bool									CombatManager::IAmOnlyClient		= false;
bool									CombatManager::IAmOnlyServer		= false;
int									CombatManager::MyId					= 0;
int									CombatManager::SyncTime				= 0;
CombatNetworkHandlerClass *	CombatManager::NetworkHandler		= NULL;
CombatMiscHandlerClass *		CombatManager::MiscHandler			= NULL;
MessageWindowClass *				CombatManager::MessageWindow		= NULL;
bool									CombatManager::EnableSkeletonSliderDemo = false;
int									CombatManager::DifficultyLevel		= 1;
bool									CombatManager::AutoTransitions		= true;
int									CombatManager::StarDamageDirection	= 0;
bool									CombatManager::AreObserversActive	= true;
bool									CombatManager::FirstPerson				= true;
bool									CombatManager::FirstPersonDefault 		= true;
bool									CombatManager::IsFirstLoad				= false;
int									CombatManager::StarKillerID			= 0;
bool									CombatManager::IsGamePaused			= false;
bool									CombatManager::IsLevelInitialized	= false;
StringClass							CombatManager::StartScript;
StringClass							CombatManager::RespawnScript;
bool									CombatManager::AutosaveRequested		= false;
DWORD									CombatManager::LastRoundTripPingMs	= 0;
DWORD									CombatManager::AvgRoundTripPingMs	= 0;
bool									CombatManager::FriendlyFirePermitted = false;
bool									CombatManager::BeaconPlacementEndsGame = false;
bool									CombatManager::HitReticleEnabled	 = true;
bool									CombatManager::IsGameplayPermitted = false;

int									CombatManager::CombatMode = CombatManager::COMBAT_MODE_NONE;
int									CombatManager::ReloadCount = 0;
StringClass							CombatManager::LastLSDName;
int									CombatManager::LoadProgress;
bool									CombatManager::MultiplayRenderingAllowed = true;

static	PhysicsSceneClass	*	GameScene	= NULL;

/*
**
*/
void	CombatManager::Init( bool render_available )
{
//	Debug_Say(("CombatManager::Init\n"));

	IsGameplayPermitted=false;

	ConversationMgrClass::Initialize ();

	MessageWindow = new MessageWindowClass;
	MessageWindow->Initialize ();

	ScriptManager::Init();

	BonesManager::Init();

	ArmorWarheadManager::Init();

	CCameraClass::Init();

	SurfaceEffectsManager::Init();

	ObjectiveManager::Init();

	CombatSoundManager::Init();

	// create THE camera
	MainCamera = new CCameraClass();

	// create the dazzle layer
	if (render_available) {
		DazzleLayer = new DazzleLayerClass;
		DazzleRenderObjClass::Set_Current_Dazzle_Layer(DazzleLayer);
	} else {
		DazzleRenderObjClass::Set_Current_Dazzle_Layer(NULL);
	}

	// Pass the main camera onto the 3D audio library as the
	// listener's position.
	SoundSceneClass *sound_scene = WWAudioClass::Get_Instance ()->Get_Sound_Scene ();
	if (sound_scene != NULL) {
		sound_scene->Attach_Listener_To_Obj (MainCamera);
	}

	HUDClass::Init(render_available);
	ScreenFadeManager::Init();
	FirstPerson	= FirstPersonDefault;
}

/*
**
*/
void	CombatManager::Shutdown( void )
{
//	Debug_Say(("CombatManager::Shutdown\n"));
	ScreenFadeManager::Shutdown();
	HUDClass::Shutdown();

	if ( GameScene != NULL ) {
		// Debug_Say(( "Releasing the PScene with %d Refs\n", GameScene->Num_Refs() ));
		GameScene->Release_Ref();
		GameScene = NULL;
	}

	//
	// Reset the audio library
	//
	if (WWAudioClass::Get_Instance () != NULL) {
		SoundSceneClass *sound_scene = WWAudioClass::Get_Instance ()->Get_Sound_Scene ();
		if (sound_scene != NULL) {
			sound_scene->Attach_Listener_To_Obj (NULL);
		}
	}

	if (DazzleLayer != NULL) {
		delete DazzleLayer;
		DazzleLayer = NULL;
		DazzleRenderObjClass::Set_Current_Dazzle_Layer(NULL);
	}

	MainCamera->Release_Ref();
	MainCamera = NULL;

	CombatSoundManager::Shutdown();

	ObjectiveManager::Shutdown();

	SurfaceEffectsManager::Shutdown();

	CCameraClass::Shutdown();

	ArmorWarheadManager::Shutdown();

	BonesManager::Shutdown();

	ScriptManager::Shutdown();

	if (MessageWindow != NULL) {
		MessageWindow->Shutdown ();
		delete MessageWindow;
		MessageWindow = NULL;
	}

	ConversationMgrClass::Shutdown ();

//	Debug_Say(( "Combat Shutdown %d refs\n", RefCountClass::Total_Refs() ));
}

void	CombatManager::Scene_Init( void )
{
	// Game scene is where the main action occurs!
	GameScene = new PhysicsSceneClass;
	GameScene->Set_Ambient_Light(Vector3(0.55f,0.55f,0.55f));
	GameScene->Set_Ambient_Light(Vector3(1,1,1));
	GameScene->Set_Fog_Color(Vector3(0.6f,0.6f,0.6f)); //Vector3(80.0f/255.0f,130.0f/255.0f,180.0f/255.0f));

	// Do all 'Enable_All's, then all 'Disable_All's, then the individual pairs
	COMBAT_SCENE->Enable_All_Collision_Detections( DEFAULT_COLLISION_GROUP );
	COMBAT_SCENE->Enable_All_Collision_Detections( BULLET_COLLISION_GROUP );
	COMBAT_SCENE->Enable_All_Collision_Detections( TERRAIN_COLLISION_GROUP );
	COMBAT_SCENE->Enable_All_Collision_Detections( PhysicsSceneClass::COLLISION_GROUP_WORLD );
	COMBAT_SCENE->Enable_All_Collision_Detections( SOLDIER_GHOST_COLLISION_GROUP );
	COMBAT_SCENE->Enable_All_Collision_Detections( SOLDIER_COLLISION_GROUP );

	COMBAT_SCENE->Disable_All_Collision_Detections( UNCOLLIDEABLE_GROUP );
	COMBAT_SCENE->Disable_All_Collision_Detections( TERRAIN_ONLY_COLLISION_GROUP );
	COMBAT_SCENE->Disable_All_Collision_Detections( TERRAIN_AND_BULLET_COLLISION_GROUP );
	COMBAT_SCENE->Disable_All_Collision_Detections( BULLET_ONLY_COLLISION_GROUP );

	COMBAT_SCENE->Enable_Collision_Detection( TERRAIN_ONLY_COLLISION_GROUP, TERRAIN_COLLISION_GROUP );
	COMBAT_SCENE->Enable_Collision_Detection( TERRAIN_AND_BULLET_COLLISION_GROUP, TERRAIN_COLLISION_GROUP );
	COMBAT_SCENE->Enable_Collision_Detection( TERRAIN_AND_BULLET_COLLISION_GROUP, BULLET_COLLISION_GROUP );
	COMBAT_SCENE->Disable_Collision_Detection( BULLET_COLLISION_GROUP, BULLET_COLLISION_GROUP );
	COMBAT_SCENE->Enable_Collision_Detection( BULLET_ONLY_COLLISION_GROUP, BULLET_COLLISION_GROUP );
	COMBAT_SCENE->Disable_Collision_Detection( PhysicsSceneClass::COLLISION_GROUP_WORLD,PhysicsSceneClass::COLLISION_GROUP_WORLD );
	COMBAT_SCENE->Disable_Collision_Detection( SOLDIER_GHOST_COLLISION_GROUP, SOLDIER_COLLISION_GROUP );
	COMBAT_SCENE->Disable_Collision_Detection( SOLDIER_GHOST_COLLISION_GROUP, SOLDIER_GHOST_COLLISION_GROUP );
}

/*
** Note:  The editor calls these per level functions only once per program execution.
** If you have code that requires a init/shutdown/reset call with each level load,
** notify Pat so he can add those calls manually
**
*/
void	CombatManager::Pre_Load_Level( bool render_available )
{
	MultiplayRenderingAllowed = true;
	if ( !IS_MISSION && !I_Am_Server() ) {
		MultiplayRenderingAllowed = false;
	}

	HUDClass::Enable( true );
	HUDClass::Reset();

	IsGamePaused = false;

	GameObjObserverManager::Reset();

	CoverManager::Init();

	GameObjManager::Init();

	BulletManager::Init();

	cEncoderList::Clear_Entries();

	cPacket::Init_Encoder();

	BackgroundScene = NEW_REF (SimpleSceneClass, ());

	SoundEnvironment = NEW_REF (SoundEnvironmentClass, ());

	BackgroundMgrClass::Init (BackgroundScene, SoundEnvironment, render_available);

	WeatherMgrClass::Init (SoundEnvironment);

	WeaponViewClass::Init();

	SmartGameObj::Set_Global_Sight_Range_Scale( 1.0f );
//	SoundSystem::Set_Global_Listener_Scale( 1.0f );

	//
	//	Map initialization (temporary -- until its more data driven)
	//
	/*MapMgrClass::Set_Map_Texture ("L1-MAP.TGA");
	MapMgrClass::Set_Map_Center (Vector2 (308.0F, 414.0F));
	MapMgrClass::Set_Map_Scale (Vector2 (0.53F, 0.54F));
	MapMgrClass::Cloud_All_Cells ();*/

	ScreenFadeManager::Enable_Letterbox( 0, 0 );
	ScreenFadeManager::Set_Screen_Overlay_Opacity( 0, 0 );

	HUDInfo::Set_HUD_Help_Text( U_CHAR("") );	// Clear text
}

bool	_preload_assets;
StringClass	_load_map_name;

static class LoadThreadClass : public ThreadClass
{
public:
	LoadThreadClass(const char *thread_name = "Game loader thread") : ThreadClass(thread_name, &Exception_Handler) {}

	void Thread_Function() override {

		CombatManager::Set_Load_Progress( 0 );

		WWMEMLOG(MEM_GAMEDATA);

		WWLOG_PREPARE_TIME_AND_MEMORY("Game loader thread");

		#ifndef PARAM_EDITING_ON
			// Tell the datasafe to expect calls from this thread now.
			GenericDataSafeClass::Set_Preferred_Thread(GetCurrentThreadId());
		#endif // PARAM_EDITING_ON

		CombatManager::Inc_Load_Progress();

		//	Reload the definition databases (to support level-specific temp ddb's)
		INIT_STATUS("Free definition databases");
		DefinitionMgrClass::Free_Definitions();
		WWLOG_INTERMEDIATE("Free definitions");

		INIT_STATUS("Load definition databases");
		SaveGameManager::Load_Definitions();
		WWLOG_INTERMEDIATE("Load definitions");

		CombatManager::Inc_Load_Progress();
		//
		//	Make sure the animated-sound system is setup.  This needs
		// to be done after the definition databases are loaded...
		//
		AnimatedSoundMgrClass::Initialize ();

		StringClass filename_to_load( _load_map_name, true );
		StringClass lsd_filename( _load_map_name,true );

		// Prep the level loading
		INIT_STATUS("Pre Load level");

		PlayerInfoLog::Set_Current_Map_Name(_load_map_name);
		SaveGameManager::Pre_Load_Game(_load_map_name, filename_to_load, lsd_filename);
		CombatManager::Set_Last_LSD_Name( lsd_filename );
		WWLOG_INTERMEDIATE("Preload game");

		CombatManager::Inc_Load_Progress();

		//
		//	Preload the assets
		//
		if ( _preload_assets ) {
			INIT_STATUS("Preload assets");
			AssetDependencyManager::Load_Always_Assets();
			WWLOG_INTERMEDIATE("Preload always assets");

			CombatManager::Inc_Load_Progress();

			AssetDependencyManager::Load_Level_Assets( lsd_filename );
			WWLOG_INTERMEDIATE("Preload level assets");
		} else {
			CombatManager::Inc_Load_Progress();
		}

		CombatManager::Inc_Load_Progress();

		// Now load the level
		INIT_STATUS("Load level");
		SaveGameManager::Load_Game( filename_to_load );
		WWLOG_INTERMEDIATE("Load game");

		CombatManager::Inc_Load_Progress();

	}
} thread;

void	CombatManager::Load_Level_Threaded( const char * map_name, bool preload_assets )
{
	//
	//	Turn off music and sound effects while loading
	//
	IsLevelInitialized = false;
	WWAudioClass::Get_Instance ()->Enable_New_Sounds (false);

	_preload_assets = preload_assets;
	_load_map_name = map_name;

	WWASSERT(!thread.Is_Running());
	thread.Execute();
}

bool	CombatManager::Is_Load_Level_Complete( void )
{
	ThreadClass::Sleep_Ms(50);
	return (thread.Is_Running() == false);
}

bool	CombatManager::Is_Loading_Level( void )
{
	return (thread.Is_Running());
}

/*
** Note:  The editor calls these per level functions only once per program execution.
** If you have code that requires a init/shutdown/reset call with each level load,
** notify Pat so he can add those calls manually
**
*/
void	CombatManager::Post_Load_Level( void )
{
	// Dont reset SyncTime.  Level from the editor should have it at 0, and levels from
	// the game need it set to continue cinematics
//	SyncTime = 0;
	CombatMode = COMBAT_MODE_NONE;
	Input::Flush();
	HitReticleEnabled	 = true;		// Default for on when you start each level

	Clear_Star_Damage_Direction();

#if 0
	// Re-init all objects loaded from editor
	if ( CombatManager::Is_First_Load() ) {
		Debug_Say(( "Re-Initing all game objects\n" ));
		GameObjManager::Init_All();
	}
#endif

	//
	//	Build network wrappers for every static anim object in the level
	//
	StaticNetworkObjectClass::Generate_Static_Network_Objects ();


	// Debug code to create a definition
#if 0
	DefinitionFactoryClass * def_factory = (DefinitionFactoryClass *)DefinitionFactoryMgrClass::Find_Factory( CLASSID_GLOBAL_SETTINGS_DEF_HUMAN_ANIM_OVERRIDE );
	if ( def_factory != NULL ) {
		HumanAnimOverrideDef * def = (HumanAnimOverrideDef *)def_factory->Create();
		if ( def != NULL ) {
			def->Set_Name( "HAO Test" );
			def->Set_ID (DefinitionMgrClass::Get_New_ID (def->Get_Class_ID ()));
			def->WalkEmptyHands = "H_A_432A";
			def->RunEmptyHands = "H_A_J43B";
			DefinitionMgrClass::Register_Definition( def );
		}
	}
#endif

	if (IsLevelInitialized == false && TheStar != NULL) {
		IsLevelInitialized = true;

		//
		//	Turn music and sound effects back on...
		//
		WWAudioClass::Get_Instance ()->Enable_New_Sounds (true);
	}

	//
	//	Generate the unit coordination zones
	//
	UnitCoordinationZoneMgr::Build_Zones ();
	return ;
}

/*
** Note:  The editor calls these per level functions only once per program execution.
** If you have code that requires a init/shutdown/reset call with each level load,
** notify Pat so he can add those calls manually
**
*/

void	CombatManager::Unload_Level( void )
{
	// Display please wait screen.....
	Render2DSentenceClass backdropText;
	backdropText.Set_Texture_Size_Hint( 256 );
	FontCharsClass *font	= StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_BIG_TXT );
	if (font != NULL) {
		backdropText.Set_Font( font );
		WideStringClass wide_str = TRANSLATE( IDS_MP_LOADING );
		backdropText.Build_Sentence( wide_str );
		Vector2 extents = backdropText.Get_Text_Extents( wide_str );
		float x = (Render2DClass::Get_Screen_Resolution().Right - extents.X) / 2.0f;
		float y = (Render2DClass::Get_Screen_Resolution().Bottom - extents.Y) / 2.0f;
		backdropText.Set_Location( Vector2( (int)x, (int)y ) );
		backdropText.Draw_Sentence( 0xFF00FF00 );	// Green
		WW3D::Begin_Render( true, true, Vector3(0.0f,0.0f,0.0f), NULL);
		backdropText.Render();
		WW3D::End_Render();
	}

	WWLOG_PREPARE_TIME_AND_MEMORY("Unload level");

	WWDEBUG_SAY(("CombatManager::Unload_Level\n"));
	SystemInfoLog::Set_State_Exiting();
	WWLOG_INTERMEDIATE("SystemInfoLog::Set_State_Exiting()");

	// Don't log load-on-demands after the game is over
	AssetStatusClass::Peek_Instance()->Enable_Load_On_Demand_Reporting(false);
	WWLOG_INTERMEDIATE("AssetStatusClass::Peek_Instance()->Enable_Load_On_Demand_Reporting(false)");

	//
	//	Free the static anim phys object network wrappers
	//
	StaticNetworkObjectClass::Free_Static_Network_Objects ();
	WWLOG_INTERMEDIATE("StaticNetworkObjectClass::Free_Static_Network_Objects ()");

	WeaponViewClass::Shutdown();
	WWLOG_INTERMEDIATE("WeaponViewClass::Shutdown()");

	GameObjManager::Shutdown();
	WWLOG_INTERMEDIATE("GameObjManager::Shutdown()");

	CoverManager::Shutdown();
	WWLOG_INTERMEDIATE("CoverManager::Shutdown()");

	BulletManager::Shutdown();
	WWLOG_INTERMEDIATE("BulletManager::Shutdown()");

	ObjectiveManager::Reset();
	WWLOG_INTERMEDIATE("ObjectiveManager::Reset()");

	WeatherMgrClass::Shutdown();
	WWLOG_INTERMEDIATE("WeatherMgrClass::Shutdown()");

	BackgroundMgrClass::Shutdown();
	WWLOG_INTERMEDIATE("BackgroundMgrClass::Shutdown()");

	REF_PTR_RELEASE (SoundEnvironment);
	WWLOG_INTERMEDIATE("REF_PTR_RELEASE (SoundEnvironment)");

	REF_PTR_RELEASE (BackgroundScene);
	WWLOG_INTERMEDIATE("REF_PTR_RELEASE (BackgroundScene)");

	WW3DAssetManager::Get_Instance()->Free_Assets();	// Free all assets
	WWLOG_INTERMEDIATE("WW3DAssetManager::Get_Instance()->Free_Assets()");

	WWLOG_INTERMEDIATE("Delete factories");

	//
	//	Kill the background music and flush the cache
	//
	if (WWAudioClass::Get_Instance () != NULL) {
		WWAudioClass::Get_Instance ()->Set_Background_Music (NULL);
		WWAudioClass::Get_Instance ()->Flush_Cache ();
	}
	WWLOG_INTERMEDIATE("Flush audio cache");

	SystemInfoLog::Reset_State();
	WWLOG_INTERMEDIATE("SystemInfoLog::Reset_State()");

	if (MessageWindow != NULL) {
		MessageWindow->Clear();
		MessageWindow->Clear_Log();
	}

	//
	//	Shutdown the animated-sound system
	//
	AnimatedSoundMgrClass::Shutdown();
	WWLOG_INTERMEDIATE("AnimatedSoundMgrClass::Shutdown");
	return ;
}

/*
**
*/
PhysicsSceneClass	*	CombatManager::Get_Scene( void )
{
	return PhysicsSceneClass::Get_Instance();
}

/*
** Note: Generate_Control and Think are separated because network update needs
** to happen in-between them.
*/

/*
**
*/
void 	CombatManager::Generate_Control( void )
{
	WWPROFILE( "Generate Control" );
	// First, generate input control for all necessary game objects and
	// possibly send those control to the server
	GameObjManager::Generate_Control();
}

/*
**
*/
void 	CombatManager::Handle_Input()
{
	if ( Input::Get_State( INPUT_FUNCTION_FIRST_PERSON_TOGGLE ) ) {
		Set_First_Person( !Is_First_Person() );

		if ( COMBAT_STAR ) {
			Vector3 pos;
			COMBAT_STAR->Get_Position( &pos );
			DIAG_LOG(( "SWPE", "%1.2f; %1.2f; %1.2f; %s ", pos.X, pos.Y, pos.Z, Is_First_Person() ? "First" : "Third" ));
		}
	}
}


/*
**
*/
void 	CombatManager::Think()
{
	SyncTime += (int)((TimeManager::Get_Frame_Seconds() * 1000.0f) + 0.5f);

	WWPROFILE( "CombatManager Think" );

	IsGameplayPermitted=NetworkHandler->Is_Gameplay_Permitted();

{	WWPROFILE( "Input" );
	Handle_Input();
}
//	GroupControllerManager::Update();

	//
	//	Display debug boxes for the coordinate zones as necessary
	//
	if ( SoldierGameObj::Is_Ghost_Collision_Debug_Display_Enabled() ) {
		UnitCoordinationZoneMgr::Display_Debug_Boxes();
	}

{	WWPROFILE( "Bullets" );
	BulletManager::Update();
}

	ObjectiveManager::Update( TimeManager::Get_Frame_Seconds() );

	// Now, Process all objects logically
	ConversationMgrClass::Think();
{	WWPROFILE( "Game Obj Think" );
	GameObjManager::Think();

	// Now, Process all objects physically
}{	WWPROFILE( "Scene" );
  	COMBAT_SCENE->Update( TimeManager::Get_Frame_Seconds(), 0 );

}{	WWPROFILE( "Star" );
	Update_Star();

	// In normal mode, the camera must think before Post_Think, since the
	// camera update calls Set_Targeting on the star, which must feed Update_Animation
}{	WWPROFILE( "Camera 1" );
	if ( !MainCamera->Is_Using_Host_Model() ) {
		MainCamera->Update();
	}

	// Now, Post Process all objects logically
}{	WWPROFILE( "Post Think" );
	GameObjManager::Post_Think();
}
	// In host_model mode, the camera must think after Post_Think, so the host model has
	// a chance to determine where the camera should be
	if ( MainCamera->Is_Using_Host_Model() ) {
		MainCamera->Update();
	}

	// The targeting comes from the update weapons in the post_think
	Update_Star_Targeting();

	Do_Skeleton_Slider_Demo();

{	WWPROFILE( "Message Window" );
	MessageWindow->On_Frame_Update();
}

	SpawnManager::Update();

{	WWPROFILE( "Sound Environment" );

	if ( SoundEnvironment != NULL ) {
		SoundEnvironment->Update (COMBAT_SCENE, MainCamera);
	}
}

{	WWPROFILE( "Background" );

	BackgroundMgrClass::Update (COMBAT_SCENE, MainCamera);
}

{  WWPROFILE( "Weather" );

	WeatherMgrClass::Update (COMBAT_SCENE, MainCamera);
}

	HUDClass::Think();
	WeaponViewClass::Think();
	ScreenFadeManager::Think();
}

/*
**
*/
void CombatManager::Render()
{
	if ( COMBAT_STAR != NULL ) {
		MultiplayRenderingAllowed = true;
	}

	if ( MultiplayRenderingAllowed ) {
		SystemInfoLog::Record_Frame();

		{
			WWPROFILE( "Camera Shakes" );
			COMBAT_SCENE->Apply_Camera_Shakes (*MainCamera);
		}

		DazzleRenderObjClass::Install_Dazzle_Visibility_Handler(&_TheCombatDazzleHandler);

		{
			WWPROFILE( "Combat Render BG" );

			WW3D::Render (BackgroundScene, MainCamera);
		}

		{
			WWPROFILE( "Combat Render FG" );
			WW3D::Render(COMBAT_SCENE, MainCamera);
		}

		{
			WWPROFILE( "DazzleRenderer" );
			DazzleLayerClass * dlayer = COMBAT_DAZZLE_LAYER;
			if (dlayer != NULL) {
				dlayer->Render(COMBAT_CAMERA);
			}
		}

		DazzleRenderObjClass::Install_Dazzle_Visibility_Handler(NULL);

		HUDClass::Render();

		ScreenFadeManager::Render();
	}
}

/*
**
*/
enum	{
	CHUNKID_THE_STAR							=	916991712,
	CHUNKID_FIRST_LOAD,
	CHUNKID_NOT_FIRST_LOAD,				// Delete this one, ( and change default to false )
	CHUNKID_VARIABLES,
	CHUNKID_CCAMERA,

	MICROCHUNKID_FIRST_LOAD					=	1,
	MICROCHUNKID_DIFFICULTY_LEVEL			=	2,
	MICROCHUNKID_SYNC_TIME					=	3,
	MICROCHUNKID_START_SCRIPT,
	MICROCHUNKID_RESPAWN_SCRIPT,
	MICROCHUNKID_RELOAD_COUNT,
	MICROCHUNKID_CHEAT_HISTORY,
	MICROCHUNKID_FIRST_PERSON,
};

bool	CombatManager::Save( ChunkSaveClass &csave )
{
	csave.Begin_Chunk( CHUNKID_THE_STAR );
	TheStar.Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_VARIABLES );
		bool	first_load = !Are_Observers_Active();
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_FIRST_LOAD, first_load );
		if ( !first_load ) {
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DIFFICULTY_LEVEL, DifficultyLevel );
			WRITE_MICRO_CHUNK( csave, MICROCHUNKID_FIRST_PERSON, FirstPerson );
		}
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_SYNC_TIME, SyncTime );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_START_SCRIPT, StartScript );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_RESPAWN_SCRIPT, RespawnScript );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_RELOAD_COUNT, ReloadCount );

	#ifndef PARAM_EDITING_ON
		int cheat_history = CheatMgrClass::Get_Instance()->Get_History();
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_CHEAT_HISTORY, cheat_history );
	#endif //PARAM_EDITING_ON

	csave.End_Chunk();

	if ( COMBAT_CAMERA != NULL ) {
		csave.Begin_Chunk( CHUNKID_CCAMERA );
			COMBAT_CAMERA->Save( csave );
		csave.End_Chunk();
	}

	return true;
}

bool	CombatManager::Load( ChunkLoadClass &cload )
{
	ReloadCount = 0;		// Legacy
	IsFirstLoad = true;		// Default
	int legacy_dificulty_level = DifficultyLevel;
	int cheat_history = 0;
	FirstPerson = FirstPersonDefault;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_THE_STAR:
				TheStar.Load( cload );
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, MICROCHUNKID_FIRST_LOAD, IsFirstLoad );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DIFFICULTY_LEVEL, DifficultyLevel );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_SYNC_TIME, SyncTime );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_START_SCRIPT, StartScript );
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_RESPAWN_SCRIPT, RespawnScript );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_RELOAD_COUNT, ReloadCount );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_CHEAT_HISTORY, cheat_history );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_FIRST_PERSON, FirstPerson );
						default:
							Debug_Say(( "Unrecognized CombatManager variable chunkID\n" ));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_CCAMERA:
				if ( COMBAT_CAMERA != NULL ) {
					COMBAT_CAMERA->Load( cload );
				}
				break;

			default:
				Debug_Say(( "Unrecognized CombatManager chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	if ( IsFirstLoad ) {
		DifficultyLevel = legacy_dificulty_level;
		ReloadCount = 0;
		CheatMgrClass::Get_Instance()->Reset_History();
	} else {
		ReloadCount++;		// Count another load!
		CheatMgrClass::Get_Instance()->Set_History( cheat_history );
		CheatMgrClass::Get_Instance()->Update_History();
	}

	return true;
}

/*
**
*/
bool CombatManager::Can_Damage(ArmedGameObj * p_armed_damager, PhysicalGameObj * p_phys_victim)
{
	if (NetworkHandler != NULL) {
		return NetworkHandler->Can_Damage(p_armed_damager, p_phys_victim);
	} else {
		return true;
	}
}

float CombatManager::Get_Damage_Factor(ArmedGameObj * p_armed_damager, PhysicalGameObj * p_phys_victim)
{
	if (NetworkHandler != NULL) {
		return NetworkHandler->Get_Damage_Factor(p_armed_damager, p_phys_victim);
	} else {
		return 1.0f;
	}
}

void CombatManager::On_Soldier_Kill(SoldierGameObj * p_soldier, SoldierGameObj * p_victim)
{
	WWASSERT(p_soldier != NULL);
	WWASSERT(p_victim != NULL);

	if (NetworkHandler != NULL) {
		 NetworkHandler->On_Soldier_Kill(p_soldier, p_victim);
	}
}

void CombatManager::On_Soldier_Death(SoldierGameObj * p_soldier)
{
	WWASSERT(p_soldier != NULL);

	if (NetworkHandler != NULL) {
		 NetworkHandler->On_Soldier_Death(p_soldier);
	}
}

bool CombatManager::Is_Gameplay_Permitted(void)
{
	if (NetworkHandler == NULL) {
		return true;
	} else {
		return IsGameplayPermitted;
//		return NetworkHandler->Is_Gameplay_Permitted();
	}
}

/*
** Misc Handler
*/
void	CombatManager::Mission_Complete( bool success )
{
	if (MiscHandler != NULL) {
		MiscHandler->Mission_Complete( success );
	}
}

void	CombatManager::Star_Killed( void )
{
	if (MiscHandler != NULL) {
		MiscHandler->Star_Killed();
	}
}

/*
**
*/
void	CombatManager::Set_Camera_Profile( const char * profile_name )
{
	if ( profile_name == NULL ) {
		COMBAT_CAMERA->Use_Default_Profile();
	} else {
		COMBAT_CAMERA->Use_Profile( profile_name );
	}
}

/*
**
*/
void 	CombatManager::Set_The_Star( SoldierGameObj *target, bool is_star_determining_target )
{
	if ( 	TheStar != target ) {
		HUDClass::Reset();	// Reset the HUD (clear damage and powerups)
	}

	TheStar = target;
	IsStarDeterminingTarget = is_star_determining_target;
	if ( target != NULL ) {
		COMBAT_CAMERA->Force_Heading( target->Get_Facing() );
	}

	HUDClass::Force_Weapon_Chart_Update();
	WeaponViewClass::Reset();

	if (IsLevelInitialized == false) {
		IsLevelInitialized = true;

		//
		//	Turn music and sound effects back on...
		//
		WWAudioClass::Get_Instance ()->Enable_New_Sounds (true);
	}
}

/*
**
*/
void CombatManager::Update_Star( void )
{
	SoldierGameObj * star = COMBAT_STAR;
	if ( star == NULL ) {
		return;
	}

	if ( star->Is_Destroyed() ) {
		Set_Combat_Mode( COMBAT_MODE_CORPSE );
	} else if ( star->Is_Dead() ) {
		Set_Combat_Mode( COMBAT_MODE_DIEING );
	} else if ( star->Get_Profile_Vehicle() != NULL ) {
		Set_Combat_Mode( COMBAT_MODE_IN_VEHICLE );
	} else if ( star->Is_Sniping() ) {
		Set_Combat_Mode( COMBAT_MODE_SNIPING );
	} else if ( star->Use_Ladder_View() ) {
		Set_Combat_Mode( COMBAT_MODE_ON_LADDER );
	} else if ( Is_First_Person() ) {
		Set_Combat_Mode( COMBAT_MODE_FIRST_PERSON );
	} else {
		Set_Combat_Mode( COMBAT_MODE_THIRD_PERSON );
	}

	Update_Combat_Mode();

	//
	//	Update the map
	//
	Vector3 star_pos;
	star->Get_Position (&star_pos);
	MapMgrClass::Clear_Cloud_Cell (star_pos);
	return ;
}

void CombatManager::Update_Star_Targeting( void )
{
	WWPROFILE( "Targeting" );
	SmartGameObj * star = NULL;

	if ( COMBAT_STAR != NULL ) {
		SoldierGameObj * soldier = COMBAT_STAR;

	   WWASSERT(soldier != NULL);
		star = soldier;
		if ( soldier->Get_Vehicle() ) {
			star = soldier->Get_Vehicle();
		}

		// Now, if the star has a weapon, gather the information needing to be displayed
		// such as where the bullets will hit, what we are targeting, distance, etc.
		HUDInfo::Set_Weapon_Target_Object( NULL );
		WeaponClass * weapon = star->Get_Weapon();
		if ( weapon ) {
			WWPROFILE( "Display_Targeting" );
			weapon->Display_Targeting();
		}
	}
}

bool CombatManager::Is_In_Camera_Frustrum(Vector3 & position)
{
	WWASSERT(COMBAT_CAMERA != NULL);
   Vector3 projected_mid_point;
	bool is_visible = (COMBAT_CAMERA->Project(projected_mid_point, position) == CameraClass::INSIDE_FRUSTUM);
	return is_visible;
}

void	CombatManager::Do_Skeleton_Slider_Demo( void )
{
	if (( EnableSkeletonSliderDemo == false ) ||
		 ( COMBAT_CAMERA->Is_2D_Targeting() == false )) {
		return;
	}

	if ( COMBAT_STAR != NULL ) {
		Vector2 sliders = COMBAT_CAMERA->Get_Camera_Target_2D_Offset();
		// Slide is 0-1, height * widht should be -scale - scale
		sliders.X = (sliders.X - 0.5f) * 2;
		sliders.Y = (sliders.Y - 0.5f) * -2;
		float scale = 1;
		float height = sliders.Y * scale;
		float width = sliders.X * scale;
//		Debug_Say(( "Height %f, width %f\n", height, width ));
		COMBAT_STAR->Adjust_Skeleton( height, width );
	}
}


/*
**
*/
void	Vehicle_Panic( VehicleGameObj * vehicle )
{
	VehiclePhysClass * rbody = vehicle->Peek_Vehicle_Phys();
	if (rbody != NULL) {
#if 1
		// Try to put the vehicle up-right in its current position
		Matrix3D tm = rbody->Get_Transform();
		Vector3 position;
		Vector3 forward;

		tm.Get_Translation(&position);
		tm.Get_X_Vector(&forward);
		if (forward.Z > 0.707f) {
			tm.Get_Z_Vector(&forward);
		}

		forward.Z = 0.0f;
		position.Z += 3.0f;
		Matrix3D new_tm;
		new_tm.Obj_Look_At(position,position+forward,0.0f);

		rbody->Set_Transform(new_tm);
#endif
	}
}

/*
** Combat Mode Management
*/
void	CombatManager::Set_Combat_Mode( int mode )
{
	// I don't know how to handle loading a saved game in cinematics, so postpone
	// this mode change until the camera is not host controlled?
	if ( COMBAT_CAMERA && COMBAT_CAMERA->Is_Using_Host_Model() ) {
		return;
	}

	if ( mode != CombatMode ) {
		//Debug_Say(( "Switching CombatMode to %d\n", mode ));

		// When changing to or from ladder, lerp
		if ( ( CombatMode == COMBAT_MODE_ON_LADDER ) || ( mode == COMBAT_MODE_ON_LADDER ) ) {
			COMBAT_CAMERA->Set_Lerp_Time( 0.5f );
		}

		CombatMode = mode;

		// Pause game in single player corpse mode
//		IsGamePaused = (CombatMode == COMBAT_MODE_CORPSE) && IS_SOLOPLAY;

		SoldierGameObj * star = COMBAT_STAR;

		switch ( CombatMode ) {

			case COMBAT_MODE_NONE:
				break;

			case COMBAT_MODE_FIRST_PERSON:
				COMBAT_CAMERA->Set_Is_Star_Sniping( false );
				COMBAT_CAMERA->Use_Profile( "First_Person" );
				COMBAT_CAMERA->Set_Profile_Distance( 0 );
				star->Peek_Model()->Set_Hidden( true );
				WeaponViewClass::Enable( true );
				break;

			case COMBAT_MODE_ON_LADDER:
			case COMBAT_MODE_THIRD_PERSON:
				star->Peek_Model()->Set_Hidden( false );
				COMBAT_CAMERA->Set_Is_Star_Sniping( false );
				COMBAT_CAMERA->Use_Default_Profile();
				WeaponViewClass::Enable( false );
				break;

			case COMBAT_MODE_SNIPING:
				COMBAT_CAMERA->Set_Is_Star_Sniping( true );
				star->Peek_Model()->Set_Hidden( true );
				COMBAT_CAMERA->Use_Profile( "Sniper" );
				COMBAT_CAMERA->Set_Profile_Height( star->Get_Weapon_Height() );
				COMBAT_CAMERA->Set_Profile_Distance( -star->Get_Weapon_Length() );
				COMBAT_CAMERA->Enable_2D_Targeting( false );
				WeaponViewClass::Enable( false );
				break;

			case COMBAT_MODE_IN_VEHICLE:
			{
				VehicleGameObj * vehicle = star->Get_Profile_Vehicle();
				WWASSERT( vehicle );
	 			COMBAT_CAMERA->Set_Is_Star_Sniping( false );
				if ( vehicle->Get_Profile() != NULL ) {
					COMBAT_CAMERA->Use_Profile( vehicle->Get_Profile() );
				} else {
					COMBAT_CAMERA->Use_Default_Profile();
				}
				WeaponViewClass::Enable( false );
			}
				break;

			case COMBAT_MODE_DIEING:
				star->Peek_Model()->Set_Hidden( false );
				COMBAT_CAMERA->Set_Is_Star_Sniping( false );
				COMBAT_CAMERA->Use_Profile( "Death" );
				COMBAT_CAMERA->Set_Lerp_Time( 0.5f );
				WeaponViewClass::Enable( false );
				break;

			case COMBAT_MODE_CORPSE:
				break;

			case COMBAT_MODE_SNAP_SHOT:
				break;

			default:
				Debug_Say(( "Bad CombatMode %d\n", CombatMode ));
				CombatMode = COMBAT_MODE_NONE;
				break;

		}
	}
}

void	CombatManager::Update_Combat_Mode( void )
{
	SoldierGameObj * star = COMBAT_STAR;
	Vector3	pos;
	star->Get_Position( &pos );
	COMBAT_CAMERA->Set_Anchor_Position( pos );

	//XXX
	//COMBAT_CAMERA->Enable_2D_Targeting(true);

	switch ( CombatMode ) {

		case COMBAT_MODE_NONE:
			break;

		case COMBAT_MODE_FIRST_PERSON:
			star->Peek_Model()->Set_Hidden( true );	// Putting it here, because vehicles are turning it back on
			COMBAT_CAMERA->Set_Profile_Height( star->Get_Weapon_Height() );	// This is every frame for crouching
//			COMBAT_CAMERA->Enable_2D_Targeting( Input::Get_State(INPUT_FUNCTION_CURSOR_TARGETING) );
			COMBAT_CAMERA->Enable_2D_Targeting( false );
			break;

		case COMBAT_MODE_ON_LADDER:
		case COMBAT_MODE_THIRD_PERSON:
			COMBAT_CAMERA->Enable_2D_Targeting( Input::Get_State(INPUT_FUNCTION_CURSOR_TARGETING ) );

			if ( star->Is_On_Ladder() ) {
				COMBAT_CAMERA->Force_Heading( star->Get_Facing() );
			}
			break;

		case COMBAT_MODE_SNIPING:
			COMBAT_CAMERA->Set_Profile_Height( star->Get_Weapon_Height() );	// This is every frame for crouching
			break;

		case COMBAT_MODE_IN_VEHICLE:
		{
			VehicleGameObj * vehicle = star->Get_Profile_Vehicle();
			WWASSERT( vehicle );

#ifdef WWDEBUG
			if ( Input::Get_State( INPUT_FUNCTION_PANIC ) ) {
				Vehicle_Panic( vehicle );
			}
#endif

			Vector3	pos;
			vehicle->Get_Position( &pos );
			COMBAT_CAMERA->Set_Anchor_Position( pos );

//			bool target_2d = ( !vehicle->Use_2D_Aiming() ^ Input::Get_State(INPUT_FUNCTION_CURSOR_TARGETING) ^ VehicleGameObj::Is_Target_Steering() );

			bool target_2d = (Input::Get_State(INPUT_FUNCTION_CURSOR_TARGETING) ^ VehicleGameObj::Is_Target_Steering() );

			// This can be used to always for the V key down for vehicles
			if ( VehicleGameObj::Get_Camera_Locked_To_Turret() ) {
				target_2d = !target_2d;
			}

			if ( !vehicle->Use_2D_Aiming() ) {
				target_2d = true;
			}

			if ( vehicle->Get_Gunner() == COMBAT_STAR ) {
				target_2d = true;
			}

			if ( target_2d ) {
				COMBAT_CAMERA->Enable_2D_Targeting( false );
			} else {
				COMBAT_CAMERA->Enable_2D_Targeting( true );

	// Lateral velocity offset camera code
				float desired_heading = vehicle->Get_Facing();;
				VehiclePhysClass * vehiclephys = vehicle->Peek_Physical_Object()->As_VehiclePhysClass();
				if (vehiclephys != NULL) {
					Vector3 vel,localvel;
					vehiclephys->Get_Velocity(&vel);

					Matrix3D invtm;
					vehiclephys->Get_Transform().Get_Orthogonal_Inverse(invtm);
					Matrix3D::Rotate_Vector(invtm,vel,&localvel);

	//				float lateralvel = localvel.Y;
					float heading_offset = WWMath::Atan2(localvel.Y,localvel.X);
					heading_offset = WWMath::Clamp(heading_offset,DEG_TO_RADF(-45.0f),DEG_TO_RADF(45.0f));

					float offset_scale = localvel.X / 5.0f;
					offset_scale = WWMath::Clamp(offset_scale,0.0f,1.0f);
					heading_offset *= offset_scale;

					desired_heading += heading_offset;
				}
				COMBAT_CAMERA->Force_Heading( desired_heading );
				COMBAT_CAMERA->Set_Tilt( 0 );
			}
		}
			break;

		case COMBAT_MODE_DIEING:

			// Make the camera look towards the star killer
			if ( StarKillerID != 0 ) {
				PhysicalGameObj * killer = GameObjManager::Find_PhysicalGameObj( StarKillerID );
				if ( killer ) {
					Vector3 star_pos;
					Vector3 killer_pos;
					COMBAT_STAR->Get_Position( &star_pos );
					killer->Get_Position( &killer_pos );

					Vector3 dif = killer_pos - star_pos;
					float heading = WWMath::Atan2(dif.Y,dif.X);
					COMBAT_CAMERA->Force_Heading( heading );
				}
			}

			break;

		case COMBAT_MODE_CORPSE:
			break;

		case COMBAT_MODE_SNAP_SHOT:
			break;

	}
}

void	CombatManager::Register_Star_Killer( ArmedGameObj * killer )
{
	if ( killer != NULL ) {
		StarKillerID = killer->Get_ID();
	} else {
		StarKillerID = 0;
	}
}





