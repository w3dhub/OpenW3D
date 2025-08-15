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
 *                     $Archive:: /Commando/Code/Installer/DirectoryDialog.h $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 11/24/01 8:38p                $* 
 *                                                                                             * 
 *                    $Revision:: 5                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _DIRECTORY_DIALOG_H
#define _DIRECTORY_DIALOG_H

// Includes.
#include "InstallMenuDialog.h"
#include "Installer.h"


// Forward delcarations.
class DirectoryBrowserClass;


// Dialog to set the install directory for the game (GameDirectoryClass) or an
// online component (WOLDirectoryClass).

// Classes.
class DirectoryDialogClass : public InstallMenuDialogClass
{
	public:
		DirectoryDialogClass() : InstallMenuDialogClass (IDD_DIALOG_DIRECTORY), Browser (NULL), OverwriteDialog (NULL), DriveLetter ('\0') {}
		
		// RTTI.
		void *As_DirectoryDialogClass()		{return (this);}

		void			 On_Command (int ctrl_id, int message_id, DWORD param);
		void			 Callback (int id, PopupDialogClass *popup);
		const WCHAR *Get_Path (WideStringClass &path);

	protected:

		void	 Update (bool lazyupdate = false);
		
		void	 On_Init_Dialog (void)		{InstallMenuDialogClass::On_Init_Dialog();}
		void	 On_Activate (bool onoff);
		void	 On_Frame_Update (void);

		WCHAR *Megabyte_Format (int64_t bytecount, WideStringClass &outputstring);

		virtual int64_t Get_Disk_Space_Needed() = 0;
		virtual bool	 Get_Disk_Space_Available (const WideStringClass &path, int64_t &diskspace) = 0;

		DirectoryBrowserClass *Browser;
		MessageBoxClass		 *OverwriteDialog;
		WCHAR						  DriveLetter;	
};


class GameDirectoryDialogClass : public DirectoryDialogClass
{
	public:
		
		// RTTI.
		virtual void *As_GameDirectoryDialogClass()		{return (this);}

		void On_Command (int ctrl_id, int message_id, DWORD param) {DirectoryDialogClass::On_Command (ctrl_id, message_id, param);}

	protected:

		void	  On_Init_Dialog (void);
		int64_t Get_Disk_Space_Needed()																	  {return (_Installer.Get_Game_Size (true));}
		bool	  Get_Disk_Space_Available (const WideStringClass &path, int64_t &diskspace) {return (_Installer.Get_Game_Space_Available (path, diskspace));}
};


class WOLDirectoryDialogClass : public DirectoryDialogClass
{
	public:
		
		// RTTI.
		virtual void *As_WOLDirectoryDialogClass()		{return (this);}

		void On_Command (int ctrl_id, int message_id, DWORD param);

	protected:
		
		void	  On_Init_Dialog (void);
		int64_t Get_Disk_Space_Needed()																	  {return (_Installer.Get_WOL_Size (true));}
		bool	  Get_Disk_Space_Available (const WideStringClass &path, int64_t &diskspace) {return (_Installer.Get_WOL_Space_Available (path, diskspace));}
};


#endif // _DIRECTORY_DIALOG_H