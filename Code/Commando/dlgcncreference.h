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
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/dlgcncreference.h                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/24/02 2:06p                                                 $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCNCREFERENCE_H
#define __DLGCNCREFERENCE_H

#include "menudialog.h"
#include "DlgMessageBox.h"


//////////////////////////////////////////////////////////////////////
//
//	CnCReferenceMenuClass
//
//////////////////////////////////////////////////////////////////////
class CnCReferenceMenuClass : public MenuDialogClass, public Observer<DlgMsgBoxEvent>
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	CnCReferenceMenuClass  (void);
	~CnCReferenceMenuClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void);
	void		On_Destroy (void);
	void		On_Command (int ctrl_id, int mesage_id, DWORD param);
	void		On_Menu_Activate (bool onoff);
	void		On_Frame_Update(void);


	//
	//	Singleton access
	//
	static void								Display (void);
	static CnCReferenceMenuClass *	Get_Instance (void)	{ return _TheInstance; }

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////	
	void				Prompt_User (void);
	void				HandleNotification (DlgMsgBoxEvent &event);
	void				Exit_Game (void);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////	
	static CnCReferenceMenuClass *	_TheInstance;
	MenuBackDropClass *					OldBackdrop;
	float										Timer;

	enum				{ACTION_TIMEOUT_MS = 10000};
	static DWORD	LastSuicideTimeMs;
	static DWORD	LastChangeTeamTimeMs;
};


#endif //__DLGCNCREFERENCE_H
