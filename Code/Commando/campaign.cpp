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
 *                     $Archive:: /Commando/Code/Commando/campaign.cpp          $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 1/19/02 12:30p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 33                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "campaign.h"
#include "debug.h"
#include "gamemode.h"
#include "gamedata.h"
#include "singlepl.h"
#include "gdsingleplayer.h"
#include "cnetwork.h"
#include "playertype.h"
#include "gameinitmgr.h"
#include "scorescreen.h"
#include "assets.h"
#include "movie.h"
#include "consolefunction.h"
#include "renegadedialogmgr.h"
#include "registry.h"
#include "_globals.h"
#include "crandom.h"
#include "god.h"
#include "dlgloadspgame.h"
#include "ccamera.h"
#include <cstdio>

/*
**
*/
int	CampaignManager::State = 0;
int	CampaignManager::BackdropIndex = 0;

#define	CAMPAIGN_INI_FILENAME	"campaign.ini"
#define	SECTION_CAMPAIGN			"Campaign"
#define	NOT_IN_CAMPAIGN_STATE	-10
#define	REPLAY_LEVEL			-11
#define	REPLAY_SCORE			-12

DynamicVectorClass<StringClass>	CampaignFlowDescriptions;

struct BackdropDescriptionStruct {
	int State;
	DynamicVectorClass<StringClass>	Lines;

	bool operator == (BackdropDescriptionStruct const & ) const	{ return false; }
	bool operator != (BackdropDescriptionStruct const & ) const	{ return true; }
};

DynamicVectorClass<BackdropDescriptionStruct>	BackdropDescriptions;

/*
**
*/
void	CampaignManager::Init( void )
{
	State = NOT_IN_CAMPAIGN_STATE;
	BackdropIndex = 0;

	// Load CAMPAIGN.INI to get campain flow
	INIClass	* campaignINI = Get_INI( CAMPAIGN_INI_FILENAME );
	if (campaignINI != NULL) {
		WWASSERT( campaignINI && campaignINI->Section_Count() > 0 );
		int count =  campaignINI->Entry_Count( SECTION_CAMPAIGN );
		for ( int entry = 0; entry < count; entry++ )	{
			StringClass	description(0,true);
			campaignINI->Get_String(description, SECTION_CAMPAIGN, campaignINI->Get_Entry( SECTION_CAMPAIGN, entry) );
			CampaignFlowDescriptions.Add( description );
		}

		// Load Backdrop Descriptions
		// Load the first 100, because 90-95 are multiplay.. :)
		for ( int state = 0; state < 100; state++ ) {
			StringClass section_name;
			section_name.Format( "Backdrop%d", state );
			int count =  campaignINI->Entry_Count( section_name );
			if ( count != 0 ) {
				int index = BackdropDescriptions.Count();
				BackdropDescriptions.Uninitialized_Add();
				BackdropDescriptions[index].State = state;
				for ( int entry = 0; entry < count; entry++ )	{
					StringClass	description(0,true);
					campaignINI->Get_String(description, section_name, campaignINI->Get_Entry( section_name, entry) );
					BackdropDescriptions[index].Lines.Add( description );
				}
			}

		}

		Release_INI( campaignINI );
	} else {
		Debug_Say(("CampaignManager::Init - Unable to load %s\n", CAMPAIGN_INI_FILENAME));
	}

}


/*
**
*/
void	CampaignManager::Shutdown( void )
{
	CampaignFlowDescriptions.Clear();
}



/*
**
*/
//-----------------------------------------------------------------------------
enum	{
	CHUNKID_VARIABLES = 906011356,

	MICROCHUNK_STATE = 1,
	MICROCHUNK_BACKDROP_INDEX,
};

//-----------------------------------------------------------------------------
bool CampaignManager::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK(csave, MICROCHUNK_STATE, State);
		WRITE_MICRO_CHUNK(csave, MICROCHUNK_BACKDROP_INDEX, BackdropIndex);
	csave.End_Chunk();
	return true;
}

//-----------------------------------------------------------------------------
bool CampaignManager::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload, MICROCHUNK_STATE, State);
						READ_MICRO_CHUNK(cload, MICROCHUNK_BACKDROP_INDEX, BackdropIndex);
						default:
							Debug_Say(( "Unrecognized Campaign Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized campaign chunkID\n" ));
				break;
		}
		cload.Close_Chunk();
	}

	return true;
}




/*
**
*/
void	CampaignManager::Start_Campaign( int difficulty )
{
	Debug_Say(( "CampaignManager::Start_Campaign( %d )\n", difficulty ));

	State = -1;
	BackdropIndex = 0;

	// Why was this commented out???
	CombatManager::Set_Difficulty_Level( difficulty );

	StringClass diff_string;
	diff_string.Format( "difficulty %d", difficulty );
	ConsoleFunctionManager::Parse_Input( diff_string );

	cGod::Reset_Inventory();

	Continue();
}

/*
**
*/
void	CampaignManager::Continue( bool /* success */ )
{
	BackdropIndex = 0;

	if ( State == REPLAY_LEVEL ) {
		State = REPLAY_SCORE;

		// Activeate the Score screen before the combat deactivates, so we can get the stats
		ScoreScreenGameModeClass * ss = (ScoreScreenGameModeClass *)GameModeManager::Find ("ScoreScreen");
		if ( ss != NULL ) {
		 	ss->Save_Stats();
		}

		GameModeManager::Find ("Movie")->Deactivate();
		GameModeManager::Find ("Combat")->Suspend();
		GameInitMgrClass::End_Game();
		GameModeManager::Find ("Menu")->Deactivate();

		if ( ss != NULL ) {
			ss->Activate();
		}
		return;
	}

	if ( State == NOT_IN_CAMPAIGN_STATE || State == REPLAY_SCORE || ( State >= CampaignFlowDescriptions.Count() - 1 ) ) {
		State = NOT_IN_CAMPAIGN_STATE;
		GameModeManager::Find ("Movie")->Deactivate();
		GameModeManager::Find ("ScoreScreen")->Deactivate();		// BMG???
		GameModeManager::Find ("Combat")->Suspend();
		GameInitMgrClass::End_Game();
		GameInitMgrClass::Display_End_Game_Menu();
		return;
	}

	State = State+1;

	Debug_Say(( "CampaignManager::Continue %d\n", State ));

	const char * state_description = CampaignFlowDescriptions[State];

#define	StringMatch(a,b)	(!::strncmp( a,b,strlen(b) ))

	if ( StringMatch( state_description, "Message " ) ) {

		state_description += ::strlen( "Message " );

		GameModeManager::Find ("Movie")->Deactivate();
		GameModeManager::Find ("Combat")->Suspend();
		GameInitMgrClass::End_Game();

		GameModeManager::Find ("Menu")->Deactivate();
		GameModeManager::Find ("ScoreScreen")->Activate();

	} else if ( StringMatch( state_description, "Score" ) ) {

		state_description += ::strlen( "Score" );

		// Activeate the Score screen before the combat deactivates, so we can get the stats
		ScoreScreenGameModeClass * ss = (ScoreScreenGameModeClass *)GameModeManager::Find ("ScoreScreen");
		if ( ss != NULL ) {
		 	ss->Save_Stats();
		}

		GameModeManager::Find ("Movie")->Deactivate();
		GameModeManager::Find ("Combat")->Suspend();
		GameInitMgrClass::End_Game();
		GameModeManager::Find ("Menu")->Deactivate();

		if ( ss != NULL ) {
			ss->Activate();
		}

	} else if ( StringMatch( state_description, "Level " ) ) {


		GameModeManager::Find ("Combat")->Suspend();
		GameModeManager::Find ("Movie")->Deactivate();
	    GameModeManager::Find ("ScoreScreen")->Deactivate ();

		GameInitMgrClass::End_Game();

		state_description += ::strlen( "Level " );

		int mission = cGameData::Get_Mission_Number_From_Map_Name( state_description );
		Select_Backdrop_Number( mission );
		GameInitMgrClass::Start_Game ( state_description, PLAYERTYPE_RENEGADE, 0 );

		// Hack to not autosave for Mission 0 (M13)
		if ( ::strnicmp( state_description, "M13", 3 ) != 0 ) {
			CombatManager::Request_Autosave();
		}

	} else if ( StringMatch( state_description, "Movie " ) ) {

		if (COMBAT_CAMERA != NULL) {
			COMBAT_CAMERA->Set_Host_Model (NULL);
		}

		GameModeManager::Find ("Combat")->Suspend();
		GameInitMgrClass::End_Game();
		GameModeManager::Find ("Menu")->Deactivate();
		GameModeManager::Find ("ScoreScreen")->Deactivate ();

		//
		//	Parse the parameters
		//
		const size_t len = ::strlen( state_description );
		StringClass foo( len + 1, true );
		StringClass filename( len + 1, true );
		StringClass description( len + 1, true );
		::sscanf (state_description, "%s %s %s", foo.Peek_Buffer (), filename.Peek_Buffer (), description.Peek_Buffer ());

		MovieGameModeClass * mode = (MovieGameModeClass *)GameModeManager::Find ("Movie");
		if ( mode ) {
			mode->Activate();
			mode->Start_Movie( filename );

			//
			//	Add this movie name to the registry (that way the user
			// can watch it later)
			//	
			RegistryClass registry( APPLICATION_SUB_KEY_NAME_MOVIES );
			if ( registry.Is_Valid() ) {
				registry.Set_String( filename, description );
			}
		}

	} else {

		Debug_Say(( "Failed to Parse Campaign Description %s\n", state_description ));

		State = NOT_IN_CAMPAIGN_STATE;		
		RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
	}

}

void	CampaignManager::Reset()
{
	State = NOT_IN_CAMPAIGN_STATE;		
}

/*
**
*/
void	CampaignManager::Replay_Level( const char * mission_name, int difficulty )
{
	State = REPLAY_LEVEL;

	cGod::Reset_Inventory();
	CombatManager::Set_Difficulty_Level( difficulty );

	GameInitMgrClass::Start_Game( mission_name, PLAYERTYPE_RENEGADE, 0 );
}

/*
**
*/
int	CampaignManager::Get_Backdrop_Description_Count( void )
{
	if (BackdropDescriptions.Count() > 0) {
		return BackdropDescriptions[BackdropIndex].Lines.Count();
	}
	return 0;
}

const char * CampaignManager::Get_Backdrop_Description( int index )
{
	return BackdropDescriptions[BackdropIndex].Lines[index];
}

void	CampaignManager::Select_Backdrop_Number( int state_number )
{
	// Find Backdrop Index
	BackdropIndex = 0;
	for ( int i = 0; i < BackdropDescriptions.Count(); i++ ) {
		if ( BackdropDescriptions[i].State == state_number ) {
			BackdropIndex = i;
		}
	}

	if ( BackdropIndex == 0 ) {
		Debug_Say(( "Failed to find load menu for state %d\n", state_number ));
	}
}

void	CampaignManager::Select_Backdrop_Number_By_MP_Type( [[maybe_unused]] int type )
{
	//
	//	Setup Load Menu
	//
	/*
	#define	MULTIPLAY_LOAD_MENU_NUMBER_DEATHMATCH			91
	#define	MULTIPLAY_LOAD_MENU_NUMBER_TEAM_DEATHMATCH	92
	#define	MULTIPLAY_LOAD_MENU_NUMBER_CTF					93
	#define	MULTIPLAY_LOAD_MENU_NUMBER_CNC1					94
	#define	MULTIPLAY_LOAD_MENU_NUMBER_CNC2					95
	int load_menu_number = 0;
	if ( type == cGameData::GAME_TYPE_DEATHMATCH ) {
		load_menu_number = MULTIPLAY_LOAD_MENU_NUMBER_DEATHMATCH;	
	}
	if ( type == cGameData::GAME_TYPE_TEAM_DEATHMATCH ) {
		load_menu_number = MULTIPLAY_LOAD_MENU_NUMBER_TEAM_DEATHMATCH;	
	}
	if ( type == cGameData::GAME_TYPE_CNC ) {
		load_menu_number = MULTIPLAY_LOAD_MENU_NUMBER_CNC1;	
		if ( FreeRandom.Get_Int() & 1 ) {
			load_menu_number = MULTIPLAY_LOAD_MENU_NUMBER_CNC2;	
		}
	}
	Select_Backdrop_Number( load_menu_number );
	*/

	WWASSERT(type == cGameData::GAME_TYPE_CNC);

	#define	MULTIPLAY_LOAD_MENU_NUMBER_CNC1					94
	#define	MULTIPLAY_LOAD_MENU_NUMBER_CNC2					95

	int load_menu_number = 0;
	if (FreeRandom.Get_Int() & 1) {
		load_menu_number = MULTIPLAY_LOAD_MENU_NUMBER_CNC1;
	} else {
		load_menu_number = MULTIPLAY_LOAD_MENU_NUMBER_CNC2;
	}

//	Select_Backdrop_Number(load_menu_number);
	//forget the random screen, alwayds do 94 (BMG 11/24/01)
	Select_Backdrop_Number( MULTIPLAY_LOAD_MENU_NUMBER_CNC1 );

}





















	/*
	if ( type == cGameData::GAME_TYPE_CTF ) {
		load_menu_number = MULTIPLAY_LOAD_MENU_NUMBER_CTF;	
	}
	*/
