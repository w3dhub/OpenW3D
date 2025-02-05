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
 *                     $Archive:: /Commando/Code/Commando/dlgsavegame.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/18/01 5:23p                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dlgsavegame.h"
#include "listctrl.h"
#include "dialogresource.h"
#include "gamedata.h"
#include "gamemode.h"
#include "gameinitmgr.h"
#include "savegame.h"
#include "ffactory.h"
#include "commandosaveload.h"
#include "translatedb.h"
#include "string_ids.h"
#include "editctrl.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	MBEVENT_OVERWRITE_PROMPT	= 1,
	MBEVENT_DELETE_PROMPT,
};

enum
{
	COL_DATE	= 0,
	COL_TIME,
	COL_NAME
};


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::On_Init_Dialog (void)
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
		list_ctrl->Add_Column (TRANSLATE (IDS_MENU_TEXT179), 0.5F, Vector3 (1, 1, 1));
	}

	//
	//	Limit the amount of text you can type into the edit control
	//
	EditCtrlClass *edit_ctrl = (EditCtrlClass *)Get_Dlg_Item (IDC_FILENAME_EDIT);
	if (edit_ctrl) {
		edit_ctrl->Set_Text_Limit (64);
	}
	
	//
	//	Populate the list
	//
	Reload_List (NULL);
	Update_Text_Field ();
	Update_Button_State ();

	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Column_Click
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::On_ListCtrl_Column_Click
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
SaveGameMenuClass::On_ListCtrl_Delete_Entry
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
		StringClass *filename	= (StringClass *)list_ctrl->Get_Entry_Data (item_index, 2);
		list_ctrl->Set_Entry_Data (item_index, 0, 0);
		list_ctrl->Set_Entry_Data (item_index, 2, 0);
		if (file_time != NULL) {
			delete file_time;
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
SaveGameMenuClass::LoadListSortCallback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param)
{
	int retval = 0;

	//
	//	Get the sorting params
	//
	int	sort_col_index = LOWORD (user_param);
	BOOL	sort_ascending	= HIWORD (user_param);

	if (list_ctrl->Get_Entry_Data (item_index1, 0) == NULL) {
		retval = -1;
	} else if (list_ctrl->Get_Entry_Data (item_index2, 0) == NULL) {
		retval = 1;
	} else {

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
	}


	return retval;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id)
	{
		case IDC_SAVE_GAME_BUTTON:
			Save_Game (true);
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
//	Save_Game
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::Save_Game (bool prompt)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LOAD_GAME_LIST_CTRL);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Don't save if the save button was disabled
	//
	if (Is_Dlg_Item_Enabled (IDC_SAVE_GAME_BUTTON) == false) {
		return ;
	}

	//
	//	Get the currently selected entry
	//
	int item_index = list_ctrl->Get_Curr_Sel ();
	if (item_index != -1) {

		//
		//	Determine what filename to save under
		//
		StringClass full_path;
		bool save_file = true;
		if (list_ctrl->Get_Entry_Data (item_index, 0) == NULL) {
			Get_Unique_Save_Filename (full_path);
		} else {
			StringClass filename = ((StringClass *)list_ctrl->Get_Entry_Data (item_index, 2))->Peek_Buffer ();

			//
			//	Build a full filename
			//
			full_path = "save\\";
			full_path += filename;

			//
			//	Display a message to the user asking if they really want to do this...
			//
			if (prompt) {
				DlgMsgBox::DoDialog (IDS_MENU_SAVE_OVERWRITE_TITLE, IDS_MENU_SAVE_OVERWRITE_MSG,
									DlgMsgBox::YesNo, this, MBEVENT_OVERWRITE_PROMPT);

				save_file = false;
			}
		}

		if (save_file) {

			if (Check_HD_Space ()) {

				//
				//	Save the game
				//
				SaveGameManager::Set_Description (Get_Dlg_Item_Text (IDC_FILENAME_EDIT));
				SaveGameManager::Save_Game( full_path, &_CommandoSaveLoad, NULL );

				//
				//	Quit out of the dialog
				//
				End_Dialog ();
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_DblClk
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::On_ListCtrl_DblClk
(
	ListCtrlClass *list_ctrl,
	int				ctrl_id,
	int				item_index
)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Unique_Save_Filename
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::Get_Unique_Save_Filename (StringClass &filename)
{
	int slot	= 1;
	bool done	= false;

	while (!done) {
		filename.Format ("save\\savegame%.2d.sav", slot ++);
		
		//
		//	Check to see if this file exists
		//
		FileClass *file= _TheWritingFileFactory->Get_File (filename);
		if ( file ) {
			file->Open ();
			done = !file->Is_Available ();
			_TheWritingFileFactory->Return_File (file);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Text_Field
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::Update_Text_Field (void)
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
	int curr_sel = list_ctrl->Get_Curr_Sel ();
	if (curr_sel != -1) {

		//
		//	Update the text field with the name of this save game
		//
		if (list_ctrl->Get_Entry_Data (curr_sel, 0) != NULL) {
			Set_Dlg_Item_Text (IDC_FILENAME_EDIT, list_ctrl->Get_Entry_Text (curr_sel, 2));
		} else {
			Set_Dlg_Item_Text (IDC_FILENAME_EDIT, L"");
		}

	} else {
		Set_Dlg_Item_Text (IDC_FILENAME_EDIT, L"");
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Sel_Change
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::On_ListCtrl_Sel_Change
(
	ListCtrlClass *list_ctrl,
	int				ctrl_id,
	int				old_index,
	int				new_index
)
{
	Update_Text_Field ();
	Update_Button_State ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Delete_Game
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::Delete_Game (bool prompt)
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
			StringClass filename = ((StringClass *)list_ctrl->Get_Entry_Data (item_index, 2))->Peek_Buffer ();

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
				//	Build a full path from which to delete the file
				//
				StringClass full_path = "data\\save\\";
				full_path += filename;
				
				//
				//	Delete the file and remove its entry from the list
				//
				if (::DeleteFile (full_path) != 0) {
					list_ctrl->Delete_Entry (item_index);
					Update_Text_Field ();
					Update_Button_State ();
				}
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reload_List
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::Reload_List (const char *current_filename)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LOAD_GAME_LIST_CTRL);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Start fresh
	//
	list_ctrl->Delete_All_Entries ();

	//
	//	Create an empty slot entry
	//
	int item_index = list_ctrl->Insert_Entry (0, L"");
	if (item_index >= 0) {
		list_ctrl->Set_Entry_Text (item_index, 2, TRANSLATE (IDS_MENU_EMPTY_SLOT));
		list_ctrl->Set_Curr_Sel (item_index);
	}

	WIN32_FIND_DATA find_info	= { 0 };
	BOOL keep_going				= TRUE;
	HANDLE file_find				= NULL;

	//
	//	Build a list of all the saved games we know about
	//
	int index = 1;
	for (file_find = ::FindFirstFile ("data\\save\\*.sav", &find_info);
		 (file_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFile (file_find, &find_info))
	{
		//
		//	Get the user description and map name from the file
		//
		WideStringClass description;
		WideStringClass map_name;
		SaveGameManager::Peek_Description (find_info.cFileName, description, map_name);
				
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
			
			list_ctrl->Set_Entry_Data (item_index, 0, (uint32)new FILETIME(local_time));
			list_ctrl->Set_Entry_Data (item_index, 2, (uint32)new StringClass(find_info.cFileName));

			//
			//	Select this entry if its the default
			//
			if (	current_filename != NULL && 
					::lstrcmpi (current_filename, find_info.cFileName) == 0)
			{
				list_ctrl->Set_Curr_Sel (item_index);
			}
		}
	}

	if (file_find != INVALID_HANDLE_VALUE) {			  
		::FindClose (file_find); 
	}

	//
	//	Sort the list
	//
	list_ctrl->Sort (LoadListSortCallback, MAKELONG (CurrSortCol, IsSortAscending));		

	//
	//	Update the sort marker
	//
	ListCtrlClass::SORT_TYPE type = IsSortAscending ? ListCtrlClass::SORT_ASCENDING : ListCtrlClass::SORT_DESCENDING;
	list_ctrl->Set_Sort_Designator (CurrSortCol, type);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	HandleNotification
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::HandleNotification (DlgMsgBoxEvent &event)
{
	if (event.Get_User_Data () == MBEVENT_OVERWRITE_PROMPT) {

		//
		//	The user has confirmed the overwrite, so save the game
		//
		if (event.Event () == DlgMsgBoxEvent::Yes) {
			Save_Game (false);
		}

	} else if (event.Get_User_Data () == MBEVENT_DELETE_PROMPT) {
		
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
//	Update_Button_State
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::Update_Button_State (void)
{
	WideStringClass text = Get_Dlg_Item_Text (IDC_FILENAME_EDIT);
	bool enable = (text.Get_Length () > 0);

	//
	//	Enable/disable the save game button as necessary
	//
	Enable_Dlg_Item (IDC_SAVE_GAME_BUTTON, enable);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_EditCtrl_Change
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::On_EditCtrl_Change (EditCtrlClass *edit_ctrl, int ctrl_id)
{
	if (ctrl_id == IDC_FILENAME_EDIT) {
		Update_Button_State ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_EditCtrl_Enter_Pressed
//
////////////////////////////////////////////////////////////////
void
SaveGameMenuClass::On_EditCtrl_Enter_Pressed (EditCtrlClass *edit_ctrl, int ctrl_id)
{
	if (ctrl_id == IDC_FILENAME_EDIT) {
		Save_Game (true);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Check_HD_Space
//
////////////////////////////////////////////////////////////////
bool
SaveGameMenuClass::Check_HD_Space (void)
{
	bool retval = true;

	ULARGE_INTEGER freebytecount;		// Free bytes on disk available to caller (caller may not have access to entire disk).
	ULARGE_INTEGER totalbytecount;	// Total bytes on disk.
	StringClass		kernelpathname;
	__int64			diskspace;

	int (__stdcall *getfreediskspaceex) (LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);

	//	Get the free disk space on the drive.
	// NOTE IML: For Win'95, must query for support for GetDiskFreeSpaceEx before using it - otherwise use GetDiskFreeSpace().
	GetSystemDirectory (kernelpathname.Get_Buffer (_MAX_PATH), _MAX_PATH);
	kernelpathname += "\\";
	kernelpathname += "Kernel32.dll";
	getfreediskspaceex = (int (_stdcall*) (LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER)) GetProcAddress (GetModuleHandle (kernelpathname.Peek_Buffer()), "GetDiskFreeSpaceExA");
	if (getfreediskspaceex != NULL) {

		if (!getfreediskspaceex (NULL, &freebytecount, &totalbytecount, NULL)) return (false);
	
		// Convert to a 64-bit integer.
		diskspace = freebytecount.QuadPart;
	
	} else {

		DWORD sectorspercluster, bytespersector, freeclustercount, totalclustercount;
		
		// The Ex version is not available. Use the Win'95 version.
		// QUESTION: SDK docs say that values returned by this function are erroneous if partition > 2Gb.
		//				 Does that mean that the partition is guaranteed to be <= 2Gb if Ex is not available?
		if (!GetDiskFreeSpace (NULL, &sectorspercluster, &bytespersector, &freeclustercount, &totalclustercount)) return (false); 
		diskspace = sectorspercluster * bytespersector * freeclustercount;
	}
	
	//
	//	Is there at least 2 megs of disk space available?
	//
	const __int64 TWO_MEGS = (1024 * 1024 * 2);
	if (diskspace < TWO_MEGS) {

		//
		//	Let the user know that they can't save because of lack of disk space
		//
		DlgMsgBox::DoDialog (IDS_MENU_SAVE_NO_DISK_SPACE_TITLE, IDS_MENU_SAVE_NO_DISK_SPACE_MSG);
		retval = false;
	}

	return retval;
}
