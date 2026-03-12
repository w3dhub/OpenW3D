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
 *                     $Archive:: /Commando/Code/Installer/CopyDialog.h $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 1/12/02 7:32p                 $*
 *                                                                                             *
 *                    $Revision:: 9                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef _COPY_DIALOG_H
#define _COPY_DIALOG_H

// Includes.
#include "CopyThread.h"
#include "InstallMenuDialog.h"
#include "Mutex.h"
#include "SafeTimer.h"
#include "Timer.h"


// Forward declarations.
class LightClass;
class HAnimClass;


// Dialog to give feedback to user about the state of the copying process.
class CopyDialogClass : public InstallMenuDialogClass
{
	public:
		 CopyDialogClass();
		~CopyDialogClass() {}

		// RTTI.
		void *As_CopyDialogClass()	{return (this);}

		bool Was_Successful()	{return (CopyThread->Get_Status() == CopyThreadClass::STATUS_SUCCESS);}

		void End_Dialog (void);
		void Callback (int id, PopupDialogClass *popup);
		void On_Command (int ctrl_id, int message_id, DWORD param);

	protected:

		void On_Init_Dialog (void);
		void On_Activate (bool onoff);
		void On_Frame_Update (void);
		void Add_Models();
		void Set_Model (unsigned modelindex);
		void Remove_Models();

		CopyThreadClass				   *CopyThread;			// A thread to copy files.
		RenderObjClass					   *SupplementalModel;	// Additional model to show in background.
		RenderObjClass					   *ProgressBarModel;
		HAnimClass						   *ProgressBarAnim;
		unsigned								 CurrentModel;			// Index of model to display.
		CDTimerClass <SafeTimerClass>	*CountdownTimer;
		TTimerClass <SafeTimerClass>	*FlashTimer;
		MessageBoxClass					*FileSystemErrorPopup;
		bool									 AddedModels;
};


#endif // _COPY_DIALOG_H
