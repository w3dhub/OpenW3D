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
#include "dialogresource.h"
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
LoadSPGameMenuClass *	LoadSPGameMenuClass::_TheInstance	= NULL;


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
	if (list_ctrl != NULL) {

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
		//start_index = Build_List ("data\\*.mix", start_index);
		//start_index = Build_List ("data\\m??_*.mix", start_index);
		StringClass file_filter;
		file_filter = "data\\m*.mix";

#ifdef WWDEBUG
		if (cDevOptions::FilterLevelFiles.Is_False()) {
			file_filter = "data\\*.mix";
		}
#endif // WWDEBUG

		start_index = Build_List (file_filter, start_index);
		Build_List ("data\\save\\*.sav", start_index);

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
LoadSPGameMenuClass::Build_List (const char *search_string, int start_index)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LOAD_GAME_LIST_CTRL);

	WIN32_FIND_DATA find_info	= { 0 };
	BOOL keep_going				= TRUE;
	HANDLE file_find			= NULL;

	//
	//	Get the path to the directory we'll be searching
	//
	StringClass path_name;
	const char *search_dir = ::strrchr (search_string, '\\');
	if (search_dir != NULL) {

		//
		//	Strip the search mask from the string
		//
		path_name	= search_string;
		int len		= ::strlen (search_string);
		int index	= search_dir - search_string;
		path_name.Erase (index, len - index);
	}
	
	//
	//	Build a list of all the saved games we know about
	//
	int index = start_index;
	for (file_find = ::FindFirstFile (search_string, &find_info);
		 (file_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (file_find, &find_info))
	{
//		if (1 || Is_Game_Allowed( find_info.cFileName ) ) {
		if ( Is_Game_Allowed( find_info.cFileName ) ) {

			//
			//	Get the user description and map name from the file
			//
			WideStringClass description;
			WideStringClass map_name;
			SaveGameManager::Smart_Peek_Description (find_info.cFileName, description, map_name);

			//
			//	Default to the map name if we don't have a description
			//
			if (description.Is_Empty ()) {
				description = map_name;
			}

			// Get rank
			int rank = Get_Game_Rank( find_info.cFileName );

			// Modify description for games with a rank
			if ( rank ) {
				int mission = atoi( find_info.cFileName+1 );
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
			SYSTEMTIME system_time	= { 0 };
			FILETIME local_time		= { 0 };
			::FileTimeToLocalFileTime (&find_info.ftLastWriteTime, &local_time);
			::FileTimeToSystemTime (&local_time, &system_time);

			//
			//	Build the time and date strings
			//
			WideStringClass time_string;
			WideStringClass date_string;
			time_string.Format (L"%d:%02d:%02d", system_time.wHour, system_time.wMinute, system_time.wSecond);
			date_string.Format (L"%d/%d/%d", system_time.wMonth, system_time.wDay, system_time.wYear);

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
				StringClass file_path = path_name;
				file_path += "\\";
				file_path += find_info.cFileName;
				
				list_ctrl->Set_Entry_Data (item_index, 0, (uint32)new FILETIME(local_time));
				list_ctrl->Set_Entry_Data (item_index, 1, (uint32)new StringClass(file_path));
				list_ctrl->Set_Entry_Data (item_index, 2, (uint32)new StringClass(find_info.cFileName));
			}
		}
	}

	if (file_find != INVALID_HANDLE_VALUE) {			  
		::FindClose (file_find); 
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
			CurrSortCol = col_index;
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
		FILETIME *file_time		= (FILETIME *)list_ctrl->Get_Entry_Data (item_index, 0);
		StringClass *path			= (StringClass *)list_ctrl->Get_Entry_Data (item_index, 1);
		StringClass *filename	= (StringClass *)list_ctrl->Get_Entry_Data (item_index, 2);
		list_ctrl->Set_Entry_Data (item_index, 0, 0);
		list_ctrl->Set_Entry_Data (item_index, 1, 0);
		list_ctrl->Set_Entry_Data (item_index, 2, 0);
		
		//
		//	Free the data
		//
		if (file_time != NULL) {
			delete file_time;
		}
		
		if (path != NULL) {
			delete path;
		}
				
		if (filename != NULL) {
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
int CALLBACK
LoadSPGameMenuClass::LoadListSortCallback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param)
{
	int retval = 0;

	//
	//	Get the sorting params
	//
	int	sort_col_index = LOWORD (user_param);
	BOOL	sort_ascending	= HIWORD (user_param);

	if (sort_col_index == 0 || sort_col_index == 1) {
		
		//
		//	Sort by time
		//
		FILETIME *file_time1 = (FILETIME *)list_ctrl->Get_Entry_Data (item_index1, 0);
		FILETIME *file_time2 = (FILETIME *)list_ctrl->Get_Entry_Data (item_index2, 0);
		retval = ::CompareFileTime (file_time1, file_time2);

	} else {
		
		//
		//	Sort by name
		//
		const WCHAR *name1 = list_ctrl->Get_Entry_Text (item_index1, 2);
		const WCHAR *name2 = list_ctrl->Get_Entry_Text (item_index2, 2);
		retval = ::wcsicmp (name1, name2);
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
LoadSPGameMenuClass::On_Command (int ctrl_id, int message_id, DWORD param)
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
	if (list_ctrl == NULL) {
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
	ListCtrlClass *list_ctrl,
	int				ctrl_id,
	int				old_index,
	int				new_index
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
	if (list_ctrl == NULL) {
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
		if (list_ctrl->Get_Entry_Data (item_index, 0) != NULL) {
			StringClass filename = ((StringClass *)list_ctrl->Get_Entry_Data (item_index, 1))->Peek_Buffer ();
			
			//
			//	Check to see if this is a saved game or a level file.
			//
			int len = filename.Get_Length ();
			if (len >= 4 && ::lstrcmpi ((filename.Peek_Buffer () + (len - 4)), ".mix") != 0) {
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
	ListCtrlClass *list_ctrl,
	int				ctrl_id,
	int				item_index
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
	if (list_ctrl == NULL) {
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
		if (list_ctrl->Get_Entry_Data (item_index, 0) != NULL) {
			StringClass filename = ((StringClass *)list_ctrl->Get_Entry_Data (item_index, 1))->Peek_Buffer ();

			// Never delete .MIX files
			int len = filename.Get_Length ();
			if (len >= 4 && ::lstrcmpi ((filename.Peek_Buffer () + (len - 4)), ".mix") != 0) {

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
					if (::DeleteFile (filename) != 0) {
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
	if (_TheInstance == NULL) {
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
