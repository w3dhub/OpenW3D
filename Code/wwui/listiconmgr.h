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
 *                 Project Name : wwui                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/listiconmgr.h                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/26/01 2:49p                                                $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __LISTICONMGR_H
#define __LISTICONMGR_H

#include "vector.h"
#include "wwstring.h"
#include "stylemgr.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class Render2DClass;
class RectClass;


//////////////////////////////////////////////////////////////////////
//
//	ListIconMgrClass
//
//////////////////////////////////////////////////////////////////////
class ListIconMgrClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ListIconMgrClass (void);
	~ListIconMgrClass (void)	{ Reset_Icons (); }

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Configuration
	//
	void			Add_Icon (const char *texture_name);
	void			Remove_Icon (const char *texture_name);
	void			Reset_Icons (void);

	//
	//	Rendering
	//
	void			Reset_Renderers (void);
	void			Render_Icon (const RectClass &clip_rect, const char *texture_name);
	void			Render_Icons (void);

	//
	//	Informational
	//
	float			Get_Icon_Width (void)				{ return IconWidth * StyleMgrClass::Get_X_Scale (); }
	float			Get_Icon_Height (void)				{ return IconHeight * StyleMgrClass::Get_Y_Scale (); }

	void			Set_Icon_Width (float width)		{ IconWidth = width; }
	void			Set_Icon_Height (float height)	{ IconHeight = height; }

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	int			Find_Texture (const char *texture_name);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	DynamicVectorClass<Render2DClass *>	Renderers;
	DynamicVectorClass<StringClass>		TextureNames;
	float											IconWidth;
	float											IconHeight;
};


#endif //__LISTICONMGR_H
