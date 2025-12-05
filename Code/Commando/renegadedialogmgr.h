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
 *                     $Archive:: /Commando/Code/commando/renegadedialogmgr.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/13/02 1:58p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __RENEGADE_DIALOG_MGR_H
#define __RENEGADE_DIALOG_MGR_H

#include "dialogfactory.h"
#include "renegadedialog.h"
#include "wwuiinput.h"
#include "win.h"


////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////
int MyLoadStringW (UINT str_id, LPWSTR buffer, int buffer_len);


////////////////////////////////////////////////////////////////
//	Macros
////////////////////////////////////////////////////////////////
/*#define	LOAD_STRING(strobj, strid)	\
	::MyLoadStringW (strid, strobj.Get_Buffer (64), 64);*/


////////////////////////////////////////////////////////////////
//	Constants and globals
////////////////////////////////////////////////////////////////
const int								FACTORY_COUNT	= (DIALOG_LINK_LAST - DIALOG_LINK_FIRST);
extern DialogFactoryBaseClass *	FactoryArray[FACTORY_COUNT];
extern WWUIInputClass *				_TheWWUIInput;


////////////////////////////////////////////////////////////////
//
//	RenegadeDialogMgrClass
//
////////////////////////////////////////////////////////////////
class RenegadeDialogMgrClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		LOC_MAIN_MENU				= 0,
		LOC_INTERNET_MAIN,
		LOC_INTERNET_GAME_LIST,
		LOC_LAN_MAIN,
		LOC_ENCYCLOPEDIA,
		LOC_OBJECTIVES,
		LOC_MAP,
		LOC_CNC_REFERENCE,
		LOC_LOAD_GAME,
		LOC_IN_GAME_HELP,
		LOC_GAMESPY_MAIN,
		LOC_SPLASH_IN,
		LOC_SPLASH_OUT,
	}	LOCATION;

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	
	//
	//	Initialization
	//
	static void		Initialize (void);
	static void		Shutdown (void);

	//
	//	Dialog creation
	//
	static void		Do_Dialog_By_Button_ID (int button_id);

	//
	//	Dialog creation
	//
	static void		Do_Simple_Dialog (const DialogResource *dialog_resource);

	//
	//	Menu traversal access
	//
	static void		Goto_Location (LOCATION location);
};


////////////////////////////////////////////////////////////////
//	Inlines
////////////////////////////////////////////////////////////////
inline void
RenegadeDialogMgrClass::Do_Dialog_By_Button_ID (int button_id)
{
	WWASSERT (button_id >= DIALOG_LINK_FIRST && button_id <= DIALOG_LINK_LAST);
	
	//
	//	Start the dialog
	//
	FactoryArray[button_id - DIALOG_LINK_FIRST]->Do_Dialog ();
	return ;
}


#endif //__RENEGADE_DIALOG_MGR_H

