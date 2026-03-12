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
 *                 Project Name : wwphys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/renegadeterrainmaterialpass.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/04/02 6:06p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __RENEGADETERRAINMATERIALPASS_H
#define __RENEGADETERRAINMATERIALPASS_H

#include "vector.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class Vector2;
class TerrainMaterialClass;
class DX8IndexBufferClass;
class DX8VertexBufferClass;
class ChunkSaveClass;
class ChunkLoadClass;


//////////////////////////////////////////////////////////////////////
//
//	RenegadeTerrainMaterialPassClass
//
//////////////////////////////////////////////////////////////////////
class RenegadeTerrainMaterialPassClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public data types
	///////////////////////////////////////////////////////////////////
	typedef enum
	{
		PASS_BASE			= 0,
		PASS_ALPHA,
		PASS_COUNT

	} TEXTURE_PASS_TYPE;

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	RenegadeTerrainMaterialPassClass  (void);
	virtual ~RenegadeTerrainMaterialPassClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public Methods
	///////////////////////////////////////////////////////////////////
	void		Allocate (int vertex_count);
	void		Reset (void);

	//
	//	Save/load support
	//
	bool		Save (ChunkSaveClass &csave);
	bool		Load (ChunkLoadClass &cload);
	void		Load_Variables (ChunkLoadClass &cload);

	///////////////////////////////////////////////////////////////////
	//	Public member data
	///////////////////////////////////////////////////////////////////
	int								VertexCount;
	float *							VertexAlpha;
	Vector2 *						GridUVs;
	TerrainMaterialClass *		Material;
	DynamicVectorClass<int>		QuadList[PASS_COUNT];
	DynamicVectorClass<int>		VertexRenderList[PASS_COUNT];
	int *								VertexIndexMap[PASS_COUNT];

	DX8IndexBufferClass *		IndexBuffers[PASS_COUNT];
	DX8VertexBufferClass *		VertexBuffers[PASS_COUNT];
};


#endif //__RENEGADETERRAINMATERIALPASS_H
