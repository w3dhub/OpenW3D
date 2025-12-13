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
 *                     $Archive:: /Commando/Code/commando/dlgcontroltabs.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/09/01 1:45p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCONTROLTABS_H
#define __DLGCONTROLTABS_H


#include "dlgcontrolslisttab.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
//
//	ControlsBasicMvmtTabClass
//
////////////////////////////////////////////////////////////////
class ControlsBasicMvmtTabClass : public ControlsListTabClass
{
public:
	ControlsBasicMvmtTabClass (void);
};


////////////////////////////////////////////////////////////////
//
//	ControlsMultiPlayTabClass
//
////////////////////////////////////////////////////////////////
class ControlsMultiPlayTabClass : public ControlsListTabClass
{
public:
	ControlsMultiPlayTabClass (void);
};


////////////////////////////////////////////////////////////////
//
//	ControlsMiscTabClass
//
////////////////////////////////////////////////////////////////
class ControlsMiscTabClass : public ControlsListTabClass
{
public:
	ControlsMiscTabClass (void);

};


////////////////////////////////////////////////////////////////
//
//	ControlsLookTabClass
//
////////////////////////////////////////////////////////////////
class ControlsLookTabClass : public ControlsListTabClass
{
public:
	ControlsLookTabClass (void);

	void			On_Init_Dialog (void) override;
	bool			On_Apply (void) override;
	void			On_Reload (void) override;

private:
	void			Load_Controls (void);
};


////////////////////////////////////////////////////////////////
//
//	ControlsWeaponsTabClass
//
////////////////////////////////////////////////////////////////
class ControlsWeaponsTabClass : public ControlsListTabClass
{
public:
	ControlsWeaponsTabClass (void);
};


////////////////////////////////////////////////////////////////
//
//	ControlsAttackTabClass
//
////////////////////////////////////////////////////////////////
class ControlsAttackTabClass : public ControlsListTabClass
{
public:
	ControlsAttackTabClass (void);

	void			On_Init_Dialog (void) override;
	bool			On_Apply (void) override;
	void			On_Reload (void) override;

private:
	void			Load_Controls (void);
};


#endif //__DLGCONTROLTABS_H

