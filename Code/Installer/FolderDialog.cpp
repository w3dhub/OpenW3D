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
 *                 Project Name : Installer                                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Installer/FolderDialog.cpp $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 12/13/01 5:46p                $* 
 *                                                                                             * 
 *                    $Revision:: 9                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "FolderDialog.h"
#include "ErrorHandler.h"
#include "DialogText.h"
#include "Installer.h"
#include "listctrl.h"
#include	"RegistryManager.h"
#include "resource.h"
#include "Translator.h"
#include <io.h>


/***********************************************************************************************
 * FolderDialogClass::On_Init_Dialog --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void FolderDialogClass::On_Init_Dialog (void)
{
	WideStringClass wildcardfolderpath;

	// Set-up the folder list control with available folders.
	if (_RegistryManager.Get_Folder_Path (wildcardfolderpath)) {

		const WCHAR *wildcardname = L"*.*";

		ListCtrlClass  *listctrl;
		unsigned			 i;
		WIN32_FIND_DATA finddata;	
		HANDLE			 handle;
		bool				 done = false;			

		wildcardfolderpath += L"\\";
		wildcardfolderpath += wildcardname;

		StringClass multibytewildcardfolderpath (wildcardfolderpath);

		// Add a list item for each subdirectory.
		listctrl = Get_Dlg_Item (IDC_FOLDER_LIST)->As_ListCtrlClass();
		listctrl->Add_Column (L"", 1.0f, Vector3 (1.0f, 1.0f, 1.0f));
		i = 0;
		handle = FindFirstFile (multibytewildcardfolderpath, &finddata);
		if (handle != INVALID_HANDLE_VALUE) {
			while (!done) {

				WideStringClass filename (finddata.cFileName);

				if ((filename [0] != L'.') && (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					listctrl->Insert_Entry (i, filename);
				}
				if (done = FindNextFile (handle, &finddata) == 0) {
					if (GetLastError() != ERROR_NO_MORE_FILES) FATAL_SYSTEM_ERROR;
				}
			
				i++;
			}
			if (!FindClose (handle)) FATAL_SYSTEM_ERROR;
	
			listctrl->Sort_Alphabetically (0, ListCtrlClass::SORT_ASCENDING);
		}
	}
	
	InstallMenuDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * FolderDialogClass::Get_Folder --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
const WCHAR *FolderDialogClass::Get_Folder (WideStringClass &folder)
{														  
	folder = Get_Dlg_Item_Text (IDC_FOLDER_EDIT);
	return (folder);
}


/***********************************************************************************************
 * GameFolderDialogClass::On_Init_Dialog --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void GameFolderDialogClass::On_Init_Dialog (void)
{
	WideStringClass folder;

	// Initialize strings.
	Set_Dlg_Item_Text (IDC_FOLDER_STATIC1, TxWideStringClass (IDS_SELECT_GAME_FOLDER));
	
	// Extract game folder from registry (if it exists) - otherwise use a default.
	if (_RegistryManager.Get_Target_Game_Folder (folder)) {
		Set_Dlg_Item_Text (IDC_FOLDER_EDIT, folder);
	} else {
		Set_Dlg_Item_Text (IDC_FOLDER_EDIT, TxWideStringClass (IDS_DEFAULT_GAME_FOLDER, IDS_RESOURCE_DEFAULT_GAME_FOLDER));
	}
	
	FolderDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * WOLFolderDialogClass::On_Init_Dialog --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void WOLFolderDialogClass::On_Init_Dialog (void)
{
	WideStringClass folder;

	// Initialize strings.
	Set_Dlg_Item_Text (IDC_FOLDER_STATIC1, TxWideStringClass (IDS_SELECT_WOL_FOLDER));
	
	// Extract game folder from registry (if it exists) - otherwise use a default.
	if (_RegistryManager.Get_Target_WOL_Folder (RegistryManagerClass::WOLAPI_COMPONENT, folder)) {
		Set_Dlg_Item_Text (IDC_FOLDER_EDIT, folder);
	} else {
		Set_Dlg_Item_Text (IDC_FOLDER_EDIT, TxWideStringClass (IDS_DEFAULT_WOL_FOLDER, IDS_RESOURCE_DEFAULT_WOL_FOLDER));
	}

	FolderDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * FolderDialogClass::On_ListCtrl_Sel_Change --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void FolderDialogClass::On_ListCtrl_Sel_Change (ListCtrlClass *list_ctrl, int ctrl_id, int old_index, int new_index)
{
	Set_Dlg_Item_Text (IDC_FOLDER_EDIT, list_ctrl->Get_Entry_Text (new_index, 0));	
}


/***********************************************************************************************
 * FolderDialogClass::On_Command --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                * 
 *=============================================================================================*/
void FolderDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id) {

		case IDOK:
		{	
			WideStringClass foldername (Get_Dlg_Item_Text (IDC_FOLDER_EDIT));
			WideStringClass folderpathname;
			bool				 hasnonspacechar;	
			int				 dummyid;				

			// Check that the selected folder name has non-space characters.
			hasnonspacechar = false;
			for (int i = 0; i < foldername.Get_Length(); i++) {
				if (foldername [i] != L' ') hasnonspacechar = true;
			}
			if (!hasnonspacechar) {
				MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_INVALID_FOLDER_NAME), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
				return;
			}

			// Check that the selected folder path is valid.
			_RegistryManager.Get_Folder_Path (folderpathname);
			folderpathname += L"\\";
			folderpathname += Get_Dlg_Item_Text (IDC_FOLDER_EDIT);
			if (!Validate_Path (folderpathname, dummyid)) {
				MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_INVALID_FOLDER_NAME), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
				return;
			}
		}

		default:
			break;
	}

	InstallMenuDialogClass::On_Command (ctrl_id, message_id, param);
}

