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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/dlgconfigperformancetab.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/31/01 8:49p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCONFIGPERFORMANCETAB_H
#define __DLGCONFIGPERFORMANCETAB_H


#include "childdialog.h"
#include "vector.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class DialogControlClass;


//////////////////////////////////////////////////////////////////////
//
//	DlgConfigPerformanceTabClass
//
//////////////////////////////////////////////////////////////////////
class DlgConfigPerformanceTabClass : public ChildDialogClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	DlgConfigPerformanceTabClass  (void);
	~DlgConfigPerformanceTabClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Command (int ctrl_id, int message_id, DWORD param) override;
	void		On_SliderCtrl_Pos_Changed (SliderCtrlClass *slider_ctrl, int ctrl_id, int new_pos) override;
	bool		On_Apply (void) override;
	
protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////	
	void		Build_Expert_Window_List (void);
	void		Display_Expert_Settings (bool onoff);
	void		Setup_Controls (void);
	void		Load_Values (void);
	void		Determine_Performance_Setting (void);
	void		Update_Expert_Controls (int level);
	void		Get_Settings (DynamicVectorClass<int> &settings);	
	
	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	DynamicVectorClass<DialogControlClass *>	ExpertWindowList;
};


#endif //__DLGCONFIGPERFORMANCETAB_H
