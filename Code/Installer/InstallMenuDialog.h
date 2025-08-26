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
 *                     $Archive:: /Commando/Code/Installer/InstallMenuDialog. $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 12/04/01 4:43p                $* 
 *                                                                                             * 
 *                    $Revision:: 5                     $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _INSTALL_MENU_DIALOG_H
#define _INSTALL_MENU_DIALOG_H

// Includes.
#include "menudialog.h"
#include "resource.h"
#include "MessageBox.h"


// Classes.
class InstallMenuDialogClass : public CallbackMenuDialogClass
{
	public:
		 InstallMenuDialogClass (int resourceid);
		~InstallMenuDialogClass();

		// RTTI.
		virtual void *As_CopyDialogClass()					{return (NULL);}
		virtual void *As_DirectoryDialogClass()			{return (NULL);}
		virtual void *As_FinalDialogClass()					{return (NULL);}
		virtual void *As_GameFolderDialogClass()			{return (NULL);}
		virtual void *As_GameDirectoryDialogClass()		{return (NULL);}
		virtual void *As_LicenseDialogClass()				{return (NULL);}
		virtual void *As_NDADialogClass()					{return (NULL);}
		virtual void *As_WOLDirectoryDialogClass()		{return (NULL);}
		virtual void *As_WOLFolderDialogClass()			{return (NULL);}
		virtual void *As_ReviewDialogClass()				{return (NULL);}
		virtual void *As_SerialDialogClass()				{return (NULL);}
		virtual void *As_WelcomeDialogClass()				{return (NULL);}
		virtual void *As_WhatToInstallDialogClass()		{return (NULL);}
		virtual void *As_WOL1DialogClass()					{return (NULL);}
		virtual void *As_WOL2DialogClass()					{return (NULL);}

		void On_Command (int ctrl_id, int message_id, DWORD param);
};


#endif // _INSTALL_MENU_DIALOG_H