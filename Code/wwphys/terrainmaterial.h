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
 *                 Project Name : leveledit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/terrainmaterial.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/08/02 2:49p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __TERRAINMATERIAL_H
#define __TERRAINMATERIAL_H


#include "wwstring.h"
#include "refcount.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class TextureClass;
class ChunkSaveClass;
class ChunkLoadClass;


//////////////////////////////////////////////////////////////////////
//
//	TerrainMaterialClass
//
//////////////////////////////////////////////////////////////////////
class TerrainMaterialClass : public RefCountClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	TerrainMaterialClass  (void);
	virtual ~TerrainMaterialClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Texture support
	//
	TextureClass *			Peek_Texture (void)							{ return Texture; }
	const char *			Get_Texture_Name (void)						{ return TextureName; }
	void						Set_Texture (const char *texture_name);

	//
	//	UV mapping control
	//
	void						Set_Meters_Per_Tile (float value)		{ MetersPerTile = value; }
	float						Get_Meters_Per_Tile (void) const			{ return MetersPerTile; }

	void						Mirror_UVs (bool onoff)						{ AreUVsMirrored = onoff; }
	bool						Are_UVs_Mirrored (void) const				{ return AreUVsMirrored; }

	//
	//	Surface type support
	//
	void						Set_Surface_Type (int type);
	int						Get_Surface_Type (void)				{ return SurfaceType; }

	//
	//	Save/load support
	//
	bool						Save (ChunkSaveClass &csave);
	bool						Load (ChunkLoadClass &cload);
	void						Load_Variables (ChunkLoadClass &cload);

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	StringClass				TextureName;
	TextureClass *			Texture;
	float						MetersPerTile;
	bool						AreUVsMirrored;
	int						SurfaceType;
};


#endif //__TERRAINMATERIAL_H
