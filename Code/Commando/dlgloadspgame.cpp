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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgloadspgame.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/23/02 12:05p                                              $*
 *                                                                                             *
 *                    $Revision:: 29                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dlgloadspgame.h"
#include "listctrl.h"
#include "renegadedialog.h"
#include "gamedata.h"
#include "gamemode.h"
#include "gameinitmgr.h"
#include "savegame.h"
#include "devoptions.h"
#include "translatedb.h"
#include "string_ids.h"
#include "campaign.h"
#include "dialogmgr.h"
#include "renegadedialogmgr.h"
#include "god.h"
#include "registry.h"
#include "_globals.h"
#include "dialogtests.h"
#include "specialbuilds.h"
#include <chrono>
#include <filesystem>


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	MBEVENT_DELETE_PROMPT	= 1,
};

enum
{
	COL_DATE	= 0,
	COL_TIME,
	COL_NAME
};

////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
LoadSPGameMenuClass *	LoadSPGameMenuClass::_TheInstance	= nullptr;


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::On_Init_Dialog (void)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LOAD_GAME_LIST_CTRL);
	if (list_ctrl != nullptr) {

		//
		//	Configure the columns
		//
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_TIME), 0.25F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_DATE), 0.25F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_TEXT177), 0.5F, Vector3 (1, 1, 1));

		//
		//	Build the lists
		//
		int start_index = 0;
		std::string file_filter;
		file_filter = "^M.*\\.mix$";

#ifdef WWDEBUG
		if (cDevOptions::FilterLevelFiles.Is_False()) {
			file_filter = "^.*\\.mix$";
		}
#endif // WWDEBUG

		start_index = Build_List ("Data", std::regex(file_filter, std::regex::icase), start_index);
		Build_List ("Data/save", std::regex("^.*\\.sav$", std::regex::icase), start_index);

		//
		//	Sort the list and select the first entry
		//
		list_ctrl->Sort (LoadListSortCallback, MAKELONG (CurrSortCol, IsSortAscending));
		list_ctrl->Set_Curr_Sel (0);
		Update_Button_State ();

		//
		//	Update the sort marker
		//
		ListCtrlClass::SORT_TYPE type = IsSortAscending ? ListCtrlClass::SORT_ASCENDING : ListCtrlClass::SORT_DESCENDING;
		list_ctrl->Set_Sort_Designator (CurrSortCol, type);
	}

	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Build_List
//
////////////////////////////////////////////////////////////////
int
LoadSPGameMenuClass::Build_List (const char *search_dir, const std::regex &search_string, int start_index)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LOAD_GAME_LIST_CTRL);

	//
	//	Build a list of all the saved games we know about
	//
	int index = start_index;
	for (const auto& entry : std::filesystem::directory_iterator(search_dir)) {
		if(!entry.is_regular_file()) {
			continue;
		}

		std::string filename = entry.path().filename().string();

		if(!std::regex_match(filename, search_string)) {
			continue;
		}

		if ( Is_Game_Allowed( filename.c_str() ) ) {

			//
			//	Get the user description and map name from the file
			//
			WideStringClass description;
			WideStringClass map_name;
			SaveGameManager::Smart_Peek_Description (filename.c_str(), description, map_name);

			//
			//	Default to the map name if we don't have a description
			//
			if (description.Is_Empty ()) {
				description = map_name;
			}

			// Get rank
			int rank = Get_Game_Rank( filename.c_str() );

			// Modify description for games with a rank
			if ( rank ) {
				int mission = atoi( filename.c_str()+1 );
				switch ( mission ) {
					case 0:	description = TRANSLATE( IDS_LoadScreen_Tutorial_Item_00_Title ); break;
					case 1:	description = TRANSLATE( IDS_Enc_Miss_Title_M01_01 ); break;
					case 2:	description = TRANSLATE( IDS_Enc_Miss_Title_M02_01 ); break;
					case 3:	description = TRANSLATE( IDS_Enc_Miss_Title_M03_01 ); break;
					case 4:	description = TRANSLATE( IDS_Enc_Miss_Title_M04_01 ); break;
					case 5:	description = TRANSLATE( IDS_Enc_Miss_Title_M05_01 ); break;
					case 6:	description = TRANSLATE( IDS_Enc_Miss_Title_M06_01 ); break;
					case 7:	description = TRANSLATE( IDS_Enc_Miss_Title_M07_01 ); break;
					case 8:	description = TRANSLATE( IDS_Enc_Miss_Title_M08_01 ); break;
					case 9:	description = TRANSLATE( IDS_Enc_Miss_Title_M09_01 ); break;
					case 10:	description = TRANSLATE( IDS_Enc_Miss_Title_M10_01 ); break;
					case 11:	description = TRANSLATE( IDS_Enc_Miss_Title_M11_01 ); break;
					case 13:	description = TRANSLATE( IDS_LoadScreen_MX0_Title ); break;
				}
			}

			//
			//	Get the time this file was last written
			//
			std::chrono::time_point<std::chrono::system_clock> ftime;
			
#if !defined __MINGW32__
		ftime = std::chrono::clock_cast<std::chrono::system_clock>(entry.last_write_time());
#elif defined _WIN32
		ftime = std::chrono::utc_clock::to_sys(std::chrono::file_clock::to_utc(entry.last_write_time()));
#else
		ftime = std::chrono::file_clock::to_sys(entry.last_write_time());
#endif

			std::chrono::time_point fd(std::chrono::floor<std::chrono::days>(ftime));
			std::chrono::year_month_day ymd(fd);
			std::chrono::hh_mm_ss hms(std::chrono::floor<std::chrono::seconds>(ftime - fd));

			//
			//	Build the time and date strings
			//
			WideStringClass time_string;
			WideStringClass date_string;
			time_string.Format (U_CHAR("%d:%02d:%02d"), hms.hours().count(), hms.minutes().count(), hms.seconds().count());
			date_string.Format (U_CHAR("%02u/%02u/%04d"), static_cast<unsigned>(ymd.month()), static_cast<unsigned>(ymd.day()), static_cast<int>(ymd.year()));

			//
			//	Add this entry to the list control
			//
			int item_index = list_ctrl->Insert_Entry (index ++, time_string);
			if (item_index >= 0) {
				list_ctrl->Set_Entry_Text (item_index, 1, date_string);
				list_ctrl->Set_Entry_Text (item_index, 2, description);
				for ( int i = 0; i < rank; i++ ) {
					list_ctrl->Add_Icon (item_index, 2, "IF_LRGSTAR.TGA");
				}

				//
				//	Build the full path to the file
				//
				StringClass file_path = search_dir;
				file_path += "/";
				file_path += filename.c_str();

				list_ctrl->Set_Entry_Data (item_index, 0, (uintptr_t)new std::chrono::time_point<std::chrono::system_clock>(ftime));
				list_ctrl->Set_Entry_Data (item_index, 1, (uintptr_t)new StringClass(file_path));
				list_ctrl->Set_Entry_Data (item_index, 2, (uintptr_t)new StringClass(filename.c_str()));
			}
		}
	}

	return index;
}

////////////////////////////////////////////////////////////////
//
//	Is_Game_Allowed
//
////////////////////////////////////////////////////////////////
bool
LoadSPGameMenuClass::Is_Game_Allowed
(
	const char * filename
)
{
	// Determine if this game is valid to present in the load game menu

	// All save games (.SAV) are allowed
	if ( ::strlen( filename ) > 4 && ::stricmp( filename + (::strlen( filename ) - 4), ".SAV" ) == 0 ) {
		return true;
	}

	// All save games not in the form Mxx.MIX are allowed
	if ( ( ::strlen( filename ) != 7 ) ||
		 ( ::stricmp( filename + (::strlen( filename ) - 4), ".MIX" ) != 0 ) ||
		 ( ::toupper( *filename ) != 'M' ) ) {
		return true;
	}

	// if we have a rank, allow it
	return ( Get_Game_Rank( filename ) > 0 );
}

////////////////////////////////////////////////////////////////
//
//	Get_Game_Rank
//
////////////////////////////////////////////////////////////////
int
LoadSPGameMenuClass::Get_Game_Rank
(
	const char * filename
)
{
	// Strip extention (so we can pass in .MIX, or .LSD
	StringClass name = filename;
	if ( name.Get_Length() > 4 ) {
		name.Erase( name.Get_Length()-4, 4 );
	}
	RegistryClass * registry = new RegistryClass( APPLICATION_SUB_KEY_NAME_MISSION_RANKS );
	WWASSERT( registry );
	int rank = 0;
	if ( registry->Is_Valid() ) {
		rank = registry->Get_Int( name,   0 );
	}
	delete registry;
	return rank;
}


////////////////////////////////////////////////////////////////
//
//	Set_Game_Rank
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::Set_Game_Rank
(
	const char * filename,
	int rank
)
{
	// Strip extention
	StringClass name = filename;
	if ( name.Get_Length() > 4 ) {
		name.Erase( name.Get_Length()-4, 4 );
	}
	RegistryClass * registry = new RegistryClass( APPLICATION_SUB_KEY_NAME_MISSION_RANKS );
	WWASSERT( registry );
	if ( registry->Is_Valid() ) {
		int old_rank = registry->Get_Int( name, 0 );
		// Set to the max of old and rank
		if ( old_rank > rank ) {
			rank = old_rank;
		}
		registry->Set_Int( name, rank );

	}
	delete registry;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Column_Click
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::On_ListCtrl_Column_Click
(
	ListCtrlClass *list_ctrl,
	int				ctrl_id,
	int				col_index
)
{
	if (ctrl_id == IDC_LOAD_GAME_LIST_CTRL) {

		//
		//	Determine
		//
		if (col_index == CurrSortCol) {
			IsSortAscending = !IsSortAscending;
		} else {
			CurrSortCol = uint16(col_index);
			IsSortAscending = true;
		}

		//
		//	Sort the list by the column that was clicked
		//
		list_ctrl->Sort (LoadListSortCallback, MAKELONG (CurrSortCol, IsSortAscending));

		//
		//	Update the sort marker
		//
		ListCtrlClass::SORT_TYPE type = IsSortAscending ? ListCtrlClass::SORT_ASCENDING : ListCtrlClass::SORT_DESCENDING;
		list_ctrl->Set_Sort_Designator (CurrSortCol, type);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Delete_Entry
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::On_ListCtrl_Delete_Entry
(
	ListCtrlClass *list_ctrl,
	int				ctrl_id,
	int				item_index
)
{
	if (ctrl_id == IDC_LOAD_GAME_LIST_CTRL) {

		//
		//	Remove the data we associated with this entry
		//
		std::chrono::time_point<std::chrono::system_clock> *file_time		= (std::chrono::time_point<std::chrono::system_clock> *)list_ctrl->Get_Entry_Data (item_index, 0);
		StringClass *path			= (StringClass *)list_ctrl->Get_Entry_Data (item_index, 1);
		StringClass *filename	= (StringClass *)list_ctrl->Get_Entry_Data (item_index, 2);
		list_ctrl->Set_Entry_Data (item_index, 0, 0);
		list_ctrl->Set_Entry_Data (item_index, 1, 0);
		list_ctrl->Set_Entry_Data (item_index, 2, 0);

		//
		//	Free the data
		//
		if (file_time != nullptr) {
			delete file_time;
		}

		if (path != nullptr) {
			delete path;
		}

		if (filename != nullptr) {
			delete filename;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	LoadListSortCallback
//
////////////////////////////////////////////////////////////////
int
LoadSPGameMenuClass::LoadListSortCallback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param)
{
	int retval = 0;

	//
	//	Get the sorting params
	//
	int	sort_col_index = user_param & 0xFFFF;
	bool	sort_ascending	= (user_param >> 16) != 0;

	if (sort_col_index == 0 || sort_col_index == 1) {

		//
		//	Sort by time
		//
		std::chrono::time_point<std::chrono::system_clock> *file_time1 = (std::chrono::time_point<std::chrono::system_clock> *)list_ctrl->Get_Entry_Data (item_index1, 0);
		std::chrono::time_point<std::chrono::system_clock> *file_time2 = (std::chrono::time_point<std::chrono::system_clock> *)list_ctrl->Get_Entry_Data (item_index2, 0);
		retval = 0;
			if (*file_time1 < *file_time2) {
				retval = -1;
			} else if (*file_time1 < *file_time2) {
				retval = 1;
			}

	} else {

		//
		//	Sort by name
		//
		const unichar_t *name1 = list_ctrl->Get_Entry_Text (item_index1, 2);
		const unichar_t *name2 = list_ctrl->Get_Entry_Text (item_index2, 2);
		retval = ::u_strcasecmp (name1, name2, U_COMPARE_CODE_POINT_ORDER);
	}

	//
	//	Invert the return value if we are sorting descendingly
	//
	if (sort_ascending == false) {
		retval = -retval;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_LOAD_GAME_BUTTON:
		case IDOK:
			Load_Game ();
			break;

		case IDC_DELETE_GAME_BUTTON:
			Delete_Game (true);
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load_Game
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::Load_Game (void)
{
#ifndef MULTIPLAYERDEMO

	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LOAD_GAME_LIST_CTRL);
	if (list_ctrl == nullptr) {
		return ;
	}

#if 0 // MOVED BELOW AND TO DIFFICULTY SELECTION
	//
	//	End the current game before we load the new one
	//
	if (GameModeManager::Find ("Combat")->Is_Suspended ()) {
		GameInitMgrClass::End_Game();
		GameModeManager::Safely_Deactivate ();
	}
#endif
	//
	//	Get the currently selected entry
	//
	int item_index = list_ctrl->Get_Curr_Sel ();
	if (item_index != -1) {

		//
		//	Get the name of the map
		//
		StringClass *filename	= (StringClass *)list_ctrl->Get_Entry_Data (item_index, 2);
		StringClass save_name(filename->Peek_Buffer(),true);

		StringClass map_name(0,true);
		int mission = 0;
		if (SaveGameManager::Peek_Map_Name (save_name, map_name)) {
			mission = cGameData::Get_Mission_Number_From_Map_Name( map_name );
		} else {
			mission = cGameData::Get_Mission_Number_From_Map_Name( save_name );
		}
		CampaignManager::Select_Backdrop_Number( mission );		// Force default loading screen

		bool is_replay = Get_Game_Rank( save_name ) > 0;
		if ( is_replay ) {
			// if replay
			DifficultyMenuClass * dialog = new DifficultyMenuClass();
			dialog->Set_Replay( save_name );
			dialog->Start_Dialog();
			dialog->Release_Ref();
		} else {

			//
			//	End the current game before we load the new one
			//
			if (GameModeManager::Find ("Combat")->Is_Suspended ()) {
				GameInitMgrClass::End_Game();
				GameModeManager::Safely_Deactivate ();
			}

			//
			//	Load the map
			//
			GameInitMgrClass::Initialize_SP ();
			GameInitMgrClass::Start_Game (save_name, -1, 0);
		}
	}

	cGod::Reset_Inventory();

	return ;

#endif // !MULTIPLAYERDEMO
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Sel_Change
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::On_ListCtrl_Sel_Change
(
	ListCtrlClass * /* list_ctrl */,
	int				/* ctrl_id */,
	int				/* old_index */,
	int				/* new_index */
)
{
	Update_Button_State ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Button_State
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::Update_Button_State (void)
{
	bool enable = false;

	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LOAD_GAME_LIST_CTRL);
	if (list_ctrl == nullptr) {
		return ;
	}

	//
	//	Get the currently selected entry
	//
	int item_index = list_ctrl->Get_Curr_Sel ();
	if (item_index != -1) {

		//
		//	Get the filename associated with this entry
		//
		if (list_ctrl->Get_Entry_Data (item_index, 0) != 0) {
			StringClass filename = ((StringClass *)list_ctrl->Get_Entry_Data (item_index, 1))->Peek_Buffer ();

			//
			//	Check to see if this is a saved game or a level file.
			//
			const size_t len = filename.Get_Length ();
			if (len >= 4 && ::stricmp ((filename.Peek_Buffer () + (len - 4)), ".mix") != 0) {
				enable = true;
			}
		}

		//
		//	Change the enable state of the button
		//
		Get_Dlg_Item (IDC_DELETE_GAME_BUTTON)->Enable (enable);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_DblClk
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::On_ListCtrl_DblClk
(
	ListCtrlClass * /* list_ctrl */,
	int				/* ctrl_id */,
	int				/* item_index */
)
{
	Load_Game ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Delete_Game
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::Delete_Game (bool prompt)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LOAD_GAME_LIST_CTRL);
	if (list_ctrl == nullptr) {
		return ;
	}

	//
	//	Get the currently selected entry
	//
	int item_index = list_ctrl->Get_Curr_Sel ();
	if (item_index != -1) {

		//
		//	Determine what filename this entry refers to
		//
		if (list_ctrl->Get_Entry_Data (item_index, 0) != 0) {
			StringClass filename = ((StringClass *)list_ctrl->Get_Entry_Data (item_index, 1))->Peek_Buffer ();

			// Never delete .MIX files
			const size_t len = filename.Get_Length ();
			if (len >= 4 && ::stricmp ((filename.Peek_Buffer () + (len - 4)), ".mix") != 0) {

				if (prompt) {

					//
					//	Build a confirmation message to display to the user
					//
					WideStringClass message;
					message.Format (TRANSLATE (IDS_MENU_DELETE_SAVE_MSG),
								list_ctrl->Get_Entry_Text (item_index, COL_NAME));

					//
					//	Display a message to the user asking if they really want to do this...
					//
					DlgMsgBox::DoDialog (TRANSLATE (IDS_MENU_DELETE_SAVE_TITLE), message, DlgMsgBox::YesNo,
									this, MBEVENT_DELETE_PROMPT);
				} else {

					//
					//	Delete the file and remove its entry from the list
					//
					if (::DeleteFileA (filename) != 0) {
						list_ctrl->Delete_Entry (item_index);
					}
				}
			}
		}
	}

	Update_Button_State ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	HandleNotification
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::HandleNotification (DlgMsgBoxEvent &event)
{
	if (event.Get_User_Data () == MBEVENT_DELETE_PROMPT) {

		//
		//	The user has confirmed the delete, so delete the file
		//
		if (event.Event () == DlgMsgBoxEvent::Yes) {
			Delete_Game (false);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::Display (void)
{
#ifndef MULTIPLAYERDEMO

	//
	//	Create the dialog if necessary, otherwise simply bring it to the front
	//
	if (_TheInstance == nullptr) {
		START_DIALOG (LoadSPGameMenuClass);
	} else {
		if (_TheInstance->Is_Active_Menu () == false) {
			DialogMgrClass::Rollback (_TheInstance);
		}
	}

	return ;

#endif // !MULTIPLAYERDEMO
}


////////////////////////////////////////////////////////////////
//
//	On_Last_Menu_Ending
//
////////////////////////////////////////////////////////////////
void
LoadSPGameMenuClass::On_Last_Menu_Ending (void)
{
	RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
	return ;
}
