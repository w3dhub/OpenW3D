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

//
// Filename:     god.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  This class handles the creations of players and soldiers.
//

#include "god.h"

#include "cnetwork.h"
#include "playermanager.h"
#include "gameobjmanager.h"
#include "spawn.h"
#include "playertype.h"
#include "objlibrary.h"
#include "soldierobserver.h"
#include "gametype.h"
#include "dialogtests.h"
#include "combatgmode.h"
#include "gameinitmgr.h"
#include "scriptman.h"
#include "debug.h"
#include "renegadedialogmgr.h"
#include "cheatmgr.h"
#include "wwmemlog.h"
#include "dialogmgr.h"
#include "encyclopediamgr.h"
#include "wolgmode.h"
#include "specialbuilds.h"
#include "demosupport.h"

/*
**
*/
typedef enum {
	GOD_STATE_UNINITIALIZED,
	GOD_STATE_MULTIPLAYER,
	GOD_STATE_EXITING,

	GOD_STATE_SINGLE_INIT,
	GOD_STATE_SINGLE_RUNNING,
	GOD_STATE_SINGLE_DEAD,

} GodState;

int		cGod::State		= GOD_STATE_UNINITIALIZED;
InventoryClass	cGod::LevelStartInventory;

//-----------------------------------------------------------------------------
enum	{
	CHUNKID_VARIABLES = 112374,

	MICROCHUNK_STATE = 1,
};

//-----------------------------------------------------------------------------
bool cGod::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(CHUNKID_VARIABLES);
	WRITE_MICRO_CHUNK(csave, MICROCHUNK_STATE, State);
	csave.End_Chunk();
	return true;
}

//-----------------------------------------------------------------------------
bool cGod::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload, MICROCHUNK_STATE, State);
						default:
							Debug_Say(( "Unrecognized cGod Variable chunkID\n" ));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(( "Unrecognized cPlayer chunkID\n" ));
				break;
		}
		cload.Close_Chunk();
	}

	return true;
}


//-----------------------------------------------------------------------------
void cGod::Think(void)
{
	WWASSERT(cNetwork::I_Am_Server());

	//if (The_Game()->IsIntermission.Is_True()) {
	WWASSERT(PTheGameData != NULL);
	if (The_Game()->IsIntermission.Is_True() ||
		 cPlayerManager::Get_Player_Object_List()->Head() == NULL) {
		return;
	}

	if ( State == GOD_STATE_UNINITIALIZED ) {
		//XXX
		State = ( IS_MISSION ) ? GOD_STATE_SINGLE_INIT : GOD_STATE_MULTIPLAYER;
	}

	if ( State == GOD_STATE_SINGLE_INIT ) {

		WWASSERT( cPlayerManager::Get_Player_Object_List()->Head() != NULL );
		// Create a Commando for the Player
		SoldierGameObj * soldier = Create_Commando( cPlayerManager::Get_Player_Object_List()->Head()->Data() );

		const char * script_name = CombatManager::Get_Start_Script();
		ScriptClass* script = ScriptManager::Create_Script( script_name );
		if (script) {
			soldier->Add_Observer( script );
		}

		State = GOD_STATE_SINGLE_RUNNING;
	}

	// This code may need to get cleaned up
	if ( State == GOD_STATE_SINGLE_RUNNING ) {
		//If we just loaded, we may have a play and a solder, but they will not be linked.
		if (cPlayerManager::Get_Player_Object_List() != NULL &&
			 cPlayerManager::Get_Player_Object_List()->Head() != NULL ) {

			cPlayer * p_player = cPlayerManager::Get_Player_Object_List()->Head()->Data();
			if ( p_player != NULL ) {
		   		SmartGameObj * p_soldier = GameObjManager::Find_Soldier_Of_Client_ID(p_player->Get_Id());
				if ( p_soldier && p_player->Get_GameObj() == NULL ) {
					// Remap
					p_soldier->Set_Player_Data( p_player );
					Debug_Say(( "Fixing up player data after load\n" ));
				}
			}
		}
	}

	DEMO_SECURITY_CHECK;

   //
   // Take a look through the player list and create commando bodies
	// for anyone who merits one
   //
	if ( State == GOD_STATE_MULTIPLAYER ) {
		for (
			SLNode<cPlayer> * objnode = cPlayerManager::Get_Player_Object_List()->Head();
			objnode;
			objnode = objnode->Next()) {

			cPlayer * p_player = objnode->Data();
			WWASSERT(p_player != NULL);

			if (p_player->Get_Is_Active().Is_False()) {
				continue;
			}

			if (p_player->Get_Is_In_Game().Is_False()) {
				continue;
			}

			SmartGameObj * p_soldier = GameObjManager::Find_Soldier_Of_Client_ID(p_player->Get_Id());

			if (p_soldier == NULL) {
				//
				// A disembodied player... give him a body
				//
				Create_Commando(p_player);
			}
		}
	}
}

//-----------------------------------------------------------------------------
cPlayer * cGod::Create_Player(int client_id, const WideStringClass & name,
															int team_choice, unsigned int clanID, bool is_invulnerable)
{
	WWMEMLOG(MEM_NETWORK);
	WWASSERT(cNetwork::I_Am_Server());
	WWASSERT(PTheGameData != NULL);
	WWASSERT(cPlayerManager::Count() < The_Game()->Get_Max_Players());

   //
	// Assign a player type
	//
	cPlayer * p_player = cPlayerManager::Find_Player(name);

	if (p_player != NULL) {
		//
		// I think this can happen when a player crashes out and then rejoins
		// before the server breaks his connection...
		//
		cNetwork::Delete_Player_Objects(p_player->Get_Id());
	} else {
		p_player = cPlayerManager::Find_Inactive_Player(name);
	}

	bool is_new = false;

	if (p_player == NULL) {
      p_player = new cPlayer();
      WWASSERT(p_player != NULL);
		is_new = true;

		p_player->Set_Name(name);
		p_player->Set_Id(client_id);
		p_player->Set_WOL_ClanID(clanID);

		int player_type = The_Game()->Choose_Player_Type(p_player, team_choice, false);
		p_player->Set_Player_Type(player_type);
	} else {
		p_player->Set_Id(client_id);
		p_player->Set_Is_In_Game(true);
		p_player->Set_Is_Waiting_For_Intermission(false);
	}

	p_player->Reset_Join_Time();
	p_player->Invulnerable.Set(is_invulnerable);
	p_player->Set_Is_Active(true);

	//
	// Tell everyone about this guy
	//
	if (is_new) {
		p_player->Init();

		GameModeClass* gameMode = GameModeManager::Find("WOL");

		if (gameMode && gameMode->Is_Active()) {
			WolGameModeClass* wolGame = reinterpret_cast<WolGameModeClass*>(gameMode);
			wolGame->Init_WOL_Player(p_player);
		}
	}

	return p_player;
}

//-----------------------------------------------------------------------------
void cGod::Create_Ai_Player(void)
{
	WWASSERT(PTheGameData != NULL);
	WWASSERT(cPlayerManager::Count() < The_Game()->Get_Max_Players());
   WWASSERT(cNetwork::I_Am_Server());

	//
	// For id, count downwards from -2.
	//
	int client_id = -1;//SmartGameObj::MOBIUS_CONTROL_OWNER;
	WideStringClass name;
	do {
		client_id--;
		WWASSERT(client_id > cPlayer::INVALID_ID);
		name.Format(U_CHAR("Guard%d"), -client_id);
	} while (cPlayerManager::Is_Player_Present(name));

	Create_Player(client_id, name, -1, 0);
}

//-----------------------------------------------------------------------------
SoldierGameObj * cGod::Create_Commando(int client_id, int player_type/*, int model_num*/)
{
   WWASSERT(cNetwork::I_Am_Server());
	WWASSERT(player_type >= PLAYERTYPE_NEUTRAL && player_type <= PLAYERTYPE_LAST);

	WWASSERT(PTheGameData != NULL);

	StringClass preset_name;
	preset_name.Format("Commando");

	if (IS_MISSION) {

#ifndef MULTIPLAYERDEMO
		SpawnerClass * p_spawner = SpawnManager::Get_Primary_Spawner();
		if (p_spawner != NULL) {
			const DynamicVectorClass<int>	& def_list = p_spawner->Get_Definition().Get_Spawn_Definition_ID_List();
			WWASSERT(def_list.Count() >= 1);
			PhysicalGameObjDef * p_def = (PhysicalGameObjDef *)DefinitionMgrClass::Find_Definition(def_list[0]);
			if (p_def != NULL) {
				preset_name.Format("%s", p_def->Get_Name());
			}
		}
#endif // !MULTIPLAYERDEMO

	} else if (The_Game()->Is_Cnc() || The_Game()->Is_Skirmish()) {
		if (player_type == PLAYERTYPE_NOD) {
			preset_name.Format("CnC_Nod_Minigunner_0");
		} else {
			preset_name.Format("CnC_GDI_MiniGunner_0");
		}
	}

	WWASSERT(!preset_name.Is_Empty());
	PhysicalGameObj * p_phys_obj = ObjectLibraryManager::Create_Object(preset_name);
	WWASSERT(p_phys_obj != NULL);

	SoldierGameObj * p_soldier = p_phys_obj->As_SoldierGameObj();
	WWASSERT(p_soldier != NULL);
	WWASSERT(p_soldier->Peek_Physical_Object() != NULL);

	if (IS_SOLOPLAY) {
		// Setup initial health depending on difficulty level
		float max = 100.0f;
		switch ( CombatManager::Get_Difficulty_Level() ) {
			case 0:	max = 200;	break;
			case 1:	max = 100;	break;
			case 2:	max = 75;	break;
		};
		p_soldier->Get_Defense_Object()->Set_Health_Max( max );
		p_soldier->Get_Defense_Object()->Set_Health( max );
		p_soldier->Get_Defense_Object()->Set_Shield_Strength_Max( max );
		p_soldier->Get_Defense_Object()->Set_Shield_Strength( max );
	}

	Matrix3D transform;
	if (IS_MISSION && player_type == PLAYERTYPE_GDI) {
		transform = SpawnManager::Get_Primary_Spawn_Location();
	} else {
		transform = SpawnManager::Get_Multiplayer_Spawn_Location(player_type,p_soldier);
	}
	p_soldier->Set_Transform(transform);

	p_soldier->Set_Control_Owner(client_id);
	cPlayer * player = cPlayerManager::Find_Player( client_id );
	p_soldier->Set_Player_Data( player );

	if ( The_Game()->Remember_Inventory() ) {
		if (IS_MISSION) {
			cGod::Restore_Inventory( p_soldier );
		}
	}

	p_soldier->Set_Player_Type(player_type);

	//
	// TSS082901 - We cannot remove all observers - there may be scripts granting
	// initial weapons. Instead, make sure the presets don't UseInnateBehavior
	//
	//p_soldier->Remove_All_Observers();

	if (!p_soldier->Is_Human_Controlled()) {
		//
		// Add an observer to do innate AI
		//
		p_soldier->Set_Innate_Observer(new SoldierObserverClass);
		p_soldier->Add_Observer(p_soldier->Get_Innate_Observer());
	}

	//
	// Added this 090401
	//
	p_soldier->Start_Observers();

	The_Game()->Soldier_Added(p_soldier);

	if (cNetwork::I_Am_Client() && client_id == cNetwork::Get_My_Id()) {
		ActionParamsStruct parameters;
		WWASSERT(p_soldier->Get_Action() != NULL);
		p_soldier->Get_Action()->Follow_Input(parameters);
		CombatManager::Set_The_Star(p_soldier);

		//
		//	Let the cheat manager apply its cheats to the new player
		//
		CheatMgrClass::Get_Instance()->Apply_Cheats();


#ifdef WWDEBUG
		Reinitialize_Ai_On_Star();
#endif // WWDEBUG
	}

	return p_soldier;
}

//-----------------------------------------------------------------------------
SoldierGameObj * cGod::Create_Commando(cPlayer * p_player)
{
   WWASSERT(cNetwork::I_Am_Server());
	WWASSERT(p_player != NULL);

	int client_id		= p_player->Get_Id();
	int player_type	= p_player->Get_Player_Type();
	//int model_num		= p_player->Get_Model();

	return Create_Commando(client_id, player_type/*, model_num*/);
}

//-----------------------------------------------------------------------------
void cGod::Create_Grunt(Vector3 & pos)
{
	WWASSERT(cNetwork::I_Am_Server());

	int client_id		= SmartGameObj::SERVER_CONTROL_OWNER;

	WWASSERT(PTheGameData != NULL);
	int player_type	= The_Game()->Choose_Player_Type(NULL, -1, true);
	//int model_num		= rand() % NUM_MP_PLAYABLE_MODELS;

	SoldierGameObj * p_soldier = Create_Commando(
		client_id, player_type/*, model_num*/);
	WWASSERT(p_soldier != NULL);

	p_soldier->Set_Position(pos);
	p_soldier->Perturb_Position();
}

//-----------------------------------------------------------------------------
#ifdef WWDEBUG
void cGod::Reinitialize_Ai_On_Star(void)
{
	WWASSERT(cNetwork::I_Am_Client());

	SmartGameObj * p_my_soldier = GameObjManager::Find_Soldier_Of_Client_ID(cNetwork::Get_My_Id());

	if (p_my_soldier != NULL) {

		//
		// Remove any innate observers
		//
		const GameObjObserverList & observer_list = p_my_soldier->Get_Observers();
		for (int index = 0; index < observer_list.Count(); index++) {
			if (!stricmp(observer_list[index]->Get_Name(), "Innate Soldier")) {
				p_my_soldier->Remove_Observer(observer_list[index]);
				break; // probably not safe to continue
			}
		}

		cPlayer * p_player = cNetwork::Get_My_Player_Object();
		WWASSERT(p_player != NULL);

		ActionParamsStruct parameters;
		WWASSERT(p_my_soldier->Get_Action() != NULL);
		p_my_soldier->Get_Action()->Follow_Input(parameters);

		CombatManager::Set_Is_Star_Determining_Target(true);
	}
}
#endif // WWDEBUG

//-----------------------------------------------------------------------------
Matrix3D		_StarRespawnTM;
InventoryClass	_DeathInventory;

void cGod::Reset( void )
{
	State = GOD_STATE_UNINITIALIZED;
}

void cGod::Exit( void )
{
	State = GOD_STATE_EXITING;
}

void cGod::Star_Killed( void )
{
	if ( State == GOD_STATE_SINGLE_RUNNING ) {
		State = GOD_STATE_SINGLE_DEAD;
		WWDEBUG_SAY(( "Star Killed\n" ));

		_DeathInventory.Store_Inventory( COMBAT_STAR );

		_StarRespawnTM = COMBAT_STAR->Get_Transform();

		DeathOptionsPopupClass * popup = new DeathOptionsPopupClass;
		popup->Start_Dialog();
		popup->Release_Ref();
	} else if ( State == GOD_STATE_MULTIPLAYER ) {

		if (GameModeManager::Find ("Combat")->Is_Active ()) {
			if (GameModeManager::Find ("Menu")->Is_Active ()) {
				GameModeManager::Find ("Menu")->Deactivate ();
			} else {
				DialogMgrClass::Flush_Dialogs ();
			}
		}
	}
}

void cGod::Respawn( void )
{
	WWASSERT( State == GOD_STATE_SINGLE_DEAD );
	SoldierGameObj * soldier = Create_Commando( cPlayerManager::Get_Player_Object_List()->Head()->Data() );
	soldier->Set_Transform( _StarRespawnTM );
	_DeathInventory.Restore_Inventory( soldier );

	const char * script_name = CombatManager::Get_Respawn_Script();
	ScriptClass* script = ScriptManager::Create_Script( script_name );
	if (script) {
		soldier->Add_Observer( script );
	}

	State = GOD_STATE_SINGLE_RUNNING;
}

void cGod::Restart( void )
{
	if ( State == GOD_STATE_SINGLE_DEAD ) {
//		WWASSERT( State == GOD_STATE_SINGLE_DEAD );

		State = GOD_STATE_SINGLE_RUNNING;	// Incase we get a second call!
		((CombatGameModeClass *)GameModeManager::Find("Combat"))->Core_Restart();

		//
		//	Reset the player's stats
		//
		cPlayer *player_info = cPlayerManager::Get_Player_Object_List()->Head()->Data();
		if ( player_info != NULL ) {
			player_info->Stats_Reset();
		}

		SoldierGameObj * soldier = Create_Commando( player_info );

		const char * script_name = CombatManager::Get_Start_Script();
		ScriptClass* script = ScriptManager::Create_Script( script_name );
		if (script) {
			soldier->Add_Observer( script );
		}
		State = GOD_STATE_SINGLE_RUNNING;	// restart makes it exit

	}
}

void cGod::Load_Game( void )
{
	WWASSERT( State == GOD_STATE_SINGLE_DEAD );
	GameInitMgrClass::End_Game();
	RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_LOAD_GAME);
}


void cGod::Mission_Failed( void )
{
	if ( State == GOD_STATE_SINGLE_RUNNING ) {
		State = GOD_STATE_SINGLE_DEAD;
		WWDEBUG_SAY(( "Mission Failed\n" ));

		FailedOptionsPopupClass * popup = new FailedOptionsPopupClass;
		popup->Start_Dialog();
		popup->Release_Ref();
	}
}


void cGod::Store_Inventory( SoldierGameObj * soldier )
{
	LevelStartInventory.Store_Inventory( soldier );
	EncyclopediaMgrClass::Store_Data();
	return ;
}

void cGod::Restore_Inventory( SoldierGameObj * soldier )
{
	LevelStartInventory.Restore_Inventory( soldier );
	EncyclopediaMgrClass::Restore_Data();
	return ;
}

void cGod::Reset_Inventory( void )
{
	LevelStartInventory.Reset();
}


