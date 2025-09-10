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
 *                     $Archive:: /Commando/Code/Installer/DirectoryBrowser.cpp $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/25/01 11:12p               $* 
 *                                                                                             * 
 *                    $Revision:: 6                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "DirectoryBrowser.h"
#include "ErrorHandler.h"
#include "TreeCtrl.h"
#include <io.h>


/***********************************************************************************************
 * DirectoryBrowserClass::Get_Selected_Path --																  *
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
void DirectoryBrowserClass::Get_Selected_Path (WideStringClass &selectedpath)
{
	TreeCtrlClass *treectrl;

	treectrl = Get_Dlg_Item (IDC_BROWSER_TREE)->As_TreeCtrlClass();
	if (treectrl->Get_Selected_Item() != NULL) {
		Build_Pathname (treectrl->Get_Selected_Item(), selectedpath);
	} else {
		selectedpath = L"???";
	}
}


/***********************************************************************************************
 * BrowserDialogClass::On_Init_Dialog --																		  *
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
void DirectoryBrowserClass::On_Init_Dialog (void)
{
	TreeCtrlClass *treectrl;

	char				drivename [] = "?:";

	treectrl = Get_Dlg_Item (IDC_BROWSER_TREE)->As_TreeCtrlClass();

	// Make an entry at the root of the tree control for every fixed drive.
	for (char r = 'A'; r <= 'Z'; r++) {
			
		drivename [0] = r;

		// If the drive is a fixed hard disc...
		if (GetDriveType (drivename) == DRIVE_FIXED) {

			TreeItemClass *treeitem;
			
			treeitem = treectrl->Insert_Item (WideStringClass (drivename), TreeCtrlClass::ICON_FOLDER, TreeCtrlClass::ICON_FOLDER_OPEN, NULL);
			treeitem->Set_Needs_Children (Has_Children (treectrl, treectrl->Get_ID(), treeitem));
		}
	}

	treectrl->Sort_Children_Alphabetically (NULL);
	
	// Must call base class.
	PopupDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * DirectoryBrowserClass::On_TreeCtrl_Needs_Children --													  *
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
void DirectoryBrowserClass::On_TreeCtrl_Needs_Children (TreeCtrlClass *tree_ctrl, int ctrl_id, TreeItemClass *parent_item)
{
	if (parent_item != NULL) {

		WideStringClass path;
	
		Build_Pathname (parent_item, path);
		Add_Folders (true, path, tree_ctrl, parent_item);
		tree_ctrl->Sort_Children_Alphabetically (parent_item);
		parent_item->Set_Needs_Children (false);
	}
}


/***********************************************************************************************
 * DirectoryBrowserClass::Has_Children --																		  *
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
bool DirectoryBrowserClass::Has_Children (TreeCtrlClass *tree_ctrl, int ctrl_id, TreeItemClass *parent_item)
{
	bool haschildren = true;

	if (parent_item != NULL) {

		WideStringClass path;
	
		Build_Pathname (parent_item, path);
		haschildren = Add_Folders (false, path, tree_ctrl, parent_item); 	
	}

	return (haschildren);
}


/***********************************************************************************************
 * DirectoryBrowserClass::Build_Pathname --																	  *
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
void DirectoryBrowserClass::Build_Pathname (TreeItemClass *treenode, WideStringClass &path)
{
	WideStringClass backslash ("\\");

	if (treenode->Get_Parent() != NULL) {
		Build_Pathname (treenode->Get_Parent(), path);
	}

	if (path.Get_Length() > 0) {
		path += backslash;
	}

	// Append the tree node's name whilst recursively unwinding.
	path += treenode->Get_Name();
}


/***********************************************************************************************
 * DirectoryBrowserClass::Add_Folders --																		  *
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
bool DirectoryBrowserClass::Add_Folders (bool recurse, const WideStringClass &path, TreeCtrlClass *treectrl, TreeItemClass *treenode)
{
	const wchar_t *wildcardname = L"*.*";

	WideStringClass pathname (path);
	StringClass		 multibytepathname;	
	WIN32_FIND_DATA finddata;	
	HANDLE			 handle;
	bool				 done = false;			
	bool				 haschildren = false;	

	// Add a child node for each subdirectory.
	pathname += L"\\";
	pathname += wildcardname;
	multibytepathname = pathname;
	handle = FindFirstFile (multibytepathname, &finddata);
	if (handle != INVALID_HANDLE_VALUE) {

		while (!done) {

			WideStringClass filename (finddata.cFileName);

			if ((filename [0] != L'.') && (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

				TreeItemClass *treechild;
	
				haschildren = true;
				if (!recurse) break;

				treechild = treectrl->Insert_Item (filename, TreeCtrlClass::ICON_FOLDER, TreeCtrlClass::ICON_FOLDER_OPEN, treenode);
				treechild->Set_Needs_Children (Has_Children (treectrl, treectrl->Get_ID(), treechild));
			}
			if (done = FindNextFile (handle, &finddata) == 0) {
				if (GetLastError() != ERROR_NO_MORE_FILES) FATAL_SYSTEM_ERROR;
			}	
		}
		if (!FindClose (handle)) FATAL_SYSTEM_ERROR;
	}

	// Does this node have any children?
	return (haschildren);
}