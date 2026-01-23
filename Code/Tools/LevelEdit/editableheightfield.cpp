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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/editableheightfield.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/11/02 5:01p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"

#include "editableheightfield.h"
#include "Utils.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"
#include "sortingrenderer.h"
#include "rinfo.h"
#include "camera.h"
#include "dx8fvf.h"
#include "filemgr.h"
#include "vector2i.h"
#include "terrainmaterial.h"
#include "renegadeterrainpatch.h"
#include "renegadeterrainmaterialpass.h"
#include "terrainmaterial.h"
#include "staticphys.h"
#include "sceneeditor.h"
#include "filemgr.h"
#include "nodemgr.h"
#include <algorithm>

////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES				= 0x02261040,
	CHUNKID_HEIGHTS,
	CHUNKID_TEXTURE_WEIGHTS,
	CHUNKID_MATERIAL_LIST,
	CHUNKID_MATERIAL,
	CHUNKID_EMPTY_MATERIAL_SLOT,
	CHUNKID_QUAD_FLAGS
};

enum
{
	VARID_GRID_PTS_X				= 0x01,
	VARID_GRID_PTS_Y,
	VARID_GRID_PT_COUNT,
	VARID_GRID_WIDTH,
	VARID_GRID_HEIGHT,
	VARID_GRID_DENSITY,
	VARID_MAX_TEXTURE_PASSES,
};


//////////////////////////////////////////////////////////////////////
//
//	EditableHeightfieldClass
//
//////////////////////////////////////////////////////////////////////
EditableHeightfieldClass::EditableHeightfieldClass (void)	:
	Width (0),
	Height (0),
	Density (1.0F),
	GridPointsX (0),
	GridPointsY (0),
	GridPointCount (0),
	Grid (NULL),
	GridNormals (NULL),
	QuadFlags (NULL),
	PatchGrid (NULL),
	PhysObjGrid (NULL),
	PatchesX (0),
	PatchesY (0),
	PatchGridPointsX (0),
	PatchGridPointsY (0),
	PatchWidth (0.0F),
	PatchHeight (0.0F)
{
	Initialize ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	EditableHeightfieldClass
//
//////////////////////////////////////////////////////////////////////
EditableHeightfieldClass::EditableHeightfieldClass (const EditableHeightfieldClass &src)	:
	Width (0),
	Height (0),
	Density (1.0F),
	GridPointsX (0),
	GridPointsY (0),
	GridPointCount (0),
	Grid (NULL),
	GridNormals (NULL),
	QuadFlags (NULL),
	PatchGrid (NULL),
	PhysObjGrid (NULL),
	PatchesX (0),
	PatchesY (0),
	PatchGridPointsX (0),
	PatchGridPointsY (0),
	PatchWidth (0.0F),
	PatchHeight (0.0F)
{
	Initialize ();
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~EditableHeightfieldClass
//
//////////////////////////////////////////////////////////////////////
EditableHeightfieldClass::~EditableHeightfieldClass (void)
{
	Free_Grid ();
	Free_Material ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	operator=
//
//////////////////////////////////////////////////////////////////////
const EditableHeightfieldClass &
EditableHeightfieldClass::operator= (const EditableHeightfieldClass &/* src */)
{
	return *this;
}



//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Initialize (void)
{
	//
	//	Zero all the lists
	//
	::memset (MaterialList, 0, sizeof (MaterialList));
	::memset (GridUVs, 0, sizeof (GridUVs));
	::memset (TextureWeights, 0, sizeof (TextureWeights));

	//
	//	Initialize the material settings
	//
	Initialize_Material ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Create (float width, float height, float meters_per_point)
{
	Set_Dimensions (width, height, meters_per_point);
	Assign_Unique_IDs ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Dimensions
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Set_Dimensions (float width, float height, float meters_per_point)
{
	Free_Grid ();

	//
	//	Calculate how many grid points we will have in the x and y directions
	//
	Density			= meters_per_point;
	GridPointsX		= int(width / Density);
	GridPointsY		= int(height / Density);
	GridPointsX		= std::max (1, GridPointsX);
	GridPointsY		= std::max (1, GridPointsY);	
	GridPointCount	= (GridPointsX * GridPointsY);
	Width				= (GridPointsX * Density);
	Height			= (GridPointsY * Density);

	//
	//	Allocate and initialize the grid and supporting data structures
	//
	Allocate_Patches ();
	Allocate_Grid ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Allocate_Grid
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Allocate_Grid (void)
{
	//
	//	Allocate the per-texture data structures
	//
	for (int index = 0; index < MAX_TEXTURE_PASSES; index ++) {

		//
		//	Allocate the per-texture UV arrays
		//
		GridUVs[index] = new Vector2 [GridPointCount];

		//
		//	Allocate the amount of texture influences for each texture for each vertex
		//
		TextureWeights[index] = new float[GridPointCount];
		::memset (TextureWeights[index], 0, sizeof (float) * GridPointCount);
	}

	//
	//	Allocate the arrays
	//
	GridNormals	= new Vector3[GridPointCount];
	Grid			= new Vector3[GridPointCount];	

	//
	//	Allocate and initiailze the array of quad flags
	//
	int quad_count	= (GridPointsX - 1) * (GridPointsY - 1);
	QuadFlags		= new uint8[quad_count];
	::memset (QuadFlags, 0, sizeof (uint8) * quad_count);

	//
	//	Initialize the grid and the uv array's
	//
	Initialize_Grid ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize_Grid
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Initialize_Grid (void)
{
	float y_value = 0;

	//
	//	Give each point on the grid a position
	//
	for (int y_pos = 0; y_pos < GridPointsY; y_pos ++) {
		int start_index = (y_pos * GridPointsX);
		float x_value = 0;			

		for (int x_pos = 0; x_pos < GridPointsX; x_pos ++) {
			
			//
			//	Simply set the x,y,z position of this vertex
			//
			Grid[start_index + x_pos].Set (x_value, y_value, 0.0F);			
			x_value += Density;

			//
			//	Also give the vertex a texture influence weight
			//
			TextureWeights[0][start_index + x_pos] = 1.0F;

			//
			//	Calculate the UV coordinate for this texture at this vertex
			//			
			for (int index = 0; index < MAX_TEXTURE_PASSES; index ++) {
				if (MaterialList[index] != NULL) {
					float meters_per_texture = MaterialList[index]->Get_Meters_Per_Tile ();
					float u_value = Grid[start_index + x_pos].X / meters_per_texture;
					float v_value = Grid[start_index + x_pos].Y / meters_per_texture;
					GridUVs[index][start_index + x_pos].Set (u_value, v_value);
				}
			}
		}

		//
		//	Move down to the next row
		//
		y_value += Density;
	}

	//
	//	Update the normals of the grid
	//
	Update_Normals (0, 0, GridPointsX - 1, GridPointsY - 1);
	Update_Patch_Pos_And_Normals (0, 0, GridPointsX - 1, GridPointsY - 1);
	Update_UVs ();
	Update_Texture_Quad_List (0, 0, GridPointsX - 1, GridPointsY - 1);	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Free_Grid
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Free_Grid (void)
{
	SAFE_DELETE_ARRAY (Grid);
	SAFE_DELETE_ARRAY (GridNormals);	
	SAFE_DELETE_ARRAY (QuadFlags);

	//
	//	Free the texture weight arrays
	//
	for (int index = 0; index < MAX_TEXTURE_PASSES; index ++) {
		SAFE_DELETE (TextureWeights[index]);
		SAFE_DELETE (GridUVs[index]);
	}

	Width				= 0;
	Height			= 0;
	Density			= 1.0F;
	GridPointsX		= 0;
	GridPointsY		= 0;
	GridPointCount	= 0;

	//
	//	Now free each of the passes
	//
	Free_Patches ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Update_Texture_Quad_List
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Update_Texture_Quad_List (int min_x, int min_y, int max_x, int max_y)
{
	//
	//	Determine which patches this operation would possibly affect
	//
	int min_patch_x = min_x / (PatchGridPointsX - 1);
	int min_patch_y = min_y / (PatchGridPointsY - 1);
	int max_patch_x = max_x / (PatchGridPointsX - 1);
	int max_patch_y = max_y / (PatchGridPointsY - 1);
	max_patch_x = std::min (max_patch_x, (PatchesX - 1));
	max_patch_y = std::min (max_patch_y, (PatchesY - 1));

	int quads_per_patch_x = PatchGridPointsX - 1;
	int quads_per_patch_y = PatchGridPointsY - 1;

	//
	//	Loop over each quad in the heightfield and determine which pass(es)
	//	they should be rendered in...
	//
	for (int patch_y = min_patch_y; patch_y <= max_patch_y; patch_y ++) {
		for (int patch_x = min_patch_x; patch_x <= max_patch_x; patch_x ++) {			

			//
			//	Reset the passes for this patch
			//
			int patch_index = (patch_y * PatchesX) + patch_x;
			PatchGrid[patch_index]->Reset_Material_Passes ();

			for (int patch_quad_y = 0; patch_quad_y < quads_per_patch_y; patch_quad_y ++) {
				for (int patch_quad_x = 0; patch_quad_x < quads_per_patch_x; patch_quad_x ++) {

					//
					//	Determine the quad index inside the patch
					//
					int patch_quad_index	= (patch_quad_y * (PatchGridPointsX - 1)) + patch_quad_x;

					//
					//	Calculate where in the overall heigtmap this quad is
					//
					int quad_x = (patch_x * quads_per_patch_x) + patch_quad_x;
					int quad_y = (patch_y * quads_per_patch_y) + patch_quad_y;
					int overall_quad_index = (quad_y * (GridPointsX - 1)) + quad_x;

					//
					//	Copy the quad flags to the pathc
					//
					PatchGrid[patch_index]->Set_Quad_Flags (patch_quad_index, QuadFlags[overall_quad_index]);

					//
					//	Skip this quad if its hidden
					//
					if (QuadFlags[overall_quad_index] & QF_HIDDEN) {
						continue;
					}

					//
					//	Calculate what vertices define the four corners of this quad
					//
					int start_index = (quad_y * GridPointsX) + quad_x;					
					int v0_index = start_index;
					int v1_index = start_index + 1;
					int v2_index = start_index + GridPointsX + 1;
					int v3_index = start_index + GridPointsX;

					//
					//	Calculate what vertices define the four corners of this quad (in patch space)
					//
					int patch_start_index = (patch_quad_y * PatchGridPointsX) + patch_quad_x;
					int patch_v0_index = patch_start_index;
					int patch_v1_index = patch_start_index + 1;
					int patch_v2_index = patch_start_index + PatchGridPointsX + 1;
					int patch_v3_index = patch_start_index + PatchGridPointsX;

					float v0_alpha_sum = 0.0F;
					float v1_alpha_sum = 0.0F;
					float v2_alpha_sum = 0.0F;
					float v3_alpha_sum = 0.0F;

					//
					//	Determine which passes this quad should be rendered in...
					//
					bool is_set = false;
					for (int layer = 0; layer < MAX_TEXTURE_PASSES; layer ++) {
						
						//
						//	Is there a significant influence from one of the textures on this quad?
						//
						if (	TextureWeights[layer][v0_index] > 0.0F ||
								TextureWeights[layer][v1_index] > 0.0F ||
								TextureWeights[layer][v2_index] > 0.0F ||
								TextureWeights[layer][v3_index] > 0.0F)
						{
							//
							//	Get acces to the material pass for this layer
							//
							RenegadeTerrainMaterialPassClass *material_pass = NULL;
							material_pass = PatchGrid[patch_index]->Get_Material_Pass (layer, MaterialList[layer]);

							v0_alpha_sum += TextureWeights[layer][v0_index];
							v1_alpha_sum += TextureWeights[layer][v1_index];
							v2_alpha_sum += TextureWeights[layer][v2_index];
							v3_alpha_sum += TextureWeights[layer][v3_index];

							//
							//	Add this quad to either the alpha pass or the base pass
							//
							if (is_set) {
								material_pass->QuadList[PASS_ALPHA].Add (patch_quad_index);

								//
								//	Set the vertex alpha's for the verts inside the material layer
								//
								material_pass->VertexAlpha[patch_v0_index] = TextureWeights[layer][v0_index] / v0_alpha_sum;
								material_pass->VertexAlpha[patch_v1_index] = TextureWeights[layer][v1_index] / v1_alpha_sum;
								material_pass->VertexAlpha[patch_v2_index] = TextureWeights[layer][v2_index] / v2_alpha_sum;
								material_pass->VertexAlpha[patch_v3_index] = TextureWeights[layer][v3_index] / v3_alpha_sum;

							} else {
								material_pass->QuadList[PASS_BASE].Add (patch_quad_index);
								is_set = true;
							}
						}
					}
				}
			}

			//
			//	Now, make each patch update its internal rendering structures
			//
			PatchGrid[patch_index]->Update_Vertex_Render_Lists ();
			PatchGrid[patch_index]->Update_UVs ();
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Material
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Set_Material (int index, TerrainMaterialClass *material)
{
	if (index < 0 || index >= MAX_TEXTURE_PASSES) {
		return ;
	}
	
	//
	//	Store this material in its slot
	//
	REF_PTR_SET (MaterialList[index], material);

	//
	//	Update UV coords for this texture pass
	//
	On_Material_Changed (index);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Material_Changed
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::On_Material_Changed (int index)
{
	if (index < 0 || index >= MAX_TEXTURE_PASSES || MaterialList[index] == NULL) {
		return ;
	}

	//
	//	Loop over all the verts on the grid
	//
	/*for (int y_pos = 0; y_pos < GridPointsY; y_pos ++) {
		int start_index = (y_pos * GridPointsX);
		for (int x_pos = 0; x_pos < GridPointsX; x_pos ++) {
			
			//
			//	Calculate the UV coordinate for this texture at this vertex
			//			
			float meters_per_texture = MaterialList[index]->Get_Meters_Per_Tile ();
			float u_value = Grid[start_index + x_pos].X / meters_per_texture;
			float v_value = Grid[start_index + x_pos].Y / meters_per_texture;
			GridUVs[index][start_index + x_pos].Set (u_value, v_value);
		}
	}*/

	//
	//	Now, make each patch update its internal rendering structures
	//
	int patch_count = PatchesX * PatchesY; 
	for (int patch_index = 0; patch_index < patch_count; patch_index ++) {
		PatchGrid[patch_index]->Update_UVs ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Update_UVs
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Update_UVs (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Update_Normals
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Update_Normals (int min_x, int min_y, int max_x, int max_y)
{
	//
	//	First, initialize each vertex normal to 0
	//
	int y_pos;
	for (y_pos = min_y; y_pos <= max_y; y_pos ++) {
		int vert_index = (y_pos * GridPointsX) + min_x;
		for (int x_pos = min_x; x_pos <= max_x; x_pos ++) {
			
			//
			//	Initialize this normal
			//
			GridNormals[vert_index ++].Set (0.0F, 0.0F, 0.0F);
		}
	}

	//
	//	Loop over each vertex that's affected and add the plane normal of the
	// 6 triangles that are formed from this vertex .
	//
	for (y_pos = min_y; y_pos <= max_y; y_pos ++) {
		int vert_index = (y_pos * GridPointsX) + min_x;
		for (int x_pos = min_x; x_pos <= max_x; x_pos ++) {

			//
			//	Upper-left quad
			//
			if (y_pos > 0 && x_pos > 0) {
				int q0_v0_index = (vert_index - GridPointsX) - 1;
				int q0_v1_index = vert_index - GridPointsX;
				int q0_v2_index = vert_index;
				int q0_v3_index = vert_index - 1;
				const Vector3 &q0_v0 = Grid[q0_v0_index];
				const Vector3 &q0_v1 = Grid[q0_v1_index];
				const Vector3 &q0_v2 = Grid[q0_v2_index];
				const Vector3 &q0_v3 = Grid[q0_v3_index];
				Vector3 q0_normal1 = Vector3::Cross_Product (q0_v2 - q0_v1, q0_v0 - q0_v1);
				Vector3 q0_normal2 = Vector3::Cross_Product (q0_v3 - q0_v2, q0_v0 - q0_v2);
				GridNormals[vert_index] += q0_normal1;
				GridNormals[vert_index] += q0_normal2;
			}

			//
			//	Upper-right quad
			//
			if (y_pos > 0 && x_pos < (GridPointsX - 1)) {
				int q1_v0_index = vert_index - GridPointsX;
				int q1_v2_index = vert_index + 1;
				int q1_v3_index = vert_index;
				const Vector3 &q1_v0 = Grid[q1_v0_index];
				const Vector3 &q1_v2 = Grid[q1_v2_index];
				const Vector3 &q1_v3 = Grid[q1_v3_index];
				Vector3 q1_normal2 = Vector3::Cross_Product (q1_v3 - q1_v2, q1_v0 - q1_v2);
				GridNormals[vert_index] += q1_normal2;
			}

			//
			//	Lower-right quad
			//
			if (y_pos < (GridPointsY - 1) && x_pos < (GridPointsX - 1)) {
				int q2_v0_index = vert_index;
				int q2_v1_index = vert_index + 1;
				int q2_v2_index = vert_index + GridPointsX + 1;
				int q2_v3_index = vert_index + GridPointsX;
				const Vector3 &q2_v0 = Grid[q2_v0_index];
				const Vector3 &q2_v1 = Grid[q2_v1_index];
				const Vector3 &q2_v2 = Grid[q2_v2_index];
				const Vector3 &q2_v3 = Grid[q2_v3_index];
				Vector3 q2_normal1 = Vector3::Cross_Product (q2_v2 - q2_v1, q2_v0 - q2_v1);
				Vector3 q2_normal2 = Vector3::Cross_Product (q2_v3 - q2_v2, q2_v0 - q2_v2);
				GridNormals[vert_index] += q2_normal1;
				GridNormals[vert_index] += q2_normal2;
			}

			//
			//	Lower-left quad
			//
			if (y_pos < (GridPointsY - 1) && x_pos > 0) {
				int q3_v0_index = vert_index - 1;
				int q3_v1_index = vert_index;
				int q3_v2_index = vert_index + GridPointsX;
				const Vector3 &q3_v0 = Grid[q3_v0_index];
				const Vector3 &q3_v1 = Grid[q3_v1_index];
				const Vector3 &q3_v2 = Grid[q3_v2_index];
				Vector3 q3_normal1 = Vector3::Cross_Product (q3_v2 - q3_v1, q3_v0 - q3_v1);
				GridNormals[vert_index] += q3_normal1;
			}

			GridNormals[vert_index ++].Normalize ();
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize_Material
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Initialize_Material (void)
{
	//
	//	Allocate a test texture
	//
	CString full_path = ::Get_File_Mgr ()->Make_Full_Path ("Heightfield\\DIRT_01.TGA");
	MaterialList[0] = new TerrainMaterialClass;
	MaterialList[0]->Set_Texture (full_path);
	MaterialList[0]->Set_Meters_Per_Tile (25.0F);

	//
	//	Allocate a test texture
	//	
	/*full_path = ::Get_File_Mgr ()->Make_Full_Path ("Heightfield\\L03_bushes.tga");
	MaterialList[1] = new TerrainMaterialClass;
	MaterialList[1]->Set_Texture (full_path);
	MaterialList[1]->Set_Meters_Per_Tile (25.0F);

	//
	//	Allocate a test texture
	//	
	full_path = ::Get_File_Mgr ()->Make_Full_Path ("Heightfield\\cht_GryStn.tga");
	MaterialList[2] = new TerrainMaterialClass;
	MaterialList[2]->Set_Texture (full_path);
	MaterialList[2]->Set_Meters_Per_Tile (25.0F);

	//
	//	Allocate a test texture
	//	
	full_path = ::Get_File_Mgr ()->Make_Full_Path ("Heightfield\\cht_snwmts.tga");
	MaterialList[3] = new TerrainMaterialClass;
	MaterialList[3]->Set_Texture (full_path);
	MaterialList[3]->Set_Meters_Per_Tile (25.0F);

	//
	//	Allocate a test texture
	//	
	full_path = ::Get_File_Mgr ()->Make_Full_Path ("Heightfield\\lv8_dirt2.tga");
	MaterialList[4] = new TerrainMaterialClass;
	MaterialList[4]->Set_Texture (full_path);
	MaterialList[4]->Set_Meters_Per_Tile (25.0F);

	//
	//	Allocate a test texture
	//	
	full_path = ::Get_File_Mgr ()->Make_Full_Path ("Heightfield\\L03_lav001.tga");
	MaterialList[5] = new TerrainMaterialClass;
	MaterialList[5]->Set_Texture (full_path);
	MaterialList[5]->Set_Meters_Per_Tile (25.0F);

	//
	//	Allocate a test texture
	//	
	full_path = ::Get_File_Mgr ()->Make_Full_Path ("Heightfield\\L03_bchsnd.tga");
	MaterialList[6] = new TerrainMaterialClass;
	MaterialList[6]->Set_Texture (full_path);
	MaterialList[6]->Set_Meters_Per_Tile (25.0F);*/
	return ;	
}


//////////////////////////////////////////////////////////////////////
//
//	Free_Material
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Free_Material (void)
{
	//
	//	Release the textures
	//
	for (int index = 0; index < MAX_TEXTURE_PASSES; index ++) {
		REF_PTR_RELEASE (MaterialList[index]);
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Deform_Heightfield
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Deform_Heightfield
(
	const Vector3 &	world_space_center,
	float					amount,
	float					inner_radius,
	float					outter_radius
)
{
	//
	//	First, transform the center point into heightfield (object) space
	//
	const Matrix3D tm = Matrix3D (1);//Get_Transform ();
	Matrix3D world_to_obj_tm;
	tm.Get_Orthogonal_Inverse (world_to_obj_tm);

	Vector3 center;
	Matrix3D::Transform_Vector (world_to_obj_tm, world_space_center, &center);
	
	//
	//	Offset the affect verts
	//
	float min_x_value = center.X - outter_radius;
	float min_y_value = center.Y - outter_radius;
	float max_x_value = center.X + outter_radius;
	float max_y_value = center.Y + outter_radius;

	int min_x = WWMath::Float_To_Int_Floor ((min_x_value / Density));
	int min_y = WWMath::Float_To_Int_Floor ((min_y_value / Density));
	int max_x = WWMath::Float_To_Int_Floor ((max_x_value / Density));
	int max_y = WWMath::Float_To_Int_Floor ((max_y_value / Density));
	min_x = WWMath::Clamp_Int (min_x, 0, GridPointsX - 1);
	min_y = WWMath::Clamp_Int (min_y, 0, GridPointsY - 1);
	max_x = WWMath::Clamp_Int (max_x, 0, GridPointsX - 1);
	max_y = WWMath::Clamp_Int (max_y, 0, GridPointsY - 1);

	float delta_radius = (outter_radius - inner_radius);

	//
	//	Loop over all the verts in this region and apply the deformation
	//

	for (int y_pos = min_y; y_pos <= max_y; y_pos ++) {
		int curr_offset = (y_pos * GridPointsX) + min_x;

		for (int x_pos = min_x; x_pos <= max_x; x_pos ++) {
			
			Vector3 &vert_pos = Grid[curr_offset];

			//
			//	Calculate how far this vertex is from the center of the deformation
			//
			Vector2 delta (vert_pos.X - center.X, vert_pos.Y - center.Y);
			float dist = WWMath::Sqrt ((delta.X * delta.X) + (delta.Y * delta.Y));

			//
			//	Is this vertex inside the affected region?
			//
			if (dist <= outter_radius) {
				
				//
				//	Calculate what percentage this vertex is affected by the deformation
				//
				float percent = 1.0F;				
				if (dist > inner_radius && delta_radius > 0.0F) {
					percent = 1.0F - ((dist - inner_radius) / delta_radius);
				}
				
				//
				//	Scale the amount and apply it to the z-position of the vertex
				//
				float delta_amount = percent * amount;
				vert_pos.Z += delta_amount;
			}
			
			//
			//	Advance to the next vertex
			//
			curr_offset ++;
		}
	}
	
	//
	//	Update the normals of the affected verts
	//
	Update_Normals (min_x, min_y, max_x, max_y);
	Update_Patch_Pos_And_Normals (min_x, min_y, max_x, max_y);
	Update_UVs ();	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Smooth_Foundation_Heightfield
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Smooth_Foundation_Heightfield
(
	const Vector3 &	world_space_center,
	float					/* amount */,
	float					inner_radius,
	float					outter_radius
)
{
	//
	//	First, transform the center point into heightfield (object) space
	//
	const Matrix3D tm = Matrix3D (1);
	Matrix3D world_to_obj_tm;
	tm.Get_Orthogonal_Inverse (world_to_obj_tm);

	Vector3 center;
	Matrix3D::Transform_Vector (world_to_obj_tm, world_space_center, &center);
	
	//
	//	Offset the affect verts
	//
	float min_x_value = center.X - outter_radius;
	float min_y_value = center.Y - outter_radius;
	float max_x_value = center.X + outter_radius;
	float max_y_value = center.Y + outter_radius;

	int min_x = WWMath::Float_To_Int_Floor ((min_x_value / Density));
	int min_y = WWMath::Float_To_Int_Floor ((min_y_value / Density));
	int max_x = WWMath::Float_To_Int_Floor ((max_x_value / Density));
	int max_y = WWMath::Float_To_Int_Floor ((max_y_value / Density));
	min_x = WWMath::Clamp_Int (min_x, 0, GridPointsX - 1);
	min_y = WWMath::Clamp_Int (min_y, 0, GridPointsY - 1);
	max_x = WWMath::Clamp_Int (max_x, 0, GridPointsX - 1);
	max_y = WWMath::Clamp_Int (max_y, 0, GridPointsY - 1);

	float delta_radius = (outter_radius - inner_radius);

	float avg_height = 0;
	int count = 0;

	//
	//	Loop over all the verts in this region and apply the deformation
	//
	int y_pos;
	for (y_pos = min_y; y_pos <= max_y; y_pos ++) {
		int curr_offset = (y_pos * GridPointsX) + min_x;

		for (int x_pos = min_x; x_pos <= max_x; x_pos ++) {
			
			Vector3 &vert_pos = Grid[curr_offset];

			//
			//	Calculate how far this vertex is from the center of the deformation
			//
			Vector2 delta (vert_pos.X - center.X, vert_pos.Y - center.Y);
			float dist = WWMath::Sqrt ((delta.X * delta.X) + (delta.Y * delta.Y));

			//
			//	Is this vertex inside the affected region?
			//
			if (dist <= outter_radius) {

				avg_height += vert_pos.Z;
				count ++;
			}
			
			//
			//	Advance to the next vertex
			//
			curr_offset ++;
		}
	}




	if (count == 0) {
		return ;
	}

	avg_height = avg_height / count;


	//
	//	Loop over all the verts in this region and apply the deformation
	//
	for (y_pos = min_y; y_pos <= max_y; y_pos ++) {
		int curr_offset = (y_pos * GridPointsX) + min_x;

		for (int x_pos = min_x; x_pos <= max_x; x_pos ++) {
			
			Vector3 &vert_pos = Grid[curr_offset];

			//
			//	Calculate how far this vertex is from the center of the deformation
			//
			Vector2 delta (vert_pos.X - center.X, vert_pos.Y - center.Y);
			float dist = WWMath::Sqrt ((delta.X * delta.X) + (delta.Y * delta.Y));

			//
			//	Is this vertex inside the affected region?
			//
			if (dist <= outter_radius) {
				
				//
				//	Calculate what percentage this vertex is affected by the deformation
				//
				float percent = 1.0F;				
				if (dist > inner_radius && delta_radius > 0.0F) {
					percent = 1.0F - ((dist - inner_radius) / delta_radius);
				}
				
				//
				//	Scale the amount and apply it to the z-position of the vertex
				//
				float real_percent = percent * 0.5F;
				float z_value = (vert_pos.Z - avg_height) * real_percent;
				vert_pos.Z -= z_value;
			}
			
			//
			//	Advance to the next vertex
			//
			curr_offset ++;
		}
	}
	
	//
	//	Update the normals of the affected verts
	//
	Update_Normals (min_x, min_y, max_x, max_y);
	Update_Patch_Pos_And_Normals (min_x, min_y, max_x, max_y);
	Update_UVs ();	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Smooth_Heightfield
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Smooth_Heightfield
(
	const Vector3 &	world_space_center,
	float					/* amount */,
	float					inner_radius,
	float					outter_radius
)
{
	//
	//	First, transform the center point into heightfield (object) space
	//
	const Matrix3D tm = Matrix3D (1);
	Matrix3D world_to_obj_tm;
	tm.Get_Orthogonal_Inverse (world_to_obj_tm);

	Vector3 center;
	Matrix3D::Transform_Vector (world_to_obj_tm, world_space_center, &center);
	
	//
	//	Offset the affect verts
	//
	float min_x_value = center.X - outter_radius;
	float min_y_value = center.Y - outter_radius;
	float max_x_value = center.X + outter_radius;
	float max_y_value = center.Y + outter_radius;

	int min_x = WWMath::Float_To_Int_Floor ((min_x_value / Density));
	int min_y = WWMath::Float_To_Int_Floor ((min_y_value / Density));
	int max_x = WWMath::Float_To_Int_Floor ((max_x_value / Density));
	int max_y = WWMath::Float_To_Int_Floor ((max_y_value / Density));
	min_x = WWMath::Clamp_Int (min_x, 0, GridPointsX - 1);
	min_y = WWMath::Clamp_Int (min_y, 0, GridPointsY - 1);
	max_x = WWMath::Clamp_Int (max_x, 0, GridPointsX - 1);
	max_y = WWMath::Clamp_Int (max_y, 0, GridPointsY - 1);

	float delta_radius = (outter_radius - inner_radius);

	static const Vector2i INDEX_RING_OFFSETS[] =
	{
		Vector2i (-1, -1),	Vector2i (0, -1),	Vector2i (+1, -1),
		Vector2i (-1, 0),								Vector2i (+1, 0), 
		Vector2i (-1, +1),	Vector2i (0, +1),	Vector2i (+1, +1)
	};

	//
	//	Create a list we can use to keep track of the height changes
	//
	DynamicVectorClass<float> new_height_list;
	DynamicVectorClass<int> new_height_list_pos;
	new_height_list.Set_Growth_Step (1000);
	new_height_list_pos.Set_Growth_Step (1000);

	//
	//	Loop over all the verts in this region and apply the deformation
	//
	for (int y_pos = min_y; y_pos <= max_y; y_pos ++) {
		int curr_offset = (y_pos * GridPointsX) + min_x;

		for (int x_pos = min_x; x_pos <= max_x; x_pos ++) {
			
			Vector3 &vert_pos = Grid[curr_offset];

			//
			//	Calculate how far this vertex is from the center of the deformation
			//
			Vector2 delta (vert_pos.X - center.X, vert_pos.Y - center.Y);
			float dist = WWMath::Sqrt ((delta.X * delta.X) + (delta.Y * delta.Y));

			//
			//	Is this vertex inside the affected region?
			//
			if (dist <= outter_radius) {
				
				//
				//	Calculate what percentage this vertex is affected by the deformation
				//
				float percent = 1.0F;				
				if (dist > inner_radius && delta_radius > 0.0F) {
					percent = 1.0F - ((dist - inner_radius) / delta_radius);
				}
				
				//
				//	Scale the amount and apply it to the z-position of the vertex
				//
				float real_percent = percent * 0.5F;


				//
				//	Now, average the height of this vertex with the surrounding heights
				//
				int count = 0;
				float avg_height = 0;

				for (int index = 0; index < 7; index ++) {
					int new_x_pos = x_pos + INDEX_RING_OFFSETS[index].I;
					int new_y_pos = y_pos + INDEX_RING_OFFSETS[index].J;
					
					//
					//	If there is a vertex in this position, the take
					// into consideration its height
					//
					if (	new_x_pos >= 0 && new_x_pos < GridPointsX &&
							new_y_pos >= 0 && new_y_pos < GridPointsY)
					{
						int test_index = (new_y_pos * GridPointsX) + new_x_pos;
						avg_height += Grid[test_index].Z;
						count ++;
					}
				}

				float z_value = vert_pos.Z;

				//
				//	Use the average height value if possible
				//
				if (count > 0) {
					avg_height	= avg_height / count;
					z_value		-= (vert_pos.Z - avg_height) * real_percent;
				}

				new_height_list.Add (z_value);
				new_height_list_pos.Add (curr_offset);
			}
			
			//
			//	Advance to the next vertex
			//
			curr_offset ++;
		}
	}

	//
	//	Now, loop over all the verts in this region and apply the new heights
	//
	for (int index = 0; index < new_height_list.Count (); index ++) {
		
		//
		//	Set this vertex's height
		//
		Vector3 &vert_pos = Grid[new_height_list_pos[index]];
		vert_pos.Z = new_height_list[index];
	}
	
	//
	//	Update the normals of the affected verts
	//
	Update_Normals (min_x, min_y, max_x, max_y);
	Update_Patch_Pos_And_Normals (min_x, min_y, max_x, max_y);
	Update_UVs ();	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Cutout_Heightfield
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Cutout_Heightfield
(
	const Vector3 &	world_space_center,
	float					outter_radius,
	bool					subtract
)
{
	//
	//	First, transform the center point into heightfield (object) space
	//
	const Matrix3D tm = Matrix3D (1);
	Matrix3D world_to_obj_tm;
	tm.Get_Orthogonal_Inverse (world_to_obj_tm);

	Vector3 center;
	Matrix3D::Transform_Vector (world_to_obj_tm, world_space_center, &center);

	//
	//	Determine which quads are affected by this operation
	//
	float min_x_value = center.X - outter_radius;
	float min_y_value = center.Y - outter_radius;
	float max_x_value = center.X + outter_radius;
	float max_y_value = center.Y + outter_radius;
	int min_x = Get_Quad_Index_X (min_x_value);
	int min_y = Get_Quad_Index_Y (min_y_value);
	int max_x = Get_Quad_Index_X (max_x_value);
	int max_y = Get_Quad_Index_Y (max_y_value);

	//
	//	Loop over all the quads in this region...
	//
	for (int quad_y = min_y; quad_y <= max_y; quad_y ++) {
		int quad_index = (quad_y * (GridPointsX - 1)) + min_x;
		for (int quad_x = min_x; quad_x <= max_x; quad_x ++) {
			
			//
			//	Set the bits that specify this quad is not to be rendered
			//
			if (subtract) {
				QuadFlags[quad_index] |= QF_HIDDEN;
			} else {
				QuadFlags[quad_index] &= (~QF_HIDDEN);
			}

			//
			//	Advance to the next quad
			//
			quad_index ++;
		}
	}
	
	//
	//	Update the texture rendering lists
	//
	Update_Texture_Quad_List (Get_Point_Index_X (min_x_value), Get_Point_Index_Y (min_y_value),
		Get_Point_Index_X (max_x_value), Get_Point_Index_Y (max_y_value));
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Paint_Heightfield
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Paint_Heightfield
(
	const Vector3 &	world_space_center,
	int					texture_index,
	float					amount,
	float					inner_radius,
	float					outter_radius
)
{
	//
	//	First, transform the center point into heightfield (object) space
	//
	const Matrix3D tm = Matrix3D (1);
	Matrix3D world_to_obj_tm;
	tm.Get_Orthogonal_Inverse (world_to_obj_tm);

	Vector3 center;
	Matrix3D::Transform_Vector (world_to_obj_tm, world_space_center, &center);
	
	//
	//	Offset the affect verts
	//
	float min_x_value = center.X - outter_radius;
	float min_y_value = center.Y - outter_radius;
	float max_x_value = center.X + outter_radius;
	float max_y_value = center.Y + outter_radius;

	int min_x = WWMath::Float_To_Int_Floor ((min_x_value / Density));
	int min_y = WWMath::Float_To_Int_Floor ((min_y_value / Density));
	int max_x = WWMath::Float_To_Int_Floor ((max_x_value / Density));
	int max_y = WWMath::Float_To_Int_Floor ((max_y_value / Density));
	min_x = WWMath::Clamp_Int (min_x, 0, GridPointsX - 1);
	min_y = WWMath::Clamp_Int (min_y, 0, GridPointsY - 1);
	max_x = WWMath::Clamp_Int (max_x, 0, GridPointsX - 1);
	max_y = WWMath::Clamp_Int (max_y, 0, GridPointsY - 1);

	float delta_radius = (outter_radius - inner_radius);

	//
	//	Loop over all the verts in this region and apply the deformation
	//

	for (int y_pos = min_y; y_pos <= max_y; y_pos ++) {
		int curr_offset = (y_pos * GridPointsX) + min_x;

		for (int x_pos = min_x; x_pos <= max_x; x_pos ++) {
			
			Vector3 &vert_pos = Grid[curr_offset];

			//
			//	Calculate how far this vertex is from the center of the deformation
			//
			Vector2 delta (vert_pos.X - center.X, vert_pos.Y - center.Y);
			float dist = WWMath::Sqrt ((delta.X * delta.X) + (delta.Y * delta.Y));

			//
			//	Is this vertex inside the affected region?
			//
			if (dist <= outter_radius) {
				
				//
				//	Calculate what percentage this vertex is affected by the deformation
				//
				float percent = 1.0F;				
				if (dist > inner_radius && delta_radius > 0.0F) {
					percent = 1.0F - ((dist - inner_radius) / delta_radius);
				}
				
				//
				//	Scale the amount and apply it to the z-position of the vertex
				//
				float delta_amount = percent * amount;
				
				//
				//	Determine how much influence each of the other texture channel had
				//
				float weight_amount					= TextureWeights[texture_index][curr_offset];
				float old_remainder_amount	= 1.0F - weight_amount;

				//
				//	Increment the amount of influence for this texture channel
				//
				float new_amount	= weight_amount + delta_amount;
				new_amount			= WWMath::Clamp (new_amount, 0.0F, 1.0F);

				//
				//	Calculate how much influence is left for the other texture channels
				//
				float remainder = (1.0F - new_amount);
				
				if (old_remainder_amount != 0.0F) {
					
					//
					//	Reduce the amount of influence each pass has after this operation...
					//
					float scale_factor = remainder / old_remainder_amount;
					for (int index = 0; index < MAX_TEXTURE_PASSES; index ++) {
						if (index != texture_index) {
							TextureWeights[index][curr_offset] *= scale_factor;
						}
					}
				}

				TextureWeights[texture_index][curr_offset] = new_amount;
			}
			
			//
			//	Advance to the next vertex
			//
			curr_offset ++;
		}
	}
	
	//
	//	Update the texture rendering lists
	//
	Update_Texture_Quad_List (min_x, min_y, max_x, max_y);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////
bool
EditableHeightfieldClass::Save (ChunkSaveClass &csave)
{
	//
	//	Write the variables
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_PTS_X,				GridPointsX);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_PTS_Y,				GridPointsY);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_PT_COUNT,			GridPointCount);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_WIDTH,				Width);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_HEIGHT,				Height);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_DENSITY,			Density);

		int texture_passes = MAX_TEXTURE_PASSES;		
		WRITE_MICRO_CHUNK (csave, VARID_MAX_TEXTURE_PASSES,	texture_passes);

	csave.End_Chunk ();

	//
	//	Now, write out the "array" of heights
	//
	csave.Begin_Chunk (CHUNKID_HEIGHTS);
		int index;
		for (index = 0; index < GridPointCount; index ++) {
			csave.Write (&Grid[index].Z, sizeof (float));
		}
				
	csave.End_Chunk ();

	//
	//	Now, write out the "array" of weights
	//
	csave.Begin_Chunk (CHUNKID_TEXTURE_WEIGHTS);

		//
		//	Save the array of texture weights per vertex per pass
		//
		for (index = 0; index < GridPointCount; index ++) {			
			for (int pass = 0; pass < MAX_TEXTURE_PASSES; pass ++) {
				csave.Write (&TextureWeights[pass][index], sizeof (float));
			}
		}
				
	csave.End_Chunk ();	

	//
	//	Now, write out the array of quad flags
	//
	csave.Begin_Chunk (CHUNKID_QUAD_FLAGS);

		int quad_count = (GridPointsX - 1) * (GridPointsY - 1);
		csave.Write (QuadFlags, sizeof (uint8) * quad_count);
				
	csave.End_Chunk ();

	//
	//	Now, write out the list of materials
	//
	csave.Begin_Chunk (CHUNKID_MATERIAL_LIST);

		for (index = 0; index < MAX_TEXTURE_PASSES; index ++) {
			
			//
			//	Is there a material to save?
			//
			if (MaterialList[index] != NULL) {

				//
				//	Save the material's definition to this chunk
				//
				csave.Begin_Chunk (CHUNKID_MATERIAL);
					MaterialList[index]->Save (csave);
				csave.End_Chunk ();

			} else {

				//
				//	Save an empty chunk so we know to leave this slot empty
				//
				csave.Begin_Chunk (CHUNKID_EMPTY_MATERIAL_SLOT);
				csave.End_Chunk ();
			}
		}
				
	csave.End_Chunk ();

	return true;
}


//////////////////////////////////////////////////////////////////////
//
//	Load_Materials
//
//////////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Load_Materials (ChunkLoadClass &cload)
{
	//
	//	Start fresh
	//
	Free_Material ();
	int index = 0;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			//
			//	Load all the variables from this chunk
			//
			case CHUNKID_MATERIAL:
			{
				//
				//	Load the material
				//
				TerrainMaterialClass *material = new TerrainMaterialClass;
				material->Load (cload);

				//
				//	Reset the texture filename as a "relative" path
				//
				CString full_texture_path	= material->Get_Texture_Name ();
				CString rel_texture_path	= ::Get_File_Mgr ()->Make_Relative_Path (full_texture_path);
				material->Set_Texture (rel_texture_path);
				

				//
				//	Insert the material into our list
				//
				MaterialList[index ++] = material;
				break;
			}

			case CHUNKID_EMPTY_MATERIAL_SLOT:
				index ++;
				break;
		}

		cload.Close_Chunk ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////
bool
EditableHeightfieldClass::Load (ChunkLoadClass &cload)
{
	Free_Grid ();

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			//
			//	Load all the variables from this chunk
			//
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			case CHUNKID_HEIGHTS:
			{
				//
				//	Read the array of heights
				//
				for (int index = 0; index < GridPointCount; index ++) {
					cload.Read (&Grid[index].Z, sizeof (float));
				}

				break;
			}

			case CHUNKID_TEXTURE_WEIGHTS:
			{
				//
				//	Read the array of texture weights per vertex per pass
				//
				for (int index = 0; index < GridPointCount; index ++) {			
					for (int pass = 0; pass < MAX_TEXTURE_PASSES; pass ++) {
						cload.Read (&TextureWeights[pass][index], sizeof (float));
					}
				}

				break;
			}

			case CHUNKID_QUAD_FLAGS:
			{
				//
				//	Read the array of quad flags
				//
				int quad_count = (GridPointsX - 1) * (GridPointsY - 1);
				cload.Read (QuadFlags, sizeof (uint8) * quad_count);
				break;
			}

			case CHUNKID_MATERIAL_LIST:
			{
				Load_Materials (cload);
				break;
			}
		}

		cload.Close_Chunk ();
	}

	//
	//	Update all our internal data structures from the loaded data
	//
	Update_Normals (0, 0, GridPointsX - 1, GridPointsY - 1);
	Update_Patch_Pos_And_Normals (0, 0, GridPointsX - 1, GridPointsY - 1);
	Update_UVs ();
	Update_Texture_Quad_List (0, 0, GridPointsX - 1, GridPointsY - 1);
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Load_Variables (ChunkLoadClass &cload)
{
	int texture_passes = MAX_TEXTURE_PASSES;

	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			//
			//	Read each of the microchunks
			//
			READ_MICRO_CHUNK (cload, VARID_GRID_PTS_X,				GridPointsX);
			READ_MICRO_CHUNK (cload, VARID_GRID_PTS_Y,				GridPointsY);
			READ_MICRO_CHUNK (cload, VARID_GRID_PT_COUNT,			GridPointCount);
			READ_MICRO_CHUNK (cload, VARID_GRID_WIDTH,				Width);
			READ_MICRO_CHUNK (cload, VARID_GRID_HEIGHT,				Height);
			READ_MICRO_CHUNK (cload, VARID_GRID_DENSITY,				Density);			
			READ_MICRO_CHUNK (cload, VARID_MAX_TEXTURE_PASSES,		texture_passes);			
		}

		cload.Close_Micro_Chunk ();
	}

	WWASSERT (texture_passes == MAX_TEXTURE_PASSES);	

	//
	//	Allocate and initialize the grid and supporting data structures
	//
	Allocate_Patches ();
	Allocate_Grid ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Post_Load
//
////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::On_Post_Load (void)
{
	//Assign_Unique_IDs ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Assign_Unique_IDs
//
////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Assign_Unique_IDs (void)
{

	//
	//	Assign each patch a unique ID
	//
	int patch_count	= PatchesX * PatchesY;
	for (int index = 0; index < patch_count; index ++) {
		PhysObjGrid[index]->Set_ID (NodeMgrClass::Get_Node_ID (NODE_TYPE_TERRAIN));
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Create
(
	const char *	heightmap_filename,
	float				width,
	float				height,
	float				meters_per_point,
	float				scale
)
{
	//
	//	Load the bitmap from disk
	//
	HBITMAP bitmap = (HBITMAP)::LoadImage (NULL, heightmap_filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (bitmap != NULL) {

		//
		//	Initialize the heightfield from the bitmap
		//
		Create (bitmap, width, height, meters_per_point, scale);
		::DeleteObject (bitmap);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create
//
////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Create
(
	HBITMAP		bitmap,
	float			width,
	float			height,
	float			meters_per_point,
	float			scale
)
{
	if (bitmap == NULL) {
		return ;
	}

	//
	//	Get the dimensions of the bitmap
	//
	BITMAP bmp_info = { 0 };
	::GetObject (bitmap, sizeof (BITMAP), &bmp_info);

	//
	// Set-up the fields of the BITMAPINFOHEADER so we can retrieve the bitmap data in this format.
	//	Note: Top-down DIBs use negative height in Win32.
	//
	BITMAPINFOHEADER bitmap_info = { 0 };
	bitmap_info.biSize				= sizeof (BITMAPINFOHEADER);
	bitmap_info.biWidth				= bmp_info.bmWidth;
	bitmap_info.biHeight				= bmp_info.bmHeight;
	bitmap_info.biPlanes				= 1;
	bitmap_info.biBitCount			= 24;
	bitmap_info.biCompression		= BI_RGB;
	bitmap_info.biSizeImage			= ((bmp_info.bmWidth * bmp_info.bmHeight) * 3);

	//
	//	Allocate a buffer to store the bitmap data int
	//
	int stride			= (((bmp_info.bmWidth * 3) + 3) & ~3);
	uint8 *bmp_bits	= new uint8[stride * bmp_info.bmHeight];
	
	//
	//	Get the bitmap data
	//
	HDC screen_dc = ::GetDC (NULL);
	::GetDIBits (screen_dc, bitmap, 0, bmp_info.bmHeight,
			(LPVOID)bmp_bits, (BITMAPINFO *)&bitmap_info, DIB_RGB_COLORS);
	::ReleaseDC (NULL, screen_dc);

	//
	//	First, create the grid
	//
	Set_Dimensions (width, height, meters_per_point);

	float half_x_dist = ((float)bmp_info.bmWidth / (float)GridPointsX) / 2;
	float half_y_dist = ((float)bmp_info.bmHeight / (float)GridPointsY) / 2;

	//
	//	Now, loop over each point of the grid and determine what its height should
	// be based on the intensity of the corresponding pixel in the bitmap.
	//
	for (int y_pos = 0; y_pos < GridPointsY; y_pos ++) {
		int start_index = (y_pos * GridPointsX);
		for (int x_pos = 0; x_pos < GridPointsX; x_pos ++) {
			
			float x_percent = (float)x_pos / (float)(GridPointsX - 1);
			float y_percent = (float)y_pos / (float)(GridPointsY - 1);

			float pixel_x = ((bmp_info.bmWidth - 1) * x_percent) + 0.5F;
			float pixel_y = ((bmp_info.bmHeight - 1) * y_percent) + 0.5F;

			float z_value = 0.0F;

			//
			//	Do we need to sub-sample or super-sample?
			//
			if (	((bmp_info.bmWidth >= bmp_info.bmHeight) && (GridPointsX < bmp_info.bmWidth)) ||
					((bmp_info.bmHeight >= bmp_info.bmWidth) && (GridPointsY < bmp_info.bmHeight)))
			{
				//
				//	Determine what range of pixels this vertex overlays
				//
				/*float next_x_percent = (float)(x_pos + 1) / (float)(GridPointsX - 1);
				float next_y_percent = (float)(y_pos + 1) / (float)(GridPointsY - 1);
				float next_pixel_x = ((bmp_info.bmWidth - 1) * next_x_percent) + 0.5F;
				float next_pixel_y = ((bmp_info.bmHeight - 1) * next_y_percent) + 0.5F;*/

				float pixel_x_start	= pixel_x - half_x_dist;
				float pixel_y_start	= pixel_y - half_y_dist;
				float pixel_x_end		= pixel_x + half_x_dist;
				float pixel_y_end		= pixel_y + half_y_dist;

				//
				//	Clip the region to the bitmap
				//
				int pixel_x0 = WWMath::Clamp (pixel_x_start, 0, (bmp_info.bmWidth - 1));
				int pixel_y0 = WWMath::Clamp (pixel_y_start, 0, (bmp_info.bmHeight - 1));
				int pixel_x1 = WWMath::Clamp (pixel_x_end, 0, (bmp_info.bmWidth - 1));
				int pixel_y1 = WWMath::Clamp (pixel_y_end, 0, (bmp_info.bmHeight - 1));
				float total_intensity	= 0.0F;
				int count					= 0;

				//
				//	Sum the pixels in this region
				//
				for (int curr_pixel_y = pixel_y0; curr_pixel_y <= pixel_y1; curr_pixel_y ++) {
					for (int curr_pixel_x = pixel_x0; curr_pixel_x <= pixel_x1; curr_pixel_x ++) {
						total_intensity += bmp_bits[(curr_pixel_y * stride) + (curr_pixel_x * 3)];
						count ++;
					}
				}

				//
				//	The height value is the average of the pixel intensities in this region
				//
				if (count > 0) {
					float intensity = total_intensity / (count * 255);
					z_value = intensity * scale;
				}
				
			} else {

				int pixel_x0 = (int)pixel_x;
				int pixel_x1 = std::min<int> (pixel_x0 + 1, bmp_info.bmWidth - 1);
				int pixel_y0 = (int)pixel_y;
				int pixel_y1 = std::min<int> (pixel_y0 + 1, bmp_info.bmHeight - 1);

				//
				//	Get the z-values of the four corners of the pixel
				//
				float z0 = (float)bmp_bits[(pixel_y0 * stride) + (pixel_x0 * 3)];
				float z1 = (float)bmp_bits[(pixel_y0 * stride) + (pixel_x1 * 3)];
				float z2 = (float)bmp_bits[(pixel_y1 * stride) + (pixel_x1 * 3)];
				float z3 = (float)bmp_bits[(pixel_y1 * stride) + (pixel_x0 * 3)];

				//
				//	Determine how much weight we should give the z-value of each corner
				//
				float x0_percent = 1.0F - ((pixel_x - 0.5F) - pixel_x0);
				float y0_percent = 1.0F - ((pixel_y - 0.5F) - pixel_y0);
				float x1_percent = ((pixel_x - 0.5F) - pixel_x0);
				float y1_percent = ((pixel_y - 0.5F) - pixel_y0);

				//
				//	Calculate what value each corner will contribute
				//
				float z0_part = (x0_percent * y0_percent) * z0;
				float z1_part = (x1_percent * y0_percent) * z1;
				float z2_part = (x1_percent * y1_percent) * z2;
				float z3_part = (x0_percent * y1_percent) * z3;

				//
				//	Sum the contributions from each of the four corners
				// of the grid cell to form the height at this point.
				//
				z_value = (z0_part + z1_part + z2_part + z3_part) / 255.0F;
				z_value *= scale;
			}

			//
			//	Get the vertex at this position and re-assign its new Z value
			//
			Vector3 &vertex_pos	= Grid[start_index++];
			vertex_pos.Z			= z_value;
		}
	}

	//
	//	Now, recalculate the rendering information
	//
	Update_Normals (0, 0, GridPointsX - 1, GridPointsY - 1);
	Update_Patch_Pos_And_Normals (0, 0, GridPointsX - 1, GridPointsY - 1);
	Update_UVs ();
	Update_Texture_Quad_List (0, 0, GridPointsX - 1, GridPointsY - 1);
	Assign_Unique_IDs ();

	//
	//	Free the bitmap buffer
	//
	SAFE_DELETE (bmp_bits);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Allocate_Patches
//
////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Allocate_Patches (void)
{
	PatchGridPointsX	= 64;
	PatchGridPointsY	= 64;
	PatchWidth			= PatchGridPointsX * Density;
	PatchHeight			= PatchGridPointsY * Density;

	//
	//	Determine how many patches will compose this grid
	//
	PatchesX = int(((float)GridPointsX / (float)PatchGridPointsX) + 0.5F);
	PatchesY = int(((float)GridPointsY / (float)PatchGridPointsY) + 0.5F);
	PatchesX = std::max (1, PatchesX);
	PatchesY = std::max (1, PatchesY);

	//
	//	Allocate the grid of patches
	//
	int patch_count	= PatchesX * PatchesY;
	PatchGrid			= new RenegadeTerrainPatchClass *[patch_count];
	PhysObjGrid			= new StaticPhysClass *[patch_count];
	for (int index = 0; index < patch_count; index ++) {
		
		//
		//	Create this patch
		//
		PatchGrid[index] = new RenegadeTerrainPatchClass;
		PatchGrid[index]->Allocate (PatchGridPointsX, PatchGridPointsY, Density);
		
		//
		//	Calculate the coordinate for this patch
		//
		int patch_y_pos = (index / PatchesX);
		int patch_x_pos = index - (patch_y_pos * PatchesX);
		
		//
		//	Let the patch know what its bounding box is...
		//
		float patch_min_x = patch_x_pos * (PatchWidth - Density);
		float patch_min_y = patch_y_pos * (PatchHeight - Density);
		float patch_max_x = (patch_x_pos + 1) * (PatchWidth - Density);
		float patch_max_y = (patch_y_pos + 1) * (PatchHeight - Density);
		PatchGrid[index]->Set_Bounding_Box_Min (Vector3 (patch_min_x, patch_min_y, 0.0F));
		PatchGrid[index]->Set_Bounding_Box_Max (Vector3 (patch_max_x, patch_max_y, 0.0F));
		
		//
		// Create the physics object for this heightfield
		//
		PhysObjGrid[index] = new StaticPhysClass;
		PhysObjGrid[index]->Set_Model (PatchGrid[index]);
		PhysObjGrid[index]->Set_Transform (Matrix3D (1));		

		//
		//	Add the heightfield to the scene
		//
		::Get_Scene_Editor ()->Add_Static_Object (PhysObjGrid[index]);
	}

	//
	//	Now, recompute the overall dimensions of the heightfield
	//
	GridPointsX		= (PatchesX * PatchGridPointsX) - (PatchesX - 1);
	GridPointsY		= (PatchesY * PatchGridPointsY) - (PatchesY - 1);
	GridPointCount	= GridPointsX * GridPointsY;
	Width				= GridPointsX * Density;
	Height			= GridPointsY * Density;

	//
	//	Force lighting to be updated...
	//
	::Get_Scene_Editor ()->Update_Lighting ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Patch_Pos_And_Normals
//
////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Update_Patch_Pos_And_Normals
(
	int min_x,
	int min_y,
	int max_x,
	int max_y
)
{
	//
	//	Determine which patches this operation would possibly affect
	//
	int min_patch_x = min_x / (PatchGridPointsX - 1);
	int min_patch_y = min_y / (PatchGridPointsY - 1);
	int max_patch_x = max_x / (PatchGridPointsX - 1);
	int max_patch_y = max_y / (PatchGridPointsY - 1);
	max_patch_x = std::min (max_patch_x, (PatchesX - 1));
	max_patch_y = std::min (max_patch_y, (PatchesY - 1));

	//
	//	Loop over all the patches (by rows)
	//
	for (int y_pos = min_patch_y; y_pos <= max_patch_y; y_pos ++) {

		//
		//	Reset the vertical vertex patch
		//
		int min_vert_y = y_pos * (PatchGridPointsY - 1);
		int max_vert_y = (y_pos + 1) * (PatchGridPointsY - 1);
		
		for (int x_pos = min_patch_x; x_pos <= max_patch_x; x_pos ++) {

			//
			//	Reset the horizontal vertex patch
			//
			int min_vert_x = x_pos * (PatchGridPointsX - 1);
			int max_vert_x = (x_pos + 1) * (PatchGridPointsX - 1);
			
			//
			//	Determine which patch this is
			//
			int patch_index = (y_pos * PatchesX) + x_pos;					

			//
			//	Loop over the verts in this patch
			//
			for (int vert_y = min_vert_y; vert_y <= max_vert_y; vert_y ++) {
				for (int vert_x = min_vert_x; vert_x <= max_vert_x; vert_x ++) {
					
					//
					//	Get the position and normal at this vertex
					//
					const Vector3 &pos		= Grid[Grid_Index (vert_x, vert_y)];
					const Vector3 &normal	= GridNormals[Grid_Index (vert_x, vert_y)];

					//
					//	Determine what vertex index inside the patch we are changing
					//
					int patch_vert_x = vert_x - min_vert_x;
					int patch_vert_y = vert_y - min_vert_y;
					
					//
					//	Update the vertex position and normal inside the patch
					//
					PatchGrid[patch_index]->Set_Vertex_Pos (patch_vert_x, patch_vert_y, pos);
					PatchGrid[patch_index]->Set_Vertex_Normal (patch_vert_x, patch_vert_y, normal);
				}
			}

			//
			//	Update the culling box for this patch
			//
			PhysObjGrid[patch_index]->Update_Cull_Box ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Free_Patches
//
////////////////////////////////////////////////////////////////
void
EditableHeightfieldClass::Free_Patches (void)
{
	int patch_count = PatchesX * PatchesY;

	//
	//	Release our hold on each patch in the grid
	//
	for (int index = 0; index < patch_count; index ++) {				
		::Get_Scene_Editor ()->Remove_Object (PhysObjGrid[index]);
		REF_PTR_RELEASE (PhysObjGrid[index]);
		REF_PTR_RELEASE (PatchGrid[index]);
	}

	//
	//	Free the grid itself
	//
	SAFE_DELETE_ARRAY (PatchGrid);
	SAFE_DELETE_ARRAY (PhysObjGrid);
	PatchGrid	= NULL;
	PhysObjGrid	= NULL;

	PatchesX				= 0;
	PatchesY				= 0;
	PatchGridPointsX	= 0;
	PatchGridPointsY	= 0;
	PatchWidth			= 0;
	PatchHeight			= 0;
	return ;
}
