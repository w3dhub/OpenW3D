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
 *                     $Archive:: /Commando/Code/Installer/DirectoryDialog.cpp $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 12/13/01 5:46p                $* 
 *                                                                                             * 
 *                    $Revision:: 8                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "DirectoryDialog.h"
#include "dialogbase.h"
#include "DialogText.h"
#include "DirectoryBrowser.h"
#include "ErrorHandler.h"
#include "RegistryManager.h"
#include "resource.h"
#include "Installer.h"
#include "Translator.h"


/***********************************************************************************************
 * DirectoryDialogClass::Update --																				  *
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
void DirectoryDialogClass::Update (bool lazyupdate)
{
	WideStringClass	directorypath (Get_Dlg_Item_Text (IDC_DIRECTORY_EDIT));
	TxWideStringClass spaceneededstring (IDS_DISK_SPACE_NEEDED);
	WideStringClass	sizestring;
	int64_t				spaceavailable;

	if (!lazyupdate) {

		// Update static text control with disk space needed.
		spaceneededstring += Megabyte_Format (Get_Disk_Space_Needed(), sizestring);
		Set_Dlg_Item_Text	(IDC_DIRECTORY_STATIC2, spaceneededstring);
	}

	// Lazily update disk space available.
	if (directorypath.Get_Length() >= 2) {
		if (directorypath [1] == L':') {
			if (!lazyupdate || (DriveLetter != directorypath [0])) {
				if (Get_Disk_Space_Available (directorypath, spaceavailable)) {

					TxWideStringClass spaceavailablestring (IDS_DISK_SPACE_AVAILABLE);

					spaceavailablestring += Megabyte_Format (spaceavailable, sizestring);
					Set_Dlg_Item_Text	(IDC_DIRECTORY_STATIC3, spaceavailablestring);
					DriveLetter = directorypath [0];
				} else {
					Set_Dlg_Item_Text	(IDC_DIRECTORY_STATIC3, L"");
					DriveLetter = '\0';
				}
			}
		} else {
			Set_Dlg_Item_Text	(IDC_DIRECTORY_STATIC3, L"");
			DriveLetter = '\0';
		}
	} else {
		Set_Dlg_Item_Text	(IDC_DIRECTORY_STATIC3, L"");
		DriveLetter = '\0';
	}
}


/***********************************************************************************************
 * DirectoryDialogClass::Megabyte_Format --																	  *
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
WCHAR *DirectoryDialogClass::Megabyte_Format (int64_t bytecount, WideStringClass &outputstring)
{
	char	 buffer [32];
	double size;
	
	// Output byte count in Megabytes.
	buffer [sizeof (buffer) - 1] = '\0';
	size = ((double) bytecount) / ((double)(1024 * 1024));
	_snprintf (buffer, sizeof (buffer) - 1, " %.2f ", size);
	
	outputstring  = buffer;
	outputstring += TxWideStringClass (IDS_MEGABYTE_SYMBOL);

	return (outputstring.Peek_Buffer());
}


/***********************************************************************************************
 * DirectoryDialogClass::Callback --																			  *
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
void DirectoryDialogClass::Callback (int id, PopupDialogClass *popup)
{
	// Is this callback from the browser?
	if (popup == Browser) {

		switch (id) {
			
			case IDOK:
			{	
				WideStringClass path;

				// Update edit control with selected path from browser.
				Browser->Get_Selected_Path (path);
				Set_Dlg_Item_Text (IDC_DIRECTORY_EDIT, path);
				Update();
				break;
			}

			case IDCANCEL:

				// Do nothing.
				break;

			default:
				break;
		}
	
		REF_PTR_RELEASE (Browser);
	
	} else {

		if (popup == OverwriteDialog) {

			switch (id) {

				case IDC_BUTTON_YES:
					On_Command (IDC_BUTTON_YES, 0, 0);
					REF_PTR_RELEASE (OverwriteDialog);
					break;

				default:
					REF_PTR_RELEASE (OverwriteDialog);
					break;
			}

		} else {

			// Callback is from some other source. Let the base class handle it.
			InstallMenuDialogClass::Callback (id, popup);
		}
	}
}


/***********************************************************************************************
 * DirectoryDialogClass::On_Activate --																		  *
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
void DirectoryDialogClass::On_Activate (bool onoff)
{
	// NOTE 0: Disk space needed/available may have changed since this dialog was last active. 
	//	NOTE 1: It is important to ensure that the dialog is running before Update() is called 
	//			  (this indicates that the dialog has been initialized by the framework).
	if (onoff && Is_Running()) {
		Update();
	}
	
	InstallMenuDialogClass::On_Activate (onoff);
}


/***********************************************************************************************
 * DirectoryDialogClass::On_Frame_Update --																	  *
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
void DirectoryDialogClass::On_Frame_Update (void)
{
	Update (true);

	InstallMenuDialogClass::On_Frame_Update();
}


/***********************************************************************************************
 * DirectoryDialogClass::On_Command --																			  *
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
void DirectoryDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id)
	{
		case IDC_BUTTON_BROWSE:
		{
			Browser = NEW_REF (DirectoryBrowserClass, (this));
			Browser->Start_Dialog();					
			break;
		}

		case IDOK:
		{	
			WideStringClass path (Get_Dlg_Item_Text (IDC_DIRECTORY_EDIT));
			int				 errormessageid;				
			int64_t			 spaceavailable;

			// Check that the selected path is valid.
			if (!Validate_Path (path, errormessageid)) {
				MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (errormessageid), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
				return;
			}

			// Check that the selected path is not a Windows system path.
			if (Is_System_Directory (path)) {
				MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_SYSTEM_DIRECTORY), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
				return;
			}

			// Check that there is enough disk space on the selected drive.
			if (!Get_Disk_Space_Available (path, spaceavailable)) {
				MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_INVALID_PATH), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
				return;
			}
			if (Get_Disk_Space_Needed() > spaceavailable) {
				MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_NOT_ENOUGH_DISK_SPACE), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
				return;
			}

			// If the path exists ask the user if he wishes to overwrite it?
			if (Directory_Exists (path)) {
				OverwriteDialog = MessageBoxClass::Create_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_DIRECTORY_EXISTS), MessageBoxClass::MESSAGE_BOX_TYPE_YES_NO, this);
				return;
			}
			break;
		}

		case IDC_BUTTON_YES:
			
			// NOTE: This command has come from the Overwrite dialog via the callback routine.
			InstallMenuDialogClass::On_Command (IDOK, message_id, param);
			return;

		default:
			break;
	}

	InstallMenuDialogClass::On_Command (ctrl_id, message_id, param);
}


/***********************************************************************************************
 * DirectoryDialogClass::Get_Path --																			  *
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
const WCHAR *DirectoryDialogClass::Get_Path (WideStringClass &path)
{																			  
	path = Get_Dlg_Item_Text (IDC_DIRECTORY_EDIT);
	return (path);
}


/***********************************************************************************************
 * GameDirectoryDialogClass::On_Init_Dialog --																  *
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
void GameDirectoryDialogClass::On_Init_Dialog()
{
	WideStringClass	path;	
	TxWideStringClass spaceneededstring (IDS_DISK_SPACE_NEEDED);

	Set_Dlg_Item_Text	(IDC_DIRECTORY_STATIC1, TxWideStringClass (IDS_INSTALL_GAME_DIRECTORY));

	// Extract game path from registry (if it exists) - otherwise use a default.
	if (_RegistryManager.Get_Target_Game_Path (path)) {
		Set_Dlg_Item_Text (IDC_DIRECTORY_EDIT, path);
	} else {
		Set_Dlg_Item_Text (IDC_DIRECTORY_EDIT, TxWideStringClass (IDS_DEFAULT_GAME_PATH, IDS_RESOURCE_DEFAULT_GAME_PATH));
	}
	Update();

	InstallMenuDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * WOLDirectoryDialogClass::On_Init_Dialog --																  *
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
void WOLDirectoryDialogClass::On_Init_Dialog()
{
	WideStringClass	path;
	TxWideStringClass spaceneededstring (IDS_DISK_SPACE_NEEDED);
	
	Set_Dlg_Item_Text	(IDC_DIRECTORY_STATIC1, TxWideStringClass (IDS_INSTALL_WOL_DIRECTORY));

	// Extract WOL path from registry (if it exists) - otherwise use a default.
	if (_RegistryManager.Get_Target_WOL_Path (RegistryManagerClass::WOLAPI_COMPONENT, path)) {
		Set_Dlg_Item_Text (IDC_DIRECTORY_EDIT, path);
	} else {
		Set_Dlg_Item_Text (IDC_DIRECTORY_EDIT, TxWideStringClass (IDS_DEFAULT_WOL_PATH, IDS_RESOURCE_DEFAULT_WOL_PATH));
	}
	Update();

	InstallMenuDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * WOLDirectoryDialogClass::On_Command --																		  *
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
void WOLDirectoryDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id)
	{
		case IDOK:
		{	
			if (_Installer.Install_Game()) {

				WideStringClass gamepath;

				// Check that the selected WOL path is not the same as the target game path.
				if (Is_Same_Path (WideStringClass (Get_Dlg_Item_Text (IDC_DIRECTORY_EDIT)), _Installer.Get_Target_Game_Path (gamepath))) {
					MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_WARNING), TxWideStringClass (IDS_SAME_PATHS), MessageBoxClass::MESSAGE_BOX_TYPE_OK, this);
					return;
				}
			}
			break;
		}

		default:
			break;
	}

	DirectoryDialogClass::On_Command (ctrl_id, message_id, param);
}

