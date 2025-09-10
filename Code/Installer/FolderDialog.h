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
 *                     $Archive:: /Commando/Code/Installer/FolderDialog.h $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/24/01 8:52p                $* 
 *                                                                                             * 
 *                    $Revision:: 5                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _FOLDER_DIALOG_H
#define _FOLDER_DIALOG_H

// Includes.
#include "InstallMenuDialog.h"


// Dialog to select the folder to which icons will be added. GameFolderClass specifies the
// folder for game icons, WOLFolderClass specifies the directory for online icons.

// Classes.
class FolderDialogClass : public InstallMenuDialogClass
{
	public:
		FolderDialogClass() : InstallMenuDialogClass (IDD_DIALOG_FOLDER) {}
		
		// RTTI.
		void *As_FolderDialogClass()		{return (this);}

		void			 On_ListCtrl_Sel_Change (ListCtrlClass *list_ctrl, int ctrl_id, int old_index, int new_index);
		void			 On_Command (int ctrl_id, int message_id, DWORD param);
		const wchar_t *Get_Folder (WideStringClass &folder);

	protected:
		void On_Init_Dialog (void);
};


class GameFolderDialogClass : public FolderDialogClass
{
	public:
		
		// RTTI.
		virtual void *As_GameFolderDialogClass()		{return (this);}

		void On_Command (int ctrl_id, int message_id, DWORD param) {FolderDialogClass::On_Command (ctrl_id, message_id, param);}

	protected:
		void On_Init_Dialog (void);
};


class WOLFolderDialogClass : public FolderDialogClass
{
	public:
		
		// RTTI.
		virtual void *As_WOLFolderDialogClass()		{return (this);}

		void On_Command (int ctrl_id, int message_id, DWORD param) {FolderDialogClass::On_Command (ctrl_id, message_id, param);}

	protected:
		void On_Init_Dialog (void);
};


#endif // _FOLDER_DIALOG_H
