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
 *                     $Archive:: /Commando/Code/wwui/mousemgr.h              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/22/01 4:36p                                             $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MOUSE_MGR_H
#define __MOUSE_MGR_H


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class TextureClass;
class ScreenCursorClass;


////////////////////////////////////////////////////////////////
//
//	MouseMgrClass
//
////////////////////////////////////////////////////////////////
class MouseMgrClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		CURSOR_ARROW	= 0,
		CURSOR_TEXT,
		CURSOR_ACTION,
		CURSOR_BUSY,
		CURSOR_PAN_UP,
		CURSOR_ROTATE,
		CURSOR_COUNT
	} CURSOR_TYPE;

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	static void				Initialize (void);
	static void				Shutdown (void);

	//
	//	Display methods
	//
	static void				Show_Cursor (bool onoff);
	static void				Set_Cursor (CURSOR_TYPE type);
	static void				Begin_Wait_Cursor(void);
	static void				End_Wait_Cursor(void);

	//
	//	Render methods
	//
	static void				Render (void);

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	static TextureClass *			Textures[CURSOR_COUNT];
	static ScreenCursorClass *		MouseCursor;
	static CURSOR_TYPE				CursorType;

	static unsigned int CursorWaitCount;
};

#endif //__MOUSE_MGR_H

