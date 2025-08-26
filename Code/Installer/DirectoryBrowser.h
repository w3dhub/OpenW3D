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
 *                     $Archive:: /Commando/Code/Installer/DirectoryBrowser.h $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/19/01 9:53p                $* 
 *                                                                                             * 
 *                    $Revision:: 4                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _DIRECTORY_BROWSER_H
#define _DIRECTORY_BROWSER_H

// Includes
#include "MessageBox.h"
#include "resource.h"


// Popup dialog to implement directory browsing and selection.

class DirectoryBrowserClass : public MessageBoxClass {

	public:
	
		DirectoryBrowserClass (CallbackMenuDialogClass *callbackobject) : MessageBoxClass (IDD_DIALOG_BROWSER, callbackobject) {}

		void On_TreeCtrl_Needs_Children (TreeCtrlClass *tree_ctrl, int ctrl_id, TreeItemClass *parent_item);
		void Get_Selected_Path (WideStringClass &selectedpath);

	protected:

		void On_Init_Dialog (void);

		bool Has_Children (TreeCtrlClass *tree_ctrl, int ctrl_id, TreeItemClass *parent_item);
		void Build_Pathname (TreeItemClass *treenode, WideStringClass &path);
		bool Add_Folders (bool recurse, const WideStringClass &path, TreeCtrlClass *treectrl, TreeItemClass *treenode);
};


#endif // DIRECTORY_BROWSER
