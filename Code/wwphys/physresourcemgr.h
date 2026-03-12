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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/physresourcemgr.h                     $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/20/01 3:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef PHYSRESOURCEMGR_H
#define PHYSRESOURCEMGR_H

#include "always.h"
#include "shader.h"

class TextureClass;
class VertexMaterialClass;
class MaterialPassClass;


/**
** PhysResourceMgrClass
** This class is just a collection of resources that I've ended up needed at one time or another
** in the physics library.  I thought it would be easier to collect them all here so the Init and
** Shutdown code can clean them all up at once.
*/
class PhysResourceMgrClass
{
public:

	static void Init(void);
	static void Shutdown(void);

	static bool								Set_Shadow_Blob_Texture(const char * texname);
	static TextureClass *				Get_Shadow_Blob_Texture(void);

	static MaterialPassClass *			Get_Highlight_Material_Pass(void);

	static TextureClass *				Get_Stealth_Texture(void);
	static TextureClass *				Peek_Stealth_Texture(void);

	static VertexMaterialClass *		Create_Emissive_Material(void);

	static TextureClass *				Get_Grid_Texture(void);
	static TextureClass *				Peek_Grid_Texture(void);
};


#endif

