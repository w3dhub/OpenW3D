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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/editableheightfield.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/07/02 1:55p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __EDITABLEHEIGHTFIELD_H
#define __EDITABLEHEIGHTFIELD_H


#include "rendobj.h"
#include "vector.h"
#include "shader.h"
#include "coltest.h"
#include "castres.h"
#include "tri.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class VertexMaterialClass;
class TerrainMaterialClass;
class RenegadeTerrainPatchClass;
class StaticPhysClass;


//////////////////////////////////////////////////////////////////////
//
//	EditableHeightfieldClass
//
//////////////////////////////////////////////////////////////////////
class EditableHeightfieldClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	EditableHeightfieldClass (void);
	EditableHeightfieldClass (const EditableHeightfieldClass &src);
	virtual ~EditableHeightfieldClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public operators
	///////////////////////////////////////////////////////////////////
	const EditableHeightfieldClass &	operator= (const EditableHeightfieldClass &src);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	void						Create (float width, float height, float meters_per_point);
	void						Create (const char *heightmap_filename, float width, float height, float meters_per_point, float scale);
	void						Create (HBITMAP bitmap, float width, float height, float meters_per_point, float scale);

	//
	//	Configuration
	//
	void						Set_Dimensions (float width, float height, float meters_per_point);
	float						Get_Width (void) const		{ return Width; }
	float						Get_Height (void) const		{ return Height; }
	float						Get_Density (void) const	{ return Density; }

	//
	//	Height editing
	//
	void						Deform_Heightfield (const Vector3 &center, float amount, float inner_radius, float outter_radius);
	void						Smooth_Heightfield (const Vector3 &center, float amount, float inner_radius, float outter_radius);
	void						Smooth_Foundation_Heightfield (const Vector3 &center, float amount, float inner_radius, float outter_radius);
	void						Paint_Heightfield (const Vector3 &world_space_center, int texture_index, float amount, float	inner_radius, float outter_radius);
	void						Cutout_Heightfield (const Vector3 &world_space_center, float outter_radius, bool subtract);

	//
	//	Save/load support
	//
	bool						Save (ChunkSaveClass &csave);
	bool						Load (ChunkLoadClass &cload);
	void						Load_Variables (ChunkLoadClass &cload);
	void						Load_Materials (ChunkLoadClass &cload);
	void						On_Post_Load (void);
	void						Assign_Unique_IDs (void);

	//
	//	Material access
	//
	TerrainMaterialClass *	Peek_Material (int index)			{ return MaterialList[index]; }
	void							Set_Material (int index, TerrainMaterialClass *material);
	void							On_Material_Changed (int index);

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////

	//
	//	Index utility functions
	//
	int						Grid_Index (int x, int y)			{ return (y * GridPointsX) + x; }

	int						Get_Point_Index_X (float x_pos, bool clamp = true);
	int						Get_Point_Index_Y (float y_pos, bool clamp = true);
	int						Get_Quad_Index_X (float x_pos, bool clamp = true);
	int						Get_Quad_Index_Y (float y_pos, bool clamp = true);

	bool						Is_Valid_Quad (int x, int y);

	void						Free_Grid (void);
	void						Free_Patches (void);
	void						Free_Material (void);
	void						Initialize_Material (void);
	void						Initialize (void);
	void						Initialize_Grid (void);
	void						Allocate_Grid (void);

	void						Update_Normals (int min_x, int min_y, int max_x, int max_y);
	void						Update_UVs (void);
	void						Update_Texture_Quad_List (int min_x, int min_y, int max_x, int max_y);

	void						Allocate_Patches (void);
	void						Update_Patch_Pos_And_Normals (int min_x, int min_y, int max_x, int max_y);
	void						Update_Patch_Texture_Quads (void);


	///////////////////////////////////////////////////////////////////
	//	Protected data types
	///////////////////////////////////////////////////////////////////
	typedef enum
	{
		PASS_BASE			= 0,
		PASS_ALPHA,
		PASS_COUNT

	} TEXTURE_PASS_TYPE;

	enum
	{
		MAX_TEXTURE_PASSES	= 10,
	};

	enum
	{
		QF_NORMAL		= 0,
		QF_HIDDEN		= 1,
	} QUAD_FLAGS;

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	Vector3 *								Grid;
	Vector3 *								GridNormals;
	Vector2 *								GridUVs[MAX_TEXTURE_PASSES];
	float *									TextureWeights[MAX_TEXTURE_PASSES];
	uint8 *									QuadFlags;
	int										GridPointsX;
	int										GridPointsY;
	int										GridPointCount;
	float										Width;
	float										Height;
	float										Density;
	TerrainMaterialClass *				MaterialList[MAX_TEXTURE_PASSES];

	RenegadeTerrainPatchClass **		PatchGrid;
	StaticPhysClass **					PhysObjGrid;
	int										PatchesX;
	int										PatchesY;
	int										PatchGridPointsX;
	int										PatchGridPointsY;
	float										PatchWidth;
	float										PatchHeight;
};


//////////////////////////////////////////////////////////////////////
//	Get_Point_Index_X
//////////////////////////////////////////////////////////////////////
WWINLINE int
EditableHeightfieldClass::Get_Point_Index_X (float x_pos, bool clamp)
{
	int point_x = WWMath::Float_To_Int_Floor (x_pos / Density);

	//
	//	Clamp the index to the range of existing points
	//
	if (clamp) {
		point_x = WWMath::Clamp_Int (point_x, 0, GridPointsX - 1);
	}

	return point_x;
}

//////////////////////////////////////////////////////////////////////
//	Get_Point_Index_Y
//////////////////////////////////////////////////////////////////////
WWINLINE int
EditableHeightfieldClass::Get_Point_Index_Y (float y_pos, bool clamp)
{
	int point_y = WWMath::Float_To_Int_Floor (y_pos / Density);

	//
	//	Clamp the index to the range of existing point cells
	//
	if (clamp) {
		point_y = WWMath::Clamp_Int (point_y, 0, GridPointsY - 1);
	}

	return point_y;
}

//////////////////////////////////////////////////////////////////////
//	Get_Quad_Index_X
//////////////////////////////////////////////////////////////////////
WWINLINE int
EditableHeightfieldClass::Get_Quad_Index_X (float x_pos, bool clamp)
{
	int quad_x = WWMath::Float_To_Int_Floor (x_pos / Density);

	//
	//	Clamp the index to the range of existing quad cells
	//
	if (clamp) {
		quad_x = WWMath::Clamp_Int (quad_x, 0, GridPointsX - 2);
	}

	return quad_x;
}

//////////////////////////////////////////////////////////////////////
//	Get_Quad_Index_Y
//////////////////////////////////////////////////////////////////////
WWINLINE int
EditableHeightfieldClass::Get_Quad_Index_Y (float y_pos, bool clamp)
{
	int quad_y = WWMath::Float_To_Int_Floor (y_pos / Density);

	//
	//	Clamp the index to the range of existing quad cells
	//
	if (clamp) {
		quad_y = WWMath::Clamp_Int (quad_y, 0, GridPointsY - 2);
	}

	return quad_y;
}

//////////////////////////////////////////////////////////////////////
//	Is_Valid_Quad
//////////////////////////////////////////////////////////////////////
WWINLINE bool
EditableHeightfieldClass::Is_Valid_Quad (int x, int y)
{
	bool retval = false;

	//
	//	Simply check to see if the coordinates are inside the patch
	//
	if (	x >= 0 && x < (GridPointsX - 1) &&
			y >= 0 && y < (GridPointsY - 1))
	{
		retval = true;
	}

	return retval;
}


#endif //__EDITABLEHEIGHTFIELD_H
