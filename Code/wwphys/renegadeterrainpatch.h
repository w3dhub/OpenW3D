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
 *                     $Archive:: /Commando/Code/wwphys/renegadeterrainpatch.h                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/12/02 2:34p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __RENEGADETERRAINPATCH_H
#define __RENEGADETERRAINPATCH_H


#include "rendobj.h"
#include "vector.h"
#include "shader.h"
#include "coltest.h"
#include "castres.h"
#include "tri.h"
#include "renegadeterrainmaterialpass.h"
#include "terrainmaterial.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class VertexMaterialClass;
class MaterialPassClass;


//////////////////////////////////////////////////////////////////////
//
//	RenegadeTerrainPatchClass
//
//////////////////////////////////////////////////////////////////////
class RenegadeTerrainPatchClass  : public RenderObjClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	RenegadeTerrainPatchClass (void);
	RenegadeTerrainPatchClass (const RenegadeTerrainPatchClass &src);
	virtual ~RenegadeTerrainPatchClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public operators
	///////////////////////////////////////////////////////////////////
	const RenegadeTerrainPatchClass &	operator= (const RenegadeTerrainPatchClass &src);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Creation
	//
	void					Allocate (int points_x, int points_y, float meters_per_point);

	//
	//	Vertex access
	//
	void					Set_Vertex_Pos (int x, int y, const Vector3 &pos);
	void					Set_Vertex_Normal (int x, int y, const Vector3 &normal)	{ GridNormals[Grid_Index (x, y)] = normal; AreBuffersDirty = true; }

	//
	//	Quad access
	//
	void					Set_Quad_Flags (int quad_index, uint8 flags)		{ QuadFlags[quad_index] = flags; }

	//
	//	UV access
	//
	void					Update_UVs (void);
	void					Update_Vertex_Render_Lists (void);

	//
	//	Informational
	//
	void					Set_Bounding_Box_Min (const Vector3 &box_min)	{ BoundingBoxMin = box_min; }
	void					Set_Bounding_Box_Max (const Vector3 &box_max)	{ BoundingBoxMax = box_max; }

	//
	//	Save/load support
	//	
	bool					Save (ChunkSaveClass &csave) override;
	bool					Load (ChunkLoadClass &cload) override;
	void					Load_Variables (ChunkLoadClass &cload);
	void					Load_Materials (ChunkLoadClass &cload);
	const PersistFactoryClass &	Get_Factory (void) const override;

	//
	//	Inherited
	//
	RenderObjClass *	Clone (void) const override							{ return new RenegadeTerrainPatchClass (*this); }
	int					Class_ID (void) const override						{ return CLASSID_RENEGADE_TERRAIN; }
	void					Render (RenderInfoClass &rinfo) override;
	void		 			Get_Obj_Space_Bounding_Sphere (SphereClass &sphere) const override;
	void					Get_Obj_Space_Bounding_Box (AABoxClass &box) const override;
	
	//
	//	Inherited collision detection
	//
	bool					Cast_Ray (RayCollisionTestClass &raytest) override;
	bool					Cast_AABox (AABoxCollisionTestClass &boxtest) override;
	bool					Cast_OBBox (OBBoxCollisionTestClass &boxtest) override;
	bool					Intersect_AABox (AABoxIntersectionTestClass &boxtest) override;
	bool					Intersect_OBBox (OBBoxIntersectionTestClass &boxtest) override;

	//
	//	Material access
	//
	int											Add_Material (TerrainMaterialClass *material);
	void											Reset_Material_Passes (void);
	RenegadeTerrainMaterialPassClass *	Get_Material_Pass (int index, TerrainMaterialClass *material);
	int											Get_Material_Count (void)			{ return MaterialPassList.Count (); }	
	RenegadeTerrainMaterialPassClass *	Peek_Material_Pass (int index)	{ return MaterialPassList[index]; }

	//
	// Access to internal mesh data
	//
	int					Get_Vertex_Count (void) const				{ return GridPointCount; }
	Vector3 *			Get_Vertex_Array (void) const				{ return Grid; }
	Vector3 *			Get_Vertex_Normal_Array (void) const	{ return GridNormals; }

	//
	//	Lighting support
	//
	bool					Is_Prelit (void) const										{ return IsPreLit; }
	void					Set_Is_Prelit (bool onoff)									{ IsPreLit = onoff; }
	void					Set_Vertex_Color (int index, const Vector3 &color)	{ VertexColors[index] = color; AreBuffersDirty = true; }
	const Vector3 &	Get_Vertex_Color (int index)								{ return VertexColors[index]; }

protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////	

	//
	//	Index utility functions
	//
	int						Grid_Index (int x, int y)			{ return (y * GridPointsX) + x; }
	
	int						Get_Quad_Index_X (float x_pos, bool clamp = true);
	int						Get_Quad_Index_Y (float y_pos, bool clamp = true);
	float						Get_Grid_Line_Pos_X (int grid_line_index);
	float						Get_Grid_Line_Pos_Y (int grid_line_index);

	bool						Is_Valid_Quad (int x, int y);

	//
	//	Matrix utility functions
	//
	void						Get_Inverse_Transform (Matrix3D &tm);

	//
	//	Misc initialization and cleanup routines
	//
	void						Free_Grid (void);
	void						Free_Materials (void);
	void						Initialize_Material (void);
	void						Initialize (void);
	void						Allocate_Grid (void);

	//
	//	Rendering
	//
	void						Render_By_Texture (int texture_index, int pass_type);
	void						Submit_Rendering_Buffers (int texture_index, int pass_type);
	void						Update_Rendering_Buffers (void);
	void						Free_Rendering_Buffers (void);
	void						Build_Rendering_Buffers (int texture_index, int pass_type);
	void						Render_Procedural_Material_Pass(MaterialPassClass * matpass);

	//
	//	Ray casting
	//
	bool						Cast_Vertical_Ray (RayCollisionTestClass &raytest);
	bool						Cast_Non_Vertical_Ray (RayCollisionTestClass &raytest);
	bool						Brute_Force_Cast_Ray (RayCollisionTestClass &raytest);

	bool						Collide_Quad (const LineSegClass &line_seg, int cell_x, int cell_y, CastResultStruct &result);

	///////////////////////////////////////////////////////////////////
	//	Protected data types
	///////////////////////////////////////////////////////////////////
	enum
	{
		QF_NORMAL		= 0,
		QF_HIDDEN		= 1,
	} QUAD_FLAGS;

	
	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	Vector3 *							Grid;
	Vector3 *							GridNormals;
	Vector3 *							VertexColors;
	uint8 *								QuadFlags;	
	int									GridPointsX;
	int									GridPointsY;
	int									GridPointCount;
	float									Density;
	VertexMaterialClass *			BaseMaterial;
	ShaderClass							BaseShader;
	VertexMaterialClass *			LayerMaterial;	
	ShaderClass							LayerShader;
	Vector3								BoundingBoxMin;
	Vector3								BoundingBoxMax;
	bool									AreBuffersDirty;
	bool									IsPreLit;
						
	DynamicVectorClass<RenegadeTerrainMaterialPassClass *>	MaterialPassList;
};


//////////////////////////////////////////////////////////////////////
//	Collide_Quad
//////////////////////////////////////////////////////////////////////
WWINLINE bool
RenegadeTerrainPatchClass::Collide_Quad
(
	const LineSegClass &	line_seg,
	int						cell_x,
	int						cell_y,
	CastResultStruct &	result
)
{
	bool retval = false;

	//
	//	Calculate the four indices into the grid that define the four
	// corners of the quad we're testing.
	//
	int start_index	= Grid_Index (cell_x, cell_y);
	int v0_index		= start_index;
	int v1_index		= start_index + 1;
	int v2_index		= start_index + GridPointsX + 1;
	int v3_index		= start_index + GridPointsX;

	float min_pz = WWMath::Min (line_seg.Get_P0 ().Z, line_seg.Get_P1 ().Z);
	float max_pz = WWMath::Max (line_seg.Get_P0 ().Z, line_seg.Get_P1 ().Z);

	if (	Grid[v0_index].Z < min_pz &&
			Grid[v1_index].Z < min_pz &&
			Grid[v2_index].Z < min_pz &&
			Grid[v3_index].Z < min_pz)
	{
		return false;
	}

	if (	Grid[v0_index].Z > max_pz &&
			Grid[v1_index].Z > max_pz &&
			Grid[v2_index].Z > max_pz &&
			Grid[v3_index].Z > max_pz)
	{
		return false;
	}

	//
	//	Compose the two triangles for the collision check
	//
	TriClass tri1;
	TriClass tri2;
	Vector3 norm1 (0, 0, 0);
	Vector3 norm2 (0, 0, 0);
	tri1.N = &norm1;
	tri2.N = &norm2;
	tri1.V[0] = &Grid[v0_index];
	tri1.V[1] = &Grid[v2_index];
	tri1.V[2] = &Grid[v3_index];
	tri2.V[0] = &Grid[v2_index];
	tri2.V[1] = &Grid[v0_index];
	tri2.V[2] = &Grid[v1_index];
	tri1.Compute_Normal ();
	tri2.Compute_Normal ();

	//
	//	Test the two polygons that form this quad to see if the ray hit
	//
	retval = CollisionMath::Collide (line_seg, tri1, &result);
	retval |= CollisionMath::Collide (line_seg, tri2, &result);

	//
	//	Determine which surface type was hit
	//
	if (retval && MaterialPassList.Count () > 0) {
		int best_pass		= 0;
		float best_alpha	= 0;
		for (int index = 0; index < MaterialPassList.Count (); index ++) {
			float alpha = MaterialPassList[index]->VertexAlpha[v0_index];
			if (alpha > best_alpha && alpha != 1.0F) {
				best_alpha	= alpha;
				best_pass	= index;
			}
		}
		if (MaterialPassList[best_pass]->Material != NULL) {
			result.SurfaceType = MaterialPassList[best_pass]->Material->Get_Surface_Type ();
		}
	}

	return retval;
}

//////////////////////////////////////////////////////////////////////
//	Set_Vertex_Pos
//////////////////////////////////////////////////////////////////////
WWINLINE void
RenegadeTerrainPatchClass::Set_Vertex_Pos (int x, int y, const Vector3 &pos)
{
	//
	//	Update the vertex position
	//
	Grid[Grid_Index (x, y)] = pos;
	
	//
	//	Do we need to update the bounding volume of the patch?
	//
	if (pos.Z > BoundingBoxMax.Z) {
		BoundingBoxMax.Z = pos.Z;		
	}

	if (pos.Z < BoundingBoxMin.Z) {
		BoundingBoxMin.Z = pos.Z;
	}

	Invalidate_Cached_Bounding_Volumes ();
	AreBuffersDirty		= true;
	return ;
}

//////////////////////////////////////////////////////////////////////
//	Get_Quad_Index_X
//////////////////////////////////////////////////////////////////////
WWINLINE int
RenegadeTerrainPatchClass::Get_Quad_Index_X (float x_pos, bool clamp)
{
	int quad_x = WWMath::Float_To_Int_Floor ((x_pos - BoundingBoxMin.X) / Density);

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
RenegadeTerrainPatchClass::Get_Quad_Index_Y (float y_pos, bool clamp)
{
	int quad_y = WWMath::Float_To_Int_Floor ((y_pos - BoundingBoxMin.Y) / Density);

	//
	//	Clamp the index to the range of existing quad cells
	//
	if (clamp) {
		quad_y = WWMath::Clamp_Int (quad_y, 0, GridPointsY - 2);
	}

	return quad_y;
}

//////////////////////////////////////////////////////////////////////
//	Get_Grid_Line_Pos_X
//////////////////////////////////////////////////////////////////////
WWINLINE float
RenegadeTerrainPatchClass::Get_Grid_Line_Pos_X (int grid_line_index)
{
	return BoundingBoxMin.X + (grid_line_index * Density);
}

//////////////////////////////////////////////////////////////////////
//	Get_Grid_Line_Pos_Y
//////////////////////////////////////////////////////////////////////
WWINLINE float
RenegadeTerrainPatchClass::Get_Grid_Line_Pos_Y (int grid_line_index)
{
	return BoundingBoxMin.Y + (grid_line_index * Density);
}

//////////////////////////////////////////////////////////////////////
//	Get_Inverse_Transform
//////////////////////////////////////////////////////////////////////
WWINLINE void
RenegadeTerrainPatchClass::Get_Inverse_Transform (Matrix3D &tm)
{
	Get_Transform ().Get_Orthogonal_Inverse (tm);
	return ;
}

//////////////////////////////////////////////////////////////////////
//	Is_Valid_Quad
//////////////////////////////////////////////////////////////////////
WWINLINE bool
RenegadeTerrainPatchClass::Is_Valid_Quad (int x, int y)
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

#endif //__RENEGADETERRAINPATCH_H
