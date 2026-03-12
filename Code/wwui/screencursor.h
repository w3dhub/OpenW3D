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
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/screencursor.h                                                                                                                                                                                                                                                                                                                                        $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SCREENCURSOR_H
#define __SCREENCURSOR_H

#include "vector2.h"
#include "win.h"
#include "render2d.h"


///////////////////////////////////////////////////////////////////////////////
//
//	ScreenCursorClass
//
///////////////////////////////////////////////////////////////////////////////
class ScreenCursorClass
{
public:

	////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////////
	ScreenCursorClass (void);
	~ScreenCursorClass (void);

	////////////////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////////////////
	const ScreenCursorClass &operator= (const ScreenCursorClass &src);

	////////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////////
	void						Set_Texture (TextureClass *texture);
	void						Render (void);

	void						Set_Hotspot (const Vector2 &pos)	{ Hotspot = pos; }
	const Vector2 &		Get_Hotspot (void) const			{ return Hotspot; }

private:

	////////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////////
	Vector2					Hotspot;
	int 						Width;
	int						Height;
	Render2DClass			Renderer;
	TextureClass *			Texture;
};


#endif //__SCREENCURSOR_H
