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
 *                     $Archive:: /Commando/Code/Commando/combatgmode.cpp                     $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/23/02 2:04p                                               $*
 *                                                                                             *
 *                    $Revision:: 267                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "combatgmode.h"
#include "level.h"
#include "input.h"
#include "cnetwork.h"
#include "AudibleSound.h"
#include "debug.h"
#include "registry.h"
#include "_globals.h"
#include "console.h"
#include "radar.h"
#include "spawn.h"
#include "ccamera.h"
#include "scorescreen.h"
#include "devoptions.h"
#include "useroptions.h"
#include "systemsettings.h"
#include "gameobjmanager.h"
#include "building.h"
#include "saveloadstatus.h"
#include "playertype.h"
#include "bitpackids.h"
#include "vehicle.h"
#include "doors.h"
#include "elevator.h"
#include "msgloop.h"
#include "assetmgr.h"
#include "font3d.h"
#include "wwprofile.h"
#include "gametype.h"
#include "messagewindow.h"
#include "objectives.h"
#include "multihud.h"
#include "textureloader.h"
#include "wolgmode.h"
#include "renegadedialogmgr.h"
#include "textdisplay.h"
#include "playermanager.h"
#include "teammanager.h"
#include "consolefunction.h"
#include "overlay.h"
#include "gameinitmgr.h"
#include "campaign.h"
#include "diaglog.h"
#include "god.h"
#include "sbbomanager.h"
#include "savegame.h"
#include "commandosaveload.h"
#include "dlgmpingamechat.h"
#include "loadingevent.h"
#include "cheatmgr.h"
#include "menubackdrop.h"
#include "translatedb.h"
#include "thread.h"
#include "dlgcncteaminfo.h"
#include "dlgcncbattleinfo.h"
#include "bandwidthgraph.h"
#include "suicideevent.h"
#include "assetstatus.h"
#include "stylemgr.h"
#include "gametype.h"
#include "wwmemlog.h"
#include "directinput.h"
#include "CNCModeSettings.h"
#include "AnnounceEvent.h"
#include "FastAllocator.h"
#include "dlgcncserverinfo.h"
#include "radiocommanddisplay.h"
#include "hudinfo.h"
#include "string_ids.h"
#include "radiocommanddisplay.h"
#include "ConsoleMode.h"
#include "specialbuilds.h"
#include <wwlib/realcrc.h>
#include "demosupport.h"
#include "natter.h"
#include "gamespyadmin.h"
#include "dialogtests.h"
#include "dialogmgr.h"
#include "GameSpy_QnR.h"
#include <cstdio>

/*
**
*/
extern bool g_is_loading;
bool g_b_core_restart;//TSS081401
bool g_client_quit = false;

AudibleSoundClass	*	CombatGameModeClass::BackgroundMusic	= NULL;
int						CombatGameModeClass::IsHudShown			= true;
static int				ForceGod											= false;
static bool				ForceGodPending								= true;
static int				DefaultToFirstPerson							= true;
static bool				PendingCampaignContinue						= false;


/*
**
*/
class	CombatGameMiscHandlerClass : public CombatMiscHandlerClass {
public:
	virtual	void	Mission_Complete( bool success ) override;
	virtual	void	Star_Killed( void ) override;
};

CombatGameMiscHandlerClass		GameMiscHandler;

/*
**
*/
static void Combat_To_Menu (RenegadeDialogMgrClass::LOCATION location)
{
   //
	// Remember: Combat and Menu are not mutually exclusive.
	// Therefore this function may be called when menu is already active.
	//
	if (!GameModeManager::Find("Menu")->Is_Active()) {

		//
		// Do not suspend Combat in Multiplayer game!
		//
		if (IS_SOLOPLAY) {
			GameModeManager::Find( "Combat" )->Suspend();	// suspend Combat Mode
		}

		//if (IS_SOLOPLAY) {
		if (IS_MISSION) {
			RenegadeDialogMgrClass::Goto_Location (location);
		} else {
			RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_CNC_REFERENCE);
		}
	}
}

/*
**
*/
static void Start_In_Game_Help(void)
{
	//
	// Remember: Combat and Menu are not mutually exclusive.
	// Therefore this function may be called when menu is already active.
	//
	if (!GameModeManager::Find("Menu")->Is_Active()) {

		//
		// Do not suspend Combat in Multiplayer game!
		//
		if (IS_SOLOPLAY) {
			GameModeManager::Find( "Combat" )->Suspend();	// suspend Combat Mode
		}

		RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_IN_GAME_HELP);
	}
}

void	CombatGameModeClass::Combat_Keyboard( void )
{
	WWPROFILE( "Combat_Keyboard" );

#ifdef ATI_DEMO_HACK
	// HACK: Make double-ESC exit back to desktop
	static unsigned esc_pressed_time=0x7ffff;
	if ( Input::Get_State( INPUT_FUNCTION_MENU_TOGGLE ) ) {
		unsigned current_time=WW3D::Get_Sync_Time();
		if ((current_time-esc_pressed_time)<5000) {
			cDevOptions::QuickFullExit.Set(true);
			esc_pressed_time=0x7ffff;
		}
		else {
			Get_Text_Display()->Print_System( "Press ESC again to exit to desktop...\n" );
		}
		esc_pressed_time=current_time;
	}


	// HACK: In ATI demo toggle the statistics display using TAB
	if ( Input::Get_State( INPUT_FUNCTION_EVA_MISSION_OBJECTIVES_TOGGLE ) ) {
		if (StatisticsDisplayManager::Is_Current_Display( "fps" )) {
			StatisticsDisplayManager::Set_Display( "off" );
		}
		else {
			StatisticsDisplayManager::Set_Display( "fps" );
		}
	}
#else
	bool to_menu=Input::Get_State( INPUT_FUNCTION_MENU_TOGGLE );
	// If game doesn't have a focus and combat is active, suspend combat by entering menu
	if (!GameInFocus) {
		if (IS_SOLOPLAY && COMBAT_CAMERA && !COMBAT_CAMERA->Is_In_Cinematic() ) {
			to_menu|=GameModeManager::Find( "Combat" )->Is_Active();
		}
	}
	if ( to_menu ) {
		  Combat_To_Menu( RenegadeDialogMgrClass::LOC_ENCYCLOPEDIA );
	}

	//
	//	Handle the in-game EVA mission objectives window
	//
	if ( IS_MISSION && Input::Get_State( INPUT_FUNCTION_EVA_MISSION_OBJECTIVES_TOGGLE ) ) {
		if ( ObjectiveManager::Is_Viewer_Displayed() ) {
			ObjectiveManager::Page_Down_Viewer();
		} else {
			ObjectiveManager::Display_Viewer( true );
		}
   }
#endif

	if ( Input::Get_State( INPUT_FUNCTION_HELP_SCREEN ) ) {
		Start_In_Game_Help();
	}

	if ( IS_SOLOPLAY && Input::Get_State( INPUT_FUNCTION_EVA_OBJECTIVES_SCREEN ) ) {
		Combat_To_Menu( RenegadeDialogMgrClass::LOC_OBJECTIVES );
	}

	if ( IS_SOLOPLAY && Input::Get_State( INPUT_FUNCTION_EVA_MAP_SCREEN ) ) {
		Combat_To_Menu( RenegadeDialogMgrClass::LOC_MAP );
	}

	//
	// Display the server info screen
	//
	if (!IS_MISSION && Input::Get_State( INPUT_FUNCTION_SERVER_INFO_TOGGLE )) {
		START_DIALOG (CNCServerInfoDialogClass);
	}

	//
	//	Display the team information page
	//
	if (!IS_MISSION && cNetwork::I_Am_Client() && COMBAT_STAR != NULL) {
		if (Input::Get_State( INPUT_FUNCTION_TEAM_INFO_TOGGLE )) {
			START_DIALOG (CNCTeamInfoDialogClass);
		} else if (Input::Get_State( INPUT_FUNCTION_BATTLE_INFO_TOGGLE )) {
			START_DIALOG (CNCBattleInfoDialogClass);
		}/* else if (Input::Get_State( INPUT_FUNCTION_SERVER_INFO_TOGGLE )) {
			START_DIALOG (CNCServerInfoDialogClass);
		}*/

		// Handle radio commands
		for (int radioCmdIndex = INPUT_FUNCTION_RADIO_CMD_01; radioCmdIndex <= INPUT_FUNCTION_RADIO_CMD_30; ++radioCmdIndex) {
			if (Input::Get_State((InputFunction)radioCmdIndex)) {
				CNCModeSettingsDef* cncDef = CNCModeSettingsDef::Get_Instance();

				if (cncDef) {
					int radioCmdNum = (radioCmdIndex - INPUT_FUNCTION_RADIO_CMD_01);
					int radioCmd = cncDef->Get_Radio_Command(radioCmdNum);

					if (radioCmd) {
						CSAnnouncement* announce = new CSAnnouncement;
						assert(announce != NULL);
						announce->Init(cNetwork::Get_My_Team_Number(), radioCmd, ANNOUNCEMENT_TEAM, radioCmdNum);
						RadioCommandDisplayClass::Reset_Display_Timer();
					}
				}

				break;
			}
		}
	}

	if (!IS_MISSION && Input::Get_State( INPUT_FUNCTION_EVA_MISSION_OBJECTIVES_TOGGLE )) {
		//MultiHUDClass::Toggle_Verbose_Lists();
		MultiHUDClass::Next_Playerlist_Format();
	}

#ifdef WWDEBUG
   if ( Input::Get_State( INPUT_FUNCTION_QUICK_FULL_EXIT ) ) {
		cDevOptions::QuickFullExit.Set(true);
		WWDEBUG_SAY(("Quick full exit requested by keypress.\n"));
	}
	if (cDevOptions::QuickFullExit.Is_True()) {

		GameInitMgrClass::End_Game();

		GameInitMgrClass::End_Client_Server();

		extern void Stop_Main_Loop (int exitCode);
		Stop_Main_Loop (EXIT_SUCCESS);
	}
#endif

	if (	(The_Game() != NULL &&
		    (Input::Get_State(INPUT_FUNCTION_BEGIN_PUBLIC_MESSAGE) ||
		     (Input::Get_State(INPUT_FUNCTION_BEGIN_TEAM_MESSAGE) && cNetwork::I_Am_Client()))) &&
			!IS_MISSION)
	{
		MPIngameChatPopupClass * p_dialog = new MPIngameChatPopupClass;
		WWASSERT(p_dialog != NULL);

		//
		//	Configure the dialog
		//
		if (Input::Get_State(INPUT_FUNCTION_BEGIN_PUBLIC_MESSAGE)) {
			p_dialog->Set_Default_Type(TEXT_MESSAGE_PUBLIC);
		} else {
			p_dialog->Set_Default_Type(TEXT_MESSAGE_TEAM);
		}

		p_dialog->Start_Dialog();
		REF_PTR_RELEASE (p_dialog);
	}

	if ( Input::Get_State( INPUT_FUNCTION_QUICKSAVE )) {
		// Only for singeplay
		if ( IS_MISSION ) {
			Quick_Save();
		}
	}
}

/*
** called when the mode is activated
*/
void	CombatGameModeClass::Init()
{
	Debug_Say(("CombatGameModeClass::Init\n"));

	if (!IS_MISSION) {
		MultiHUDClass::Init();
	}

	PendingCampaignContinue	= false;

	//
	//	Initialize the radio command display window
	//
	RadioCommandDisplayClass::Initialize ();

	//
	// Notify combat about the state of the CameraLockedToTurret user option.
	//
	VehicleGameObj::Set_Camera_Locked_To_Turret(cUserOptions::CameraLockedToTurret.Get());
}

/*
** called when the mode is deactivated
*/
void 	CombatGameModeClass::Shutdown()
{
	Debug_Say(("CombatGameModeClass::Shutdown\n"));

   Core_Shutdown();

	//
	//	Shutdown the radio command display window
	//
	RadioCommandDisplayClass::Shutdown ();
	return ;
}

class LoadingScreenClass
{
	MenuBackDropClass	backdrop;
	Render2DSentenceClass backdropText;
	Render2DSentenceClass backdropText2;	// The BIG text
	float	LoadTime;
	float	LoadPercentage;
	float	LoadPercentageDrawn;
	float	LoadPercentageClamp;
	float	LoadPercentageRate;

public:
	LoadingScreenClass()
	{
		int color = 0xFFFFFFFF;

		WWMEMLOG(MEM_GAMEDATA);

		backdropText.Set_Texture_Size_Hint( 256 );
		backdropText2.Set_Texture_Size_Hint( 256 );

		LoadTime = 0.001f;
		LoadPercentage = 0;
		LoadPercentageDrawn = 0;
		LoadPercentageRate = 0;

		FontCharsClass *font	= StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_TXT );
		backdropText.Set_Font( font );

		font = StyleMgrClass::Peek_Font( StyleMgrClass::FONT_INGAME_BIG_TXT );
		backdropText2.Set_Font( font );

		// Parse Descriptions
		int count = CampaignManager::Get_Backdrop_Description_Count();
		for ( int i = 0; i < count; i++ ) {
			const char * read = CampaignManager::Get_Backdrop_Description( i );
//			Debug_Say(( "Parse %s\n", read ));
			StringClass desc = read;
			while ( desc.Get_Length() && desc[0] <= ' ' ) desc.Erase( 0, 1 );
				while (desc.Get_Length()) {
					const size_t tail_index = desc.Get_Length() - 1;
					if (desc[static_cast<int>(tail_index)] > ' ') {
						break;
					}
					desc.Erase(static_cast<int>(tail_index), 1);
				}

			// Parse Big Translated Text
			if ( ::strnicmp( "Text2", desc, 5 ) == 0 ) {
				desc.Erase( 0, 5 );
				while ( desc.Get_Length() && desc[0] <= ' ' ) desc.Erase( 0, 1 );
				float x,y;
				::sscanf( desc, "%f,%f,", &x, &y );
				const char * str = ::strchr( desc, ',' );
				if ( str != NULL ) {
					str = ::strchr( str+1, ',' );
					if ( str != NULL ) {
						WideStringClass wide_str = TranslateDBClass::Get_String( str+1 );
						backdropText2.Build_Sentence( wide_str );
						// Scale
						x *= Render2DClass::Get_Screen_Resolution().Right / 640.0f;
						y *= Render2DClass::Get_Screen_Resolution().Bottom / 480.0f;
						backdropText2.Set_Location( Vector2( (int)x, (int)y ) );
						backdropText2.Draw_Sentence( color );
						color=0xFFFFFFFF;
						backdropText2.Set_Wrapping_Width( 0 );
					}
				}
			}

			// Parse Translated Text
			if ( ::strnicmp( "Text", desc, 4 ) == 0 ) {
				desc.Erase( 0, 4 );
				while ( desc.Get_Length() && desc[0] <= ' ' ) desc.Erase( 0, 1 );
				float x,y;
				::sscanf( desc, "%f,%f,", &x, &y );
				const char * str = ::strchr( desc, ',' );
				if ( str != NULL ) {
					str = ::strchr( str+1, ',' );
					if ( str != NULL ) {
						WideStringClass wide_str = TranslateDBClass::Get_String( str+1 );
						backdropText.Build_Sentence( wide_str );
						// Scale
						x *= Render2DClass::Get_Screen_Resolution().Right / 640.0f;
						y *= Render2DClass::Get_Screen_Resolution().Bottom / 480.0f;
						backdropText.Set_Location( Vector2( (int)x, (int)y ) );
						backdropText.Draw_Sentence( color );
						color=0xFFFFFFFF;
						backdropText.Set_Wrapping_Width( 0 );
					}
				}
			}

			// Set Big Wrapping Width
			if ( ::strnicmp( "Wrap2", desc, 5 ) == 0 ) {
				desc.Erase( 0, 5 );
				while ( desc.Get_Length() && desc[0] <= ' ' ) desc.Erase( 0, 1 );
				float w;
				::sscanf( desc, "%f,", &w );
				// Scale
				w *= Render2DClass::Get_Screen_Resolution().Right / 640.0f;
				backdropText2.Set_Wrapping_Width( w );
			}

			// Set Wrapping Width
			if ( ::strnicmp( "Wrap", desc, 4 ) == 0 ) {
				desc.Erase( 0, 4 );
				while ( desc.Get_Length() && desc[0] <= ' ' ) desc.Erase( 0, 1 );
				float w;
				::sscanf( desc, "%f,", &w );
				// Scale
				w *= Render2DClass::Get_Screen_Resolution().Right / 640.0f;
				backdropText.Set_Wrapping_Width( w );
			}

			// Parse test Text
			if ( ::strnicmp( "Test", desc, 4 ) == 0 ) {
				desc.Erase( 0, 4 );
				while ( desc.Get_Length() && desc[0] <= ' ' ) desc.Erase( 0, 1 );
				float x,y;
				::sscanf( desc, "%f,%f,", &x, &y );
				const char * str = ::strchr( desc, ',' );
				if ( str != NULL ) {
					str = ::strchr( str+1, ',' );
					if ( str != NULL ) {
						WideStringClass wide_str;
						wide_str.Convert_From( str+1 );
						backdropText.Build_Sentence( wide_str );
						// Scale
						x *= Render2DClass::Get_Screen_Resolution().Right / 640.0f;
						y *= Render2DClass::Get_Screen_Resolution().Bottom / 480.0f;
						backdropText.Set_Location( Vector2( (int)x, (int)y ) );
						backdropText.Draw_Sentence( color );
						color=0xFFFFFFFF;
						backdropText.Set_Wrapping_Width( 0 );
					}
				}
			}

			// Parse back model
			if ( ::strnicmp( "Model", desc, 5 ) == 0 ) {
				desc.Erase( 0, 5 );
				while ( desc.Get_Length() && desc[0] <= ' ' ) desc.Erase( 0, 1 );

				backdrop.Set_Model(desc);
				StringClass anim_name;
				anim_name.Format( "%s.%s", static_cast<const char*>(desc), static_cast<const char*>(desc) );
				backdrop.Set_Animation( anim_name );
				backdrop.Set_Animation_Percentage( 0 );
			}

			if ( ::strnicmp( "Color", desc, 5 ) == 0 ) {
				desc.Erase( 0, 5 );
				while ( desc.Get_Length() && desc[0] <= ' ' ) desc.Erase( 0, 1 );
				float r,g,b;
				::sscanf( desc, "%f,%f,%f", &r, &g, &b );

				Vector3 c( r/255.0f, g/255.0f, b/255.0f );
				color = c.Convert_To_ARGB();
			}


		}
		SaveLoadStatus::Reset_Status_Count();
	}

	~LoadingScreenClass()
	{
	}

	float	Get_Predicted_Percentage( int state )
	{
		switch( state )
		{
#define	TOTAL	(18.0f)
			case 0:	return 0.1f / TOTAL;
			case 1:	return 0.2f / TOTAL;
			case 2:	return 0.3f / TOTAL;
			case 3:	return 0.7f / TOTAL;
			case 4:	return 3.8f / TOTAL;
			case 5:	return 15.5f / TOTAL;
			case 6:	return 17.6f / TOTAL;
			default:	return 1.0f;
		}
	}


	void Render(bool update_network = false)
	{
		TimeManager::Update_Frame_Time();
		LoadTime += TimeManager::Get_Frame_Seconds();

	   WW3D::Begin_Render( true, true, Vector3(0.0f,0.0f,0.0f), update_network ? &cNetwork::Update : NULL);

		backdrop.Render();
		backdropText.Render();
		backdropText2.Render();

		static int last_count = -1;
		static int _last_percent_drawn = -1;
		if ( last_count != CombatManager::Get_Load_Progress() ) {
			last_count = CombatManager::Get_Load_Progress();
			Debug_Say(( " ****** Status Count %d at %f\n", last_count, LoadTime ));
			LoadPercentage = Get_Predicted_Percentage( last_count );
			LoadPercentageRate = LoadPercentage / LoadTime;
			LoadPercentageClamp = Get_Predicted_Percentage( last_count+1 );
		}

		LoadPercentage += LoadPercentageRate * TimeManager::Get_Frame_Seconds();
		LoadPercentage = WWMath::Clamp( LoadPercentage, 0, LoadPercentageClamp );
		LoadPercentageDrawn += ( LoadPercentage - LoadPercentageDrawn ) * 0.1f;
		backdrop.Set_Animation_Percentage( LoadPercentageDrawn );
		if (ConsoleBox.Is_Exclusive() && _last_percent_drawn != LoadPercentageDrawn) {
			_last_percent_drawn = LoadPercentageDrawn;
			ConsoleBox.Print("Load %d%% complete\r", (int)(LoadPercentageDrawn * 100.0f));
		}

#if 0
		StringClass txt=SaveLoadStatus::Get_Status_Text(0);
		txt=SaveLoadStatus::Get_Status_Text(1);
#endif

		WW3D::End_Render();
	}
};

#define LOADTIME_NETWORK_UPDATE if (!IS_SOLOPLAY) cNetwork::Update()


/*
**
*/
void CombatGameModeClass::Load_Level( void )
{
	WWLOG_PREPARE_TIME_AND_MEMORY("CombatGameModeClass::Load_Level");
	WWMEMLOG(MEM_GAMEDATA);
	Debug_Say(("CombatGameModeClass::Load_Level\n"));

	ConsoleBox.Print("Loading level %s\n", The_Game()->Get_Map_Name());

	CombatManager::Set_Load_Progress(0);
	LoadingScreenClass loading_screen;	// Try moving this to very start of loading
	loading_screen.Render(true);

	// Hack load reg for default first person.  Is dont again later.
	Load_Registry_Keys();
	WWLOG_INTERMEDIATE("Load_REgistry_Keys");

	// Flush out current level
	INIT_STATUS("Release current level");
	LevelManager::Release_Level();
	WWLOG_INTERMEDIATE("Releasing old level");

	if (!ConsoleBox.Is_Exclusive()) {
		WW3D::_Invalidate_Textures();
		WWLOG_INTERMEDIATE("WW3D::_Invalidate_Textures()");
		AssetStatusClass::Peek_Instance()->Enable_Load_On_Demand_Reporting(false);
	}

//	LoadingScreenClass loading_screen;
//	loading_screen.Render(true);

	PendingCampaignContinue	= false;
	g_is_loading = true;

	//
	// Stop the network layer from processing packets. This is needed in case a packet import or export accesses the datasafe
	// from the main thread while the loader thread is loading stuff into the datasafe. This won't shut off packet acks so no-one
	// will be disconnected as a result of this.
	//
	if (cNetwork::PServerConnection) {
		cNetwork::PServerConnection->Allow_Packet_Processing(false);
	}

	if (!IS_SOLOPLAY && cNetwork::PClientConnection != NULL) {
		cNetwork::PClientConnection->Allow_Extra_Timeout_For_Loading();
	}

	CombatManager::Pre_Load_Level(ConsoleBox.Is_Exclusive() ? false : true);

	INIT_STATUS("Apply system settings");
	SystemSettings::Apply_All();

	CombatManager::Set_Combat_Misc_Handler( &GameMiscHandler );

	// Flush out current level
//	INIT_STATUS("Release current level");
//	LevelManager::Release_Level();

	WWASSERT(PTheGameData != NULL);
	StringClass map_name(The_Game()->Get_Map_Name(),true);
	WWASSERT( !map_name.Is_Empty() );

	bool preload_assets = true;
#ifdef WWDEBUG
	preload_assets = cDevOptions::PreloadAssets.Get();
#endif

	DIAG_LOG(( "LOAD", "%s", map_name ));

	NetworkObjectMgrClass::Set_Is_Level_Loading (true);

	TextureLoader::Suspend_Texture_Load();

	CombatManager::Load_Level_Threaded( map_name, preload_assets );

	WWLOG_INTERMEDIATE("Level load preprocessing");

	while (!CombatManager::Is_Load_Level_Complete() ) {
		loading_screen.Render(true);
	   Windows_Message_Handler();
		LOADTIME_NETWORK_UPDATE;
		ThreadClass::Sleep_Ms(50);
	}
	WWLOG_INTERMEDIATE("Threaded level load");

	GenericDataSafeClass::Set_Preferred_Thread(GetCurrentThreadId());
	TextureLoader::Continue_Texture_Load();
	WWLOG_INTERMEDIATE("TextureLoader::Continue_Texture_Load()");

	LOADTIME_NETWORK_UPDATE;

	// Post load processing ----------------------------------------------------
	INIT_STATUS("Post_Load_Processing");
	loading_screen.Render(true);
   Windows_Message_Handler();
	SaveLoadSystemClass::Post_Load_Processing(IS_SOLOPLAY ? NULL : &cNetwork::Update);
	NetworkObjectMgrClass::Set_Is_Level_Loading (false);
	WWLOG_INTERMEDIATE("SaveLoadSystemClass::Post_Load_Processing(&cNetwork::Update)");

	INIT_STATUS("Post_Load_Level");
	loading_screen.Render(true);
   Windows_Message_Handler();
	LOADTIME_NETWORK_UPDATE;
	CombatManager::Post_Load_Level();
	WWLOG_INTERMEDIATE("CombatManager::Post_Load_Level()");

	INIT_STATUS("Post_Load_Id_Uniqueness_Check");
	loading_screen.Render(true);
   Windows_Message_Handler();
	LOADTIME_NETWORK_UPDATE;
	Post_Load_Id_Uniqueness_Check();
	WWLOG_INTERMEDIATE("Post_Load_Id_Uniqueness_Check()");

	INIT_STATUS("Post_Load_Dynamic_Object_Filtering");
	loading_screen.Render(true);
   Windows_Message_Handler();
	LOADTIME_NETWORK_UPDATE;
	Post_Load_Dynamic_Object_Filtering();
	WWLOG_INTERMEDIATE("Post_Load_Dynamic_Object_Filtering()");

	// Spawn point validation --------------------------------------------------
	INIT_STATUS("Spawn_Point_Validation");
	loading_screen.Render(true);
   Windows_Message_Handler();
	LOADTIME_NETWORK_UPDATE;
	Spawn_Point_Validation();
	WWLOG_INTERMEDIATE("Spawn_Point_Validation()");
	// -------------------------------------------------------------------------

   //DefinitionMgrClass::List_Available_Definitions();

	INIT_STATUS("Compute world size");
	Compute_World_Size();
	WWLOG_INTERMEDIATE("Compute_World_Size()");

	//
	// Set precision for other stuff
	// This stuff should be placed elsewhere... when there are a few
	// more of them.
	//
	ControlClass::Set_Precision();
	//SoldierGameObj::Set_Precision();
	HumanStateClass::Set_Precision();
	VehicleGameObj::Set_Precision();
	DoorPhysClass::Set_Precision();
	ElevatorPhysClass::Set_Precision();
	DefenseObjectClass::Set_Precision();
	BuildingGameObj::Set_Precision();

	// Override background color if this is a dedicated server
	if (cNetwork::I_Am_Only_Server()) {
		GameModeManager::Set_Background_Color( Vector3( 0.0f, 0.0f, 0.4f ) );
	}

	INIT_STATUS("Registry keys");
   Load_Registry_Keys();
   Save_Registry_Keys();
	WWLOG_INTERMEDIATE("SaveLoadRegistry");

	// Radar init --------------------------------------------------------------
	INIT_STATUS("Init radar class");		// Init the radar after the game is loaded (so we have the global settings
	loading_screen.Render(true);
   Windows_Message_Handler();
	LOADTIME_NETWORK_UPDATE;
	RadarManager::Init();
	WWASSERT(PTheGameData != NULL);
	RadarManager::Set_Radar_Mode(The_Game()->Get_Radar_Mode());
	WWLOG_INTERMEDIATE("Init Radar");
	// -------------------------------------------------------------------------

	const bool is_reloaded = true;
	The_Game()->Reset_Game(is_reloaded);
	WWLOG_INTERMEDIATE("The_Game()->Reset_Game(is_reloaded)");

	// Buildings init ----------------------------------------------------------
	// After the level loads, the buildings collect all of their components
	INIT_STATUS("Init_Buildings");
	loading_screen.Render(true);
   Windows_Message_Handler();
	LOADTIME_NETWORK_UPDATE;
	GameObjManager::Init_Buildings();
	WWLOG_INTERMEDIATE("GameObjManager::Init_Buildings()");
	// -------------------------------------------------------------------------

	// Update texture loader ---------------------------------------------------
	// This is needed here as we want to apply all textures that may have changed
	// during the game loading before the game actually starts.
	INIT_STATUS("Init_Textures");
	loading_screen.Render(true);
   Windows_Message_Handler();
	LOADTIME_NETWORK_UPDATE;
	TextureLoader::Update(IS_SOLOPLAY ? NULL : &cNetwork::Update);
	WWLOG_INTERMEDIATE("TextureLoader::Update(&cNetwork::Update)");
	// -------------------------------------------------------------------------

	//	Notify the game of "game start"
	WWASSERT(PTheGameData != NULL);
	The_Game()->On_Game_Begin();
	WWLOG_INTERMEDIATE("The_Game()->On_Game_Begin()");

	ForceGodPending = ForceGod != 0;

/*	if ( GameModeManager::Find( "Overlay3D" ) ) {
	   ((Overlay3DGameModeClass*)GameModeManager::Find( "Overlay3D" ))->Start_Intro();
	}
*/

	LOADTIME_NETWORK_UPDATE;
	AssetStatusClass::Peek_Instance()->Enable_Load_On_Demand_Reporting(true);
	g_is_loading = false;

	ConsoleBox.Print("Load %d%% complete\n", 100);
	ConsoleBox.Print("Level loaded OK\n");

	GameSpyQnR.Init();

	//
	// Re-enable packet processing.
	//
	if (cNetwork::PServerConnection) {
		cNetwork::PServerConnection->Allow_Packet_Processing(true);
	}

	if (!ConsoleBox.Is_Exclusive()) {
		unsigned ffheap=FastAllocatorGeneral::Get_Allocator()->Get_Total_Heap_Size();
		unsigned ffuse=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocated_Size();
		unsigned actualuse=FastAllocatorGeneral::Get_Allocator()->Get_Total_Actual_Memory_Usage();
		unsigned count=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocation_Count();
		StringClass working_string(0,true);
		working_string.Format(
			"\nMalloc count: %d\n"
			"Free count: %d\n"
			"FF Heap: %d.%3.3d.%3.3d (%d Mb)\n"
			"FF Use: %d.%3.3d.%3.3d (%d Mb)\n"
			"Actual Use: %d.%3.3d.%3.3d (%d Mb)\n"
			"FF Count: %d\n"
			,
			WW3D::Get_Last_Frame_Memory_Allocation_Count(),
			WW3D::Get_Last_Frame_Memory_Free_Count(),
			ffheap/(1000*1000),(ffheap/1000)%1000,ffheap%1000, ffheap/(1024*1024),
			ffuse/(1000*1000),(ffuse/10000)%1000,ffuse%1000, ffuse/(1024*1024),
			actualuse/(1000*1000),(actualuse/1000)%1000,actualuse%1000, actualuse/(1024*1024),
			count);
		WWDEBUG_SAY((working_string));
	}
}


void 	CombatGameModeClass::Core_Shutdown()
{
	Debug_Say(("CombatGameModeClass::Core_Shutdown\n"));

	//
	// Switch to the in-game sound page to ensure all those sounds get freed
	// as we're shutting down the level.
	//
	WWAudioClass::Get_Instance ()->Push_Active_Sound_Page (WWAudioClass::PAGE_PRIMARY);

	MultiHUDClass::Shutdown();
	//cPlayerManager::Shutdown();
	//cTeamManager::Shutdown();

	// Shutdown the sound library
	if (BackgroundMusic != NULL) {
		BackgroundMusic->Stop ();
		BackgroundMusic->Release_Ref ();
		BackgroundMusic = NULL;
	}

	if (IS_MISSION) {
		//
		//	Keep the GOD manager from offering a respawn when the star is gone
		//
		cGod::Exit();
	}

	CombatManager::Unload_Level();

	LevelManager::Release_Level();

 	RadarManager::Shutdown();

	//
	//	Restore the current sound page
	//
	WWAudioClass::Get_Instance ()->Pop_Active_Sound_Page ();
}

//-----------------------------------------------------------------------------
void CombatGameModeClass::Post_Load_Id_Uniqueness_Check(void)
{
	bool list_ids = false;

	//
	// Make sure no 2 phys objects have the same id.
	//

//	Debug_Say(("Testing id uniqueness for physical objects:\n"));

	int phys_obj_count = 0;
	SLNode<BaseGameObj> * objnode;
	for (objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
		WWASSERT(objnode->Data() != NULL);
      PhysicalGameObj * p_phys_obj = objnode->Data()->As_PhysicalGameObj();
		if (p_phys_obj != NULL) {

			phys_obj_count++;

			if (list_ids) {
				Debug_Say(("  Object %d, model %s\n",
					p_phys_obj->Get_ID(),
					p_phys_obj->Peek_Model()->Get_Name()));
			}

			SLNode<BaseGameObj> * objnode2;
			for (objnode2 = GameObjManager::Get_Game_Obj_List()->Head(); objnode2; objnode2 = objnode2->Next()) {
				WWASSERT(objnode2->Data() != NULL);
				PhysicalGameObj * p_phys_obj_2 = objnode2->Data()->As_PhysicalGameObj();
				if (p_phys_obj_2 != NULL &&
					p_phys_obj_2->Get_ID() == p_phys_obj->Get_ID()) {

					//WWASSERT(p_phys_obj_2 == p_phys_obj);
					if (p_phys_obj_2 != p_phys_obj) {
						Debug_Say(("Level file error: Two phys objects found with id %d.\n",
							p_phys_obj_2->Get_ID()));
						DIE;
					}
				}
			}
		}
	}

//	Debug_Say(("  %d unique physical objects were loaded.\n", phys_obj_count));



	//
	// Make sure no 2 static objects have the same id.
	//
//	Debug_Say(("Testing id uniqueness for static objects:\n"));

	WWASSERT(COMBAT_SCENE != NULL);
	int static_obj_count = 0;
	RefPhysListIterator iter_1 = COMBAT_SCENE->Get_Static_Object_Iterator();
	for (iter_1.First(); !iter_1.Is_Done(); iter_1.Next()) {

		//StaticPhysClass * p_obj_1 = (StaticPhysClass *) iter_1.Peek_Obj();
		PhysClass * p_obj_1 = iter_1.Peek_Obj();
		WWASSERT(p_obj_1 != NULL);
		int object_id_1 = p_obj_1->Get_ID();

		static_obj_count++;

		if (list_ids) {
			//Debug_Say(("  Object %d\n", object_id_1));
			Debug_Say(("  Object %d, model %s\n",
				p_obj_1->Get_ID(),
				p_obj_1->Peek_Model()->Get_Name()));
		}

		RefPhysListIterator iter_2 = COMBAT_SCENE->Get_Static_Object_Iterator();
		for (iter_2.First(); !iter_2.Is_Done(); iter_2.Next()) {
			//StaticPhysClass * p_obj_2 = (StaticPhysClass *) iter_2.Peek_Obj();
			PhysClass * p_obj_2 = iter_2.Peek_Obj();
			WWASSERT(p_obj_2 != NULL);
			int object_id_2 = p_obj_2->Get_ID();

			//
			//
			//     DO     NOT      DISABLE     THIS
			//
			//
			if (object_id_1 == object_id_2 && p_obj_1 != p_obj_2) {
				Debug_Say(("Fatal level file error: Two static objects found with id %d.\n",
					object_id_1));
				DIE;
			}
		}
	}

//	Debug_Say(("  %d unique static objects were loaded.\n", static_obj_count));
}

//-----------------------------------------------------------------------------
void CombatGameModeClass::Post_Load_Dynamic_Object_Filtering(void)
{
	//
	// Look through the soldiers and remove any that aren't
	// appropriate for this particular game type.
	//
	//
	// Any commando's that are tagged as having human players must be removed.
	// The player id field is not valid...
	// The exception is the commando belonging to the first player.
	//
	// Remove any soldiers with invalid player types for this game.
	//
	// Remove any grunts unless this is a mission.
	//

	WWASSERT(PTheGameData != NULL);

   SLNode<SmartGameObj> * objnode;
   for (objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
		SmartGameObj * p_smart_obj = objnode->Data();
      WWASSERT(p_smart_obj != NULL);

		if (p_smart_obj->As_SoldierGameObj() != NULL) {

			if (p_smart_obj->Has_Player() && (p_smart_obj->Get_Control_Owner() > 1 ||
				(!cNetwork::I_Am_Client()))) {
				Debug_Say(("* Removing loaded soldier belonging to another player.\n"));
	         p_smart_obj->Set_Delete_Pending();
			}

			if (!p_smart_obj->Is_Delete_Pending() &&
				!The_Game()->Is_Valid_Player_Type(p_smart_obj->Get_Player_Type())) {
				Debug_Say(("* Removing loaded soldier due to invalid playertype for this game.\n"));
	         p_smart_obj->Set_Delete_Pending();
			}

			if (!p_smart_obj->Is_Delete_Pending() && !p_smart_obj->Has_Player() &&
				!IS_SOLOPLAY) {
				Debug_Say(("* Removing loaded soldier grunt in non-mission game (id = %d).\n",
					p_smart_obj->Get_ID()));
	         p_smart_obj->Set_Delete_Pending();
			}
		}
   }
}

//-----------------------------------------------------------------------------
void CombatGameModeClass::Compute_World_Size(void)
{
	//
	// Tell networking system about the size of the world, to be used
	// in filtering.
	//
	Vector3 min;
	Vector3 max;
	WWASSERT(PhysicsSceneClass::Get_Instance() != NULL);
	PhysicsSceneClass::Get_Instance()->Get_Level_Extents(min, max);
	//Debug_Say(("World extends from (%5.2f, %5.2f, %5.2f) to (%5.2f, %5.2f, %5.2f)\n",
	//	min.X, min.Y, min.Z, max.X, max.Y, max.Z));

	Vector3 longest_distance = max - min;
	//Debug_Say(("Longest distance is %5.2fm\n", longest_distance.Length()));
	WWASSERT(PTheGameData != NULL);
	The_Game()->Set_Maximum_World_Distance(longest_distance.Length());

	double margin = 1; // comfort zone

	//
	// Also, set encoding precision for positional information
	//
	cEncoderList::Set_Precision(BITPACK_WORLD_POSITION_X,
		min.X - margin, max.X + margin, 0.2);
	cEncoderList::Set_Precision(BITPACK_WORLD_POSITION_Y,
		min.Y - margin, max.Y + margin, 0.2);
	cEncoderList::Set_Precision(BITPACK_WORLD_POSITION_Z,
		min.Z - margin, max.Z + margin, 0.2);
	cEncoderList::Set_Precision(BITPACK_BUILDING_RADIUS,
		0.0, 50.0, 0.1);
}

//-----------------------------------------------------------------------------
#define ADD_CASE(exp) case exp: return #exp; break;
static LPCSTR Playertype_To_String(int player_type)
{
	switch (player_type) {
		ADD_CASE(PLAYERTYPE_MUTANT);
		ADD_CASE(PLAYERTYPE_NEUTRAL);
		ADD_CASE(PLAYERTYPE_RENEGADE);
		ADD_CASE(PLAYERTYPE_NOD);
		ADD_CASE(PLAYERTYPE_GDI);

		default:
			DIE;
			return "ERROR"; // to avoid compiler warning
	}
}

//-----------------------------------------------------------------------------
void CombatGameModeClass::Spawn_Point_Validation(void)
{
	if (cNetwork::I_Am_Server()) {

		bool is_spawners_valid = true;

		for (int spawn_type = PLAYERTYPE_RENEGADE; spawn_type <= PLAYERTYPE_LAST; spawn_type++) {

			if ( !SpawnManager::Spawner_Exists( spawn_type ) ) {
				Debug_Say(("  Error: No spawn point(s) of type %s found.\n",
					Playertype_To_String(spawn_type)));
				is_spawners_valid = false;
			} else {
				Debug_Say(("  Spawn point(s) of type %s found.\n",
					Playertype_To_String(spawn_type)));
			}
		}

		if (!IS_SOLOPLAY) {
			WWASSERT(is_spawners_valid);
		}
	}
}

//-----------------------------------------------------------------------------

void CombatGameModeClass::Core_Restart(void)
{
	Debug_Say(("CombatGameModeClass::Core_Restart\n"));

	Core_Shutdown();

	GameModeManager::Safely_Deactivate();

	INIT_STATUS("Reloading...");

	if ( IS_MISSION ) {
#ifndef MULTIPLAYERDEMO
		StringClass new_name;
		WWASSERT(PTheGameData != NULL);

		new_name = CombatManager::Get_Last_LSD_Name();

			// convert .LSD to .MIX
			const size_t name_length = new_name.Get_Length();
			if ( name_length > 4 &&
				::stricmp( &new_name[static_cast<int>(name_length - 4)], ".LSD" ) == 0 ) {
				new_name.Erase( static_cast<int>(name_length - 4), 4 );
			new_name += ".MIX";
		}

		The_Game()->Set_Map_Name( new_name );
#endif // !MULTIPLAYERDEMO
	} else {

		//
		// Reset the timer system if we are a dedicated server and there is no-one in the game.
		// ST - 1/30/2002 12:44PM
		//
		if (The_Game() && The_Game()->IsDedicated.Is_True()) {
			if (cNetwork::PServerConnection && cNetwork::PServerConnection->Get_Num_RHosts() == 0) {
				GameModeClass *game_mode = GameModeManager::Find("LAN");

				//
				// Don't do this if the NAT thread is busy since it uses timers extensively.
				//
				if ((game_mode && game_mode->Is_Active()) || WOLNATInterface.Is_NAT_Thread_Busy() == false) {
					//
					// Danger! Danger!
					// This could cause code that uses time deltas to fail. Hopefully there isn't any code doing that right now.
					//
					SystemTime.Reset();
					TimeManager::Reset();
					GenericDataSafeClass::Reset_Timers();

					GameModeClass* game_mode = GameModeManager::Find("WOL");

					if (game_mode && game_mode->Is_Active()) {
						WolGameModeClass* wol_game = reinterpret_cast<WolGameModeClass*>(game_mode);
						wol_game->System_Timer_Reset();
					}
				}
			}
		}
	}

	//
	// Reset packet optimizer bandwidth stats.
	//
	PacketManager.Reset_Stats();

	Load_Level();

#ifdef WW3D_COMPILE_WITH_DX8__
	DX8MeshRendererContainerClass::Invalidate_All();
#endif
}

//-----------------------------------------------------------------------------

/*
**
*/
void CombatGameModeClass::Load_Registry_Keys(void)
{
	RegistryClass * registry = new RegistryClass( APPLICATION_SUB_KEY_NAME_OPTIONS );
	WWASSERT( registry );
	if ( registry->Is_Valid() ) {
		IsHudShown = registry->Get_Int( "IsHudShown", IsHudShown );
		ForceGod = registry->Get_Int( "ForceGod", ForceGod );

		//TSS
		DefaultToFirstPerson = registry->Get_Int( "DefaultToFirstPerson", DefaultToFirstPerson );
		CombatManager::Set_First_Person_Default(DefaultToFirstPerson != 0);
	}
	delete registry;
}

void CombatGameModeClass::Save_Registry_Keys(void)
{
	RegistryClass * registry = new RegistryClass( APPLICATION_SUB_KEY_NAME_OPTIONS );
	WWASSERT( registry );
	if ( registry->Is_Valid() ) {
		registry->Set_Int( "IsHudShown",	IsHudShown );
		registry->Set_Int( "ForceGod",	ForceGod );

		//TSS
		DefaultToFirstPerson = CombatManager::Get_First_Person_Default();
		registry->Set_Int( "DefaultToFirstPerson",	DefaultToFirstPerson );
	}
	delete registry;
}

/*
** called each time through the main loop when active
*/
void 	CombatGameModeClass::Think()
{
	WWPROFILE( "Combat Think" );

	if ( !Is_Active() ) {
		return;
	}

	Combat_Keyboard();

	//
	// Test again to see if we are active, because quick exit may inactivate us.
	//
	if ( !Is_Active() ) {
		return;
	}

	CombatManager::Generate_Control();

	//
	// Network Update needs to be between input and think
	//

	float time_1;
	{
	WWMeasureItClass net_upd_time_s(&time_1);
	cNetwork::Update();
	}

	float time_2;
	{
	WWMeasureItClass combat_think_time_s(&time_2);
	CombatManager::Think();
	}

	if (cNetwork::I_Am_Server())
	{
		WWPROFILE( "cSbboManager stuff" );
		cSbboManager::Increment_Accum_Time_S_Net_Update(time_1);
		cSbboManager::Increment_Accum_Time_S_Combat_Think(time_2);
		cSbboManager::Think();
	}

	DEMO_SECURITY_CHECK;

	if ( COMBAT_STAR ) {
		WWPROFILE( "Stuff 1" );
		Vector3 pos;
		COMBAT_STAR->Get_Position( &pos );
		float facing = COMBAT_STAR->Get_Facing();
		Get_Console()->Set_Player_Position( pos, facing );

		if ( ForceGodPending ) {
			ForceGodPending = false;
			ConsoleFunctionManager::Parse_Input( "God" );
		}

	}

	MultiHUDClass::Think();
	cPlayerManager::Think();
	cTeamManager::Think();

	if ( PendingCampaignContinue ) {
		WWPROFILE( "Stuff 2" );
		PendingCampaignContinue	= false;
		Debug_Say(( "Handle Pending Campaign Continue\n" ));
		CampaignManager::Continue();
	}


#ifdef MULTIPLAYERDEMO
	//
	// Security check. Randomly every few minutes, check to see if a special
	// MP demo object (id 100277) exists. If it doesn't, assume someone has hacked in
	// a different map. Bail.
	//
	if ((::rand() % 10131 == 939) && (NetworkObjectMgrClass::Find_Object(100277) == NULL)) {
		WWDEBUG_SAY(("MP DEMO OBJECT NOT FOUND... BAILING.\n"));
		Suspend();
		GameInitMgrClass::End_Game();
		extern void Stop_Main_Loop (int);
		Stop_Main_Loop(EXIT_SUCCESS);
	}
#endif // MULTIPLAYERDEMO


	if (g_b_core_restart)	{
		WWPROFILE( "g_b_core_restart" );

		g_b_core_restart = false;

		//Core_Restart();

		cPlayer * p_me = cNetwork::Get_My_Player_Object();

		if (p_me != NULL) {// && cNetwork::I_Am_Only_Client()) {

			p_me->Set_Is_In_Game(false);
			cLoadingEvent * p_loading_1 = new cLoadingEvent;
			p_loading_1->Init(true);

			Core_Restart();

			p_me->Set_Is_In_Game(true);
			cLoadingEvent * p_loading_2 = new cLoadingEvent;
			p_loading_2->Init(false);

		} else {
			Core_Restart();
		}

		if (!IS_MISSION) {
			MultiHUDClass::Init();
		}

		cNetwork::Enable_Waiting_Players();

/*
#if(0)
		WWDEBUG_SAY(("****** CombatGameModeClass::Think On_Game_Begin()\n"));
		WWASSERT(The_Game() != NULL);

		//		The_Game()->On_Game_End();
		The_Game()->Reset_Game(false);
		The_Game()->On_Game_Begin();//TSS091201
#endif
*/
	}

	// Autosave, after one run throught main loop
	if ( CombatManager::Is_Autosave_Requested() ) {
		WWPROFILE( "Autosaving" );
		Debug_Say(( "Autosaving\n" ));
		int time=TIMEGETTIME();
		CombatManager::Request_Autosave( false );
		SaveGameManager::Set_Description( TRANSLATE( IDS_SAVE_AUTOSAVE ) );
		SaveGameManager::Save_Game( "save\\autosave.sav", &_CommandoSaveLoad, NULL );
		time=TIMEGETTIME()-time;
		Debug_Say(( "Autosaving Complete, took %d.%2.2d seconds\n",time/1000,(time/10)%100 ));
	}

	//TSS090401
	if (g_client_quit)
	{
		WWPROFILE( "g_client_quit" );

		//
		// This becomes true when the connection to the server breaks
		//
		g_client_quit = false;

		Suspend();
		GameInitMgrClass::End_Game();

		//GAMESPY
		if (cGameSpyAdmin::Get_Is_Launched_From_Gamespy())
		{
#ifdef MULTIPLAYERDEMO
			DialogMgrClass::Flush_Dialogs ();
			START_DIALOG (SplashOutroMenuDialogClass);
#else
			extern void Stop_Main_Loop (int);
			Stop_Main_Loop(EXIT_SUCCESS);
#endif // MULTIPLAYERDEMO
		}
		else
		{
			GameInitMgrClass::Display_End_Game_Menu();
		}
	}
}

void 	CombatGameModeClass::Render()
{
	if ( !Is_Active() ) {
		return;
	}

	if (cNetwork::I_Am_Client()) {

		/*
		** In multi-play, only render the combatmanager when we have a valid camera and the menu is not active
		*/
		bool menu_active = false;
		GameModeClass * menu_mode = GameModeManager::Find( "Menu" );		// Activate the main menu
		if ((menu_mode != NULL) && (menu_mode->Get_State() == GAME_MODE_ACTIVE)) {
			menu_active = true;
		}

		if ( COMBAT_CAMERA && COMBAT_CAMERA->Is_Valid() && (menu_active == false)) {
			CombatManager::Render();
		}
   }

	MultiHUDClass::Render();
	cBandwidthGraph::Render();
	cPlayerManager::Render();
	cTeamManager::Render();
	WWASSERT(PTheGameData != NULL);
	The_Game()->Render();
	RadioCommandDisplayClass::Render ();
}

//-----------------------------------------------------------------------------
void CombatGameModeClass::Toggle_Multi_Hud(void)
{
	MultiHUDClass::Toggle();
}

// --------------------------------------------------------------

void	CombatGameMiscHandlerClass::Mission_Complete( bool success )
{
	if (IS_MISSION) {
		if ( success ) {
			PendingCampaignContinue	= true;
		} else {
			cGod::Mission_Failed();
		}
#if 0
		GameModeManager::Find("Combat")->Suspend();
//		GameModeManager::Find("Combat")->Deactivate();
//		GameModeManager::Find("ScoreScreen")->Activate();
//		((ScoreScreenGameModeClass*)GameModeManager::Find("ScoreScreen"))->Set_Success( success );

		if ( GameModeManager::Find( "Overlay3D" ) ) {
		   ((Overlay3DGameModeClass*)GameModeManager::Find( "Overlay3D" ))->Start_End_Screen();
		}
#endif
	}
}

void	CombatGameMiscHandlerClass::Star_Killed( void )
{
	cGod::Star_Killed();
}

void	CombatGameModeClass::Resume(void)
{
	//
	//	Show the message window (if necessary)
	//
	if (	CombatManager::Get_Message_Window () != NULL &&
			CombatManager::Get_Message_Window ()->Has_Data ())
	{
		CombatManager::Get_Message_Window ()->Force_Display (true);
	}

	GameMajorModeClass::Resume();
	return ;
}

void	CombatGameModeClass::Suspend(void)
{
	//
	//	Hide the message window
	//
	if (CombatManager::Get_Message_Window () != NULL) {
		CombatManager::Get_Message_Window ()->Force_Display (false);
	}

	//
	//	Hide the objective viewer
	//
	ObjectiveManager::Display_Viewer (false);

	GameMajorModeClass::Suspend();
	return ;
}


/*
**
*/
void	CombatGameModeClass::Quick_Save( void )
{
	bool	saveA = true;

	RegistryClass * registry = new RegistryClass( APPLICATION_SUB_KEY_NAME_OPTIONS );
	WWASSERT( registry );
	if ( registry->Is_Valid() ) {
		saveA = registry->Get_Bool( "QuicksaveA", saveA );
	}

#define	SAVEGAME_NAME_A	"save\\quicksaveA.sav"
#define	SAVEGAME_NAME_B	"save\\quicksaveB.sav"

	// Check for a missing file
   if ( !cMiscUtil::File_Exists(SAVEGAME_NAME_B) ) {
		saveA = false;
   }
   if ( !cMiscUtil::File_Exists(SAVEGAME_NAME_A) ) {
		saveA = true;
   }

	if ( saveA ) {
		SaveGameManager::Set_Description( TRANSLATE(IDS_SAVE_QUICKSAVE_A) );
		SaveGameManager::Save_Game( SAVEGAME_NAME_A, &_CommandoSaveLoad, NULL );
		Debug_Say(( "Quicksaved A\n" ));
	} else {
		SaveGameManager::Set_Description( TRANSLATE(IDS_SAVE_QUICKSAVE_B) );
		SaveGameManager::Save_Game( SAVEGAME_NAME_B, &_CommandoSaveLoad, NULL );
		Debug_Say(( "Quicksaved B\n" ));
	}
	saveA = !saveA;

	if ( registry->Is_Valid() ) {
		registry->Set_Bool( "QuicksaveA",	saveA );
	}
	delete registry;

	// Display "Quick Saved"
	HUDInfo::Set_HUD_Help_Text( TRANSLATE( IDS_M00DSGN_DSGN1017I1DSGN_TXT ), Vector3( 0,1,0 ) );

}



























	/**/
	/*
	if (::rand() % 10131 == 939)) {
		bool bail = true;

		ScriptableGameObj * p_obj = GameObjManager::Find_ScriptableGameObj(100277);
		if (p_obj != NULL && p_obj->Get_Definition().Get_Name()) {
			bail = false;
		}

		if (bail) {
			Suspend();
			GameInitMgrClass::End_Game();
			extern void Stop_Main_Loop (int);
			Stop_Main_Loop(EXIT_SUCCESS);
		}
	}
	*/
