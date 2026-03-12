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
 *                     $Archive:: /Commando/Code/commando/radiocommanddisplay.h                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/18/01 11:59a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __RADIOCOMMANDDISPLAY_H
#define __RADIOCOMMANDDISPLAY_H


#include "wwstring.h"
#include "vector.h"
#include "vector3.h"
#include "textwindow.h"


//////////////////////////////////////////////////////////////////////
//
//	RadioCommandDisplayClass
//
//////////////////////////////////////////////////////////////////////
class RadioCommandDisplayClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public contants
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		DISPLAY_CMDS_01	= 0,
		DISPLAY_CMDS_02,
		DISPLAY_CMDS_03,
		DISPLAY_MAX
	} DISPLAY_TYPE;

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	static void			Initialize (void);
	static void			Shutdown (void);

	//
	//	Visibility methods
	//
	static bool			Is_Displayed (void) { return IsDisplayed; }
	static void			Display (bool onoff, DISPLAY_TYPE type = DISPLAY_CMDS_01);
	static void			Render (void);

	static void			Reset_Display_Timer (void)	{ DisplayTimer = 2.0F; }

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////

	//
	//	Display control
	//
	static void			Check_Keys (void);
	static void			Update (DISPLAY_TYPE type);

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////

private:

	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	static float					DisplayTimer;
	static bool						IsDisplayed;
	static TextWindowClass *	TextWindow;
};


#endif //__RADIOCOMMANDDISPLAY_H
