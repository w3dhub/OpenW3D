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
 *                 Project Name : WWCtrl                                                       *
 *                                                                                             *
 *                     $Archive:: /G/Tools/WWCtrl/WWCtrl.h                                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/10/99 12:31p                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __WWCTRL_H
#define __WWCTRL_H

#ifdef WWCTRL_LIB
	#define LINKAGE_SPEC	__declspec (dllexport)
#else
	#define LINKAGE_SPEC
#endif

typedef void (*WWCTRL_COLORCALLBACK)(int,int,int,void*);

//////////////////////////////////////////////////////////////////////
//
//	Prototypes
//
//////////////////////////////////////////////////////////////////////
extern "C"
{
	LINKAGE_SPEC void	Initialize_WWCtrl (void);

	LINKAGE_SPEC BOOL Show_Color_Picker (int *red, int *green, int *blue);

	LINKAGE_SPEC HWND Create_Color_Picker_Form (HWND parent, int red, int green, int blue);
	LINKAGE_SPEC BOOL Get_Form_Color (HWND form_wnd, int *red, int *green, int *blue);
	LINKAGE_SPEC BOOL Set_Form_Color (HWND form_wnd, int red, int green, int blue);
	LINKAGE_SPEC BOOL Set_Form_Original_Color (HWND form_wnd, int red, int green, int blue);

	LINKAGE_SPEC BOOL Set_Update_Callback (HWND form_wnd, WWCTRL_COLORCALLBACK callback, void *arg=NULL);
}

#endif //__WWCTRL_H

