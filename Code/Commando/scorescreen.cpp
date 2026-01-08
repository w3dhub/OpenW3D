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
 *                     $Archive:: /Commando/Code/Commando/scorescreen.cpp                     $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/24/02 11:20a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 27                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "scorescreen.h"
#include "campaign.h"
#include "renegadedialog.h"
#include "translatedb.h"
#include "string_ids.h"
#include "imagectrl.h"
#include "debug.h"
#include "wwstring.h"
#include "gamedata.h"
#include "combat.h"
#include "playerdata.h"
#include "cheatmgr.h"
#include "objectives.h"
#include "dlgloadspgame.h"
#include "savegame.h"

#define	NUM_MISSIONS	13

/*
**
*/
void 	ScoreScreenGameModeClass::Init()
{
	START_DIALOG( ScoreScreenDialogClass );
}

float _SSStats_play_time = 0;
int	  _SSStats_mission_number = 0;
int	  _SSStats_reload_count = 0;
bool  _SSStats_cheated = 0;
int	  _SSStats_secondary_objectives = 0;
int	  _SSStats_completed_secondary_objectives = 0;
int	  _SSStats_tertiary_objectives = 0;
int	  _SSStats_completed_tertiary_objectives = 0;
int	  _SSStats_building_destroyed = 0;
int	  _SSStats_vehicles_destroyed = 0;
int	  _SSStats_enemies_killed = 0;
int	  _SSStats_head_hit = 0;
int	  _SSStats_torso_hit = 0;
int	  _SSStats_arm_hit = 0;
int	  _SSStats_crotch_hit = 0;
int	  _SSStats_leg_hit = 0;
StringClass	  _SSStats_map_name;

/*
** We save all the stats before the combat is shutdown
*/
void ScoreScreenGameModeClass::Save_Stats( void )
{
	_SSStats_play_time = 100000;
	_SSStats_building_destroyed = 0;
	_SSStats_vehicles_destroyed = 0;
	_SSStats_enemies_killed = 0;
	_SSStats_head_hit = 0;
	_SSStats_torso_hit = 0;
	_SSStats_arm_hit = 0;
	_SSStats_crotch_hit = 0;
	_SSStats_leg_hit = 0;

	if ( COMBAT_STAR ) {
		PlayerDataClass * player_data = COMBAT_STAR->Get_Player_Data();
		if ( player_data != NULL ) {
			_SSStats_play_time = player_data->Get_Game_Time();
			_SSStats_building_destroyed = player_data->Get_Building_Destroyed();
			_SSStats_vehicles_destroyed = player_data->Get_Vehiclies_Destroyed();
			_SSStats_enemies_killed = player_data->Get_Enemies_Killed();
			_SSStats_head_hit = player_data->Get_Head_Shots();
			_SSStats_torso_hit = player_data->Get_Torso_Shots();
			_SSStats_arm_hit = player_data->Get_Arm_Shots();
			_SSStats_crotch_hit = player_data->Get_Crotch_Shots();
			_SSStats_leg_hit = player_data->Get_Leg_Shots();
		}
	}

	// Save the map name
	_SSStats_map_name = The_Game()->Get_Map_Name();
	StringClass map_name(0,true);
	if (SaveGameManager::Peek_Map_Name (_SSStats_map_name, map_name)) {
		_SSStats_map_name = map_name;
	}


	WWASSERT(PTheGameData != NULL);
	_SSStats_mission_number = cGameData::Get_Mission_Number_From_Map_Name( _SSStats_map_name );

	// Oh, this is just terrible!!!  Byon
#define	TUTORIAL_LOAD_MENU_NUMBER		90
	if ( _SSStats_mission_number == TUTORIAL_LOAD_MENU_NUMBER ) {
		_SSStats_mission_number = 0;
	}

	if ( _SSStats_mission_number > NUM_MISSIONS	) {
		_SSStats_mission_number = NUM_MISSIONS;
	}
	if ( _SSStats_mission_number < 0 ) {
		_SSStats_mission_number = 0;
	}

	_SSStats_reload_count = CombatManager::Get_Reload_Count();

	_SSStats_cheated = CheatMgrClass::Get_Instance()->Was_Cheat_Used( CheatMgrClass::ALL_CHEATS );

	_SSStats_secondary_objectives = ObjectiveManager::Get_Num_Objectives( ObjectiveManager::TYPE_SECONDARY );
	_SSStats_completed_secondary_objectives = ObjectiveManager::Get_Num_Completed_Objectives( ObjectiveManager::TYPE_SECONDARY );

	_SSStats_tertiary_objectives = ObjectiveManager::Get_Num_Objectives( ObjectiveManager::TYPE_TERTIARY );
	_SSStats_completed_tertiary_objectives = ObjectiveManager::Get_Num_Completed_Objectives( ObjectiveManager::TYPE_TERTIARY );

}

////////////////////////////////////////////////////////////////
//
//	ScoreScreenDialogClass
//
////////////////////////////////////////////////////////////////
ScoreScreenDialogClass::ScoreScreenDialogClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_SCORE_SCREEN))
{
	ScoreScreenActive = true;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
ScoreScreenDialogClass::On_Init_Dialog (void)
{
	MenuDialogClass::On_Init_Dialog ();

	int time_to_finish_stars		= Get_Time_To_Finish_Stars();
	int level_of_play_stars			= Get_Level_Of_Play_Stars();
	int saves_loaded_stars			= Get_Saves_Loaded_Stars();
	int secondary_missions_stars	= Get_Secondary_Missions_Stars();
	int overall_stars = (time_to_finish_stars + level_of_play_stars + 
						saves_loaded_stars + secondary_missions_stars) / 4;

	// Update the ranking
	LoadSPGameMenuClass::Set_Game_Rank( _SSStats_map_name, overall_stars );

	/*
	** Setup Star Textures
	*/

#define	BIG_STAR_TEXTURE	"IF_LrgStar.tga"
#define	SMALL_STAR_TEXTURE	"IF_LrgStar.tga"

	((ImageCtrlClass*)Get_Dlg_Item( IDC_SS_BACKGROUND ))->Set_Texture( "if_missioncomp.tga" );

	((ImageCtrlClass*)Get_Dlg_Item( IDC_MISSION_EVAL_STAR_1 ))->Set_Texture( BIG_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_MISSION_EVAL_STAR_2 ))->Set_Texture( BIG_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_MISSION_EVAL_STAR_3 ))->Set_Texture( BIG_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_MISSION_EVAL_STAR_4 ))->Set_Texture( BIG_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_MISSION_EVAL_STAR_5 ))->Set_Texture( BIG_STAR_TEXTURE );

	((ImageCtrlClass*)Get_Dlg_Item( IDC_TIME_TO_FINISH_STAR_1 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_TIME_TO_FINISH_STAR_2 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_TIME_TO_FINISH_STAR_3 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_TIME_TO_FINISH_STAR_4 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_TIME_TO_FINISH_STAR_5 ))->Set_Texture( SMALL_STAR_TEXTURE );

	((ImageCtrlClass*)Get_Dlg_Item( IDC_LEVEL_OF_PLAY_STAR_1 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_LEVEL_OF_PLAY_STAR_2 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_LEVEL_OF_PLAY_STAR_3 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_LEVEL_OF_PLAY_STAR_4 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_LEVEL_OF_PLAY_STAR_5 ))->Set_Texture( SMALL_STAR_TEXTURE );

	((ImageCtrlClass*)Get_Dlg_Item( IDC_SAVES_LOADED_STAR_1 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_SAVES_LOADED_STAR_2 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_SAVES_LOADED_STAR_3 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_SAVES_LOADED_STAR_4 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_SAVES_LOADED_STAR_5 ))->Set_Texture( SMALL_STAR_TEXTURE );

	((ImageCtrlClass*)Get_Dlg_Item( IDC_SECONDARY_MISSIONS_STAR_1 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_SECONDARY_MISSIONS_STAR_2 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_SECONDARY_MISSIONS_STAR_3 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_SECONDARY_MISSIONS_STAR_4 ))->Set_Texture( SMALL_STAR_TEXTURE );
	((ImageCtrlClass*)Get_Dlg_Item( IDC_SECONDARY_MISSIONS_STAR_5 ))->Set_Texture( SMALL_STAR_TEXTURE );

	Get_Dlg_Item( IDC_MISSION_EVAL_STAR_1 )->Show( overall_stars >= 1 );
	Get_Dlg_Item( IDC_MISSION_EVAL_STAR_2 )->Show( overall_stars >= 2 );
	Get_Dlg_Item( IDC_MISSION_EVAL_STAR_3 )->Show( overall_stars >= 3 );
	Get_Dlg_Item( IDC_MISSION_EVAL_STAR_4 )->Show( overall_stars >= 4 );
	Get_Dlg_Item( IDC_MISSION_EVAL_STAR_5 )->Show( overall_stars >= 5 );

	Get_Dlg_Item( IDC_TIME_TO_FINISH_STAR_1 )->Show( time_to_finish_stars >= 1 );
	Get_Dlg_Item( IDC_TIME_TO_FINISH_STAR_2 )->Show( time_to_finish_stars >= 2 );
	Get_Dlg_Item( IDC_TIME_TO_FINISH_STAR_3 )->Show( time_to_finish_stars >= 3 );
	Get_Dlg_Item( IDC_TIME_TO_FINISH_STAR_4 )->Show( time_to_finish_stars >= 4 );
	Get_Dlg_Item( IDC_TIME_TO_FINISH_STAR_5 )->Show( time_to_finish_stars >= 5 );

	Get_Dlg_Item( IDC_LEVEL_OF_PLAY_STAR_1 )->Show( level_of_play_stars >= 1 );
	Get_Dlg_Item( IDC_LEVEL_OF_PLAY_STAR_2 )->Show( level_of_play_stars >= 2 );
	Get_Dlg_Item( IDC_LEVEL_OF_PLAY_STAR_3 )->Show( level_of_play_stars >= 3 );
	Get_Dlg_Item( IDC_LEVEL_OF_PLAY_STAR_4 )->Show( level_of_play_stars >= 4 );
	Get_Dlg_Item( IDC_LEVEL_OF_PLAY_STAR_5 )->Show( level_of_play_stars >= 5 );

	Get_Dlg_Item( IDC_SAVES_LOADED_STAR_1 )->Show( saves_loaded_stars >= 1 );
	Get_Dlg_Item( IDC_SAVES_LOADED_STAR_2 )->Show( saves_loaded_stars >= 2 );
	Get_Dlg_Item( IDC_SAVES_LOADED_STAR_3 )->Show( saves_loaded_stars >= 3 );
	Get_Dlg_Item( IDC_SAVES_LOADED_STAR_4 )->Show( saves_loaded_stars >= 4 );
	Get_Dlg_Item( IDC_SAVES_LOADED_STAR_5 )->Show( saves_loaded_stars >= 5 );

	Get_Dlg_Item( IDC_SECONDARY_MISSIONS_STAR_1 )->Show( secondary_missions_stars >= 1 );
	Get_Dlg_Item( IDC_SECONDARY_MISSIONS_STAR_2 )->Show( secondary_missions_stars >= 2 );
	Get_Dlg_Item( IDC_SECONDARY_MISSIONS_STAR_3 )->Show( secondary_missions_stars >= 3 );
	Get_Dlg_Item( IDC_SECONDARY_MISSIONS_STAR_4 )->Show( secondary_missions_stars >= 4 );
	Get_Dlg_Item( IDC_SECONDARY_MISSIONS_STAR_5 )->Show( secondary_missions_stars >= 5 );

	bool cheater = _SSStats_cheated;

	// Build Rank Text
	int rank_ids[5] = { IDS_SS_RANK_1, IDS_SS_RANK_2, IDS_SS_RANK_3, IDS_SS_RANK_4, IDS_SS_RANK_5 };
	WideStringClass rank_name = U_CHAR("");
	rank_name += TRANSLATE( rank_ids[ overall_stars-1 ] );
	if ( cheater ) {
		rank_name += TRANSLATE( IDS_SS_CHEATER );
	}

	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_RANK, rank_name );

	// Build Rank Image
	const char * rank_images[5] = { "IF_private.tga", "IF_sargent.tga", "IF_leutenant.tga", "IF_major.tga", "IF_general.tga" };
	const char * rank_image = "IF_cheatgod.tga";
	if ( !cheater ) {
		rank_image = rank_images[ overall_stars-1 ];
	}
	((ImageCtrlClass*)Get_Dlg_Item( IDC_RANK_IMAGE ))->Set_Texture( rank_image );

	WideStringClass wstr;

	wstr.Format( U_CHAR("%d"), _SSStats_building_destroyed );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_BUILDING_COUNT, wstr );
	wstr.Format( U_CHAR("%d"), _SSStats_vehicles_destroyed );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_VEHICLE_COUNT, wstr );
	wstr.Format( U_CHAR("%d"), _SSStats_enemies_killed );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_ENEMY_COUNT, wstr );
	wstr.Format( U_CHAR("%d"), _SSStats_enemies_killed );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_ENEMY_COUNT, wstr );

	wstr.Format( U_CHAR("%d"), _SSStats_head_hit );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_HEAD_COUNT, wstr );
	wstr.Format( U_CHAR("%d"), _SSStats_torso_hit );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_TORSO_COUNT, wstr );
	wstr.Format( U_CHAR("%d"), _SSStats_arm_hit );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_ARM_COUNT, wstr );
	wstr.Format( U_CHAR("%d"), _SSStats_crotch_hit );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_CROTCH_COUNT, wstr );
	wstr.Format( U_CHAR("%d"), _SSStats_leg_hit );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_LEG_COUNT, wstr );

	WideStringClass text;
	text.Format( TRANSLATE( IDS_SS_X_OF_N ), _SSStats_completed_tertiary_objectives, _SSStats_tertiary_objectives );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_BONUS, text );

	int mission_names[ NUM_MISSIONS+1 ] = {
		IDS_LoadScreen_Tutorial_Item_00_Title,
		IDS_Enc_Miss_Title_M01_01,
		IDS_Enc_Miss_Title_M02_01,
		IDS_Enc_Miss_Title_M03_01,
		IDS_Enc_Miss_Title_M04_01,
		IDS_Enc_Miss_Title_M05_01,
		IDS_Enc_Miss_Title_M06_01,
		IDS_Enc_Miss_Title_M07_01,
		IDS_Enc_Miss_Title_M08_01,
		IDS_Enc_Miss_Title_M09_01,
		IDS_Enc_Miss_Title_M10_01,
		IDS_Enc_Miss_Title_M11_01,
		IDS_LoadScreen_MX0_Title,	// 12
		IDS_LoadScreen_MX0_Title,	// 13 (m00)
	};

	int id = mission_names[_SSStats_mission_number];
	Set_Dlg_Item_Text( IDC_SS_MISSION_NAME, TRANSLATE( id ) );

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
ScoreScreenDialogClass::On_Destroy (void)
{
	MenuDialogClass::On_Destroy ();
	if ( ScoreScreenActive ) {
		ScoreScreenActive = false;
		CampaignManager::Continue();
	}
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
ScoreScreenDialogClass::On_Command (int ctrl_id, int mesage_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDCANCEL:
		case IDOK:
			End_Dialog();
		
			break;

		default:
			MenuDialogClass::On_Command (ctrl_id, mesage_id, param);
			break;
	}


	return ;
}



/*
**
*/
float	level_star_times[] = {
/*	STARS	 2		3		4		5	*/
/* 0 */	 10,		
/* 1 */	 20,		
/* 2 */	 40,		
/* 3 */	 20,		
/* 4 */	 25,		
/* 5 */	 30,		
/* 6 */	 20,		
/* 7 */	 35,		
/* 8 */	 20,		
/* 9 */	 25,		
/* 10 */	 35,		
/* 11 */	 25,		
/* 12 */	 10,		
/* 13 */	 10,			// Mission 0
};

int	ScoreScreenDialogClass::Get_Time_To_Finish_Stars( void )
{
	int mission = _SSStats_mission_number;
	float time = (_SSStats_play_time+59.0f) / 60.0f;

	float par_time = level_star_times[mission];

	int stars = 1;
	if ( time <= par_time * 1.5f ) {
		stars = 2;
	}
	if ( time <= par_time * 1.3f ) {
		stars = 3;
	}
	if ( time <= par_time * 1.1f ) {
		stars = 4;
	}
	if ( time <= par_time ) {
		stars = 5;
	}

	int minutes = time;
	WideStringClass wtime;
	wtime.Format( TRANSLATE( IDS_SS_HOURS_MINUTES ), minutes / 60, minutes % 60 );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_PLAYTIME, wtime );

//	Debug_Say(( "Time %f Mission %d = Stars %d\n", time, mission, stars ));
	return stars;
}

/*
**
*/
int	ScoreScreenDialogClass::Get_Level_Of_Play_Stars( void )
{
	if ( CombatManager::Get_Difficulty_Level() >= 2 ) {
		Set_Dlg_Item_Text( IDC_SCORE_SCREEN_DIFFICULTY, TRANSLATE( IDS_MENU_TEXT065 ) );
		return 5;
	}
	if ( CombatManager::Get_Difficulty_Level() >= 1 ) {
		Set_Dlg_Item_Text( IDC_SCORE_SCREEN_DIFFICULTY, TRANSLATE( IDS_MENU_TEXT064 ) );
		return 3;
	}
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_DIFFICULTY, TRANSLATE( IDS_MENU_TEXT063 ) );
	return 1;
}

/*
**
*/
int	ScoreScreenDialogClass::Get_Saves_Loaded_Stars( void )
{
	WideStringClass text;
	text.Format( TRANSLATE( IDS_SS_RELOADS ), _SSStats_reload_count );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_LOADS, text );

	if ( _SSStats_reload_count == 0 ) {
		return 5;
	}
	if ( _SSStats_reload_count <= 2 ) {
		return 4;
	}
	if ( _SSStats_reload_count <= 5 ) {
		return 3;
	}
	if ( _SSStats_reload_count <= 9 ) {
		return 2;
	}
	return 1;
}

/*
**
*/
int	ScoreScreenDialogClass::Get_Secondary_Missions_Stars( void )
{
	WideStringClass text;
	text.Format( TRANSLATE( IDS_SS_X_OF_N ), _SSStats_completed_secondary_objectives, _SSStats_secondary_objectives );
	Set_Dlg_Item_Text( IDC_SCORE_SCREEN_SECONDARY, text );

	float ratio = 1.0f;
	if ( _SSStats_secondary_objectives != 0 ) {
		ratio = (float)_SSStats_completed_secondary_objectives / (float)_SSStats_secondary_objectives;
	}

	if ( ratio >= 1.0f ) {
		return 5;
	}
	if ( ratio >= 0.9f ) {
		return 4;
	}
	if ( ratio >= 0.7f ) {
		return 3;
	}
	if ( ratio >= 0.5f ) {
		return 2;
	}
	return 1;
}
