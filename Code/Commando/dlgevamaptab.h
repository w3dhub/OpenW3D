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
 *                     $Archive:: /Commando/Code/commando/dlgevamaptab.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/08/01 10:38a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_EVA_MAP_TAB_H
#define __DLG_EVA_MAP_TAB_H


#include "childdialog.h"
#include "renegadedialog.h"


////////////////////////////////////////////////////////////////
//
//	EvaMapTabClass
//
////////////////////////////////////////////////////////////////
class EvaMapTabClass : public ChildDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////	
	EvaMapTabClass (void)	:
		ChildDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_ENCYCLOPEDIA_MAP_TAB))	{}

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	void		On_MapCtrl_Marker_Hilighted (MapCtrlClass *map_ctrl, int ctrl_id, int marker_index) override;
	void		On_MapCtrl_Pos_Clicked (MapCtrlClass *map_ctrl, int ctrl_id, const Vector3 &pos) override;
	
protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;


	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
};


#endif //__DLG_EVA_MAP_TAB_H

