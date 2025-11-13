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
 *                     $Archive:: /Commando/Code/commando/dlgcontrols.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/17/01 5:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCONTROLS_H
#define __DLGCONTROLS_H


#include "menudialog.h"


////////////////////////////////////////////////////////////////
//
//	ControlsMenuClass
//
////////////////////////////////////////////////////////////////
class ControlsMenuClass : public MenuDialogClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructor
	//////////////////////////////////////////////////////////////
	ControlsMenuClass (void);	
	~ControlsMenuClass (void);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	
	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Command (int ctrl_id, int message_id, unsigned int param) override;

	//
	//	Configuration support
	//
	void		Apply_Changes (void);
	void		Reload (void);

	//
	//	Singleton instance access
	//
	static ControlsMenuClass *	Get_Instance (void)	{ return _TheInstance; }

private:

	//////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////
	static ControlsMenuClass *	_TheInstance;
};


#endif //__DLGCONTROLS_H

