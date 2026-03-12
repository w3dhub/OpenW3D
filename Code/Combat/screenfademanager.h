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
 *                 Project Name : Combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/screenfademanager.h                   $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/17/02 12:08p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef SCREENFADEMANAGER_H
#define SCREENFADEMANAGER_H

#include "always.h"

class Vector3;
class	ChunkSaveClass;
class	ChunkLoadClass;


/**
** ScreenFadeManager
** This class renders letterbox polys and full-screen "tint" polys.  It should get to
** think once per frame and get rendered last.
*/
class	ScreenFadeManager
{
public:

	/*
	** Infrastructure
	*/
	static	void	Init();
	static	void 	Shutdown();
	static	void 	Think();
	static	void 	Render();

	static	bool	Save( ChunkSaveClass & csave );
	static	bool	Load( ChunkLoadClass & cload );

	/*
	** Interface for controlling the overlay effects
	*/
	static	void	Enable_Letterbox(bool onoff, float time = 1.0f);
	static	void	Set_Screen_Overlay_Color(const Vector3 & color,float time = 0.0f);
	static	void	Set_Screen_Overlay_Color(float r,float g, float b,float time = 0.0f);
	static	void	Set_Screen_Overlay_Opacity(float opacity,float time = 0.0f);

};


#endif //SCREENFADEMANAGER_H

