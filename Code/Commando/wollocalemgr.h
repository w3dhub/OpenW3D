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
 *                     $Archive:: /Commando/Code/Commando/wollocalemgr.h                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/23/01 7:57p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __WOLLOCALEMGR_H
#define __WOLLOCALEMGR_H

#include "WWOnline/WOLSession.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class ComboBoxCtrlClass;


//////////////////////////////////////////////////////////////////////
//
//	WolLocaleMgrClass
//
//////////////////////////////////////////////////////////////////////
class WolLocaleMgrClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Locale control
	//
	static WOL::Locale	Get_Locale (const unichar_t *login_name);
	static void				Set_Locale (const unichar_t *login_name, WOL::Locale locale);

	static WOL::Locale	Get_Current_Locale (void);
	static void				Set_Current_Locale (WOL::Locale locale);

	static const unichar_t* Get_Locale_String(WOL::Locale locale);

	//
	//	UI Support
	//
	static void		Configure_Locale_Combobox (ComboBoxCtrlClass *ctrl);

	//
	//	Nag dialog support
	//
	static void		Display_Nag_Dialog (void);
	
private:
	
	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
};


#endif //__WOLLOCALEMGR_H
