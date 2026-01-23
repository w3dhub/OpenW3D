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
 *                 Project Name : wwphys																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/renegadeterrainpatch.cpp              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/12/02 2:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadeterrainpatch.h"

#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"
#include "sortingrenderer.h"
#include "rinfo.h"
#include "camera.h"
#include "dx8fvf.h"
#include "vector2i.h"
#include "terrainmaterial.h"
#include "renegadeterrainmaterialpass.h"
#include "persistfactory.h"
#include "ww3dids.h"
#include "wwhack.h"
#include "inttest.h"
#include "matpass.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//	WWHacks
////////////////////////////////////////////////////////////////
DECLARE_FORCE_LINK(RenegadeTerrainPatch);


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x02261040,
	CHUNKID_HEIGHTS,
	CHUNKID_NORMALS,
	CHUNKID_MATERIAL_LAYERS,
	CHUNKID_MATERIAL_LAYER,
	CHUNKID_QUAD_FLAGS,
	CHUNKID_VERTEX_COLORS,
};

enum
{
	VARID_GRID_PTS_X			= 0x01,
	VARID_GRID_PTS_Y,
	VARID_GRID_PT_COUNT,
	VARID_GRID_BBOX_MIN,
	VARID_GRID_BBOX_MAX,
	VARID_GRID_HEIGHT,
	VARID_GRID_DENSITY,
	VARID_MAX_TEXTURE_PASSES,
	VARID_IS_PRELIT,
};


//////////////////////////////////////////////////////////////////////
// PersistFactory for RenegadeTerrainPatchClass
//////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<RenegadeTerrainPatchClass, WW3D_PERSIST_CHUNKID_RENEGADE_TERRAIN>	_TerrainPatchFactory;


//////////////////////////////////////////////////////////////////////
//
//	RenegadeTerrainPatchClass
//
//////////////////////////////////////////////////////////////////////
RenegadeTerrainPatchClass::RenegadeTerrainPatchClass (void)	:
	Density (1.0F),
	GridPointsX (0),
	GridPointsY (0),
	GridPointCount (0),
	Grid (NULL),
	GridNormals (NULL),
	VertexColors (NULL),
	QuadFlags (NULL),
	BaseMaterial (NULL),
	LayerMaterial (NULL),
	BaseShader (0),
	LayerShader (0),
	BoundingBoxMin (0, 0, 0),
	BoundingBoxMax (0, 0, 0),
	AreBuffersDirty (true),
	IsPreLit (false)
{
	Initialize ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	RenegadeTerrainPatchClass
//
//////////////////////////////////////////////////////////////////////
RenegadeTerrainPatchClass::RenegadeTerrainPatchClass (const RenegadeTerrainPatchClass &src)	:
	Density (1.0F),
	GridPointsX (0),
	GridPointsY (0),
	GridPointCount (0),
	Grid (NULL),
	GridNormals (NULL),
	VertexColors (NULL),
	QuadFlags (NULL),
	BaseMaterial (NULL),
	LayerMaterial (NULL),
	BaseShader (0),
	LayerShader (0),
	BoundingBoxMin (0, 0, 0),
	BoundingBoxMax (0, 0, 0),
	AreBuffersDirty (true),
	IsPreLit (false),
	RenderObjClass (src)
{
	Initialize ();
	*this = src;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~RenegadeTerrainPatchClass
//
//////////////////////////////////////////////////////////////////////
RenegadeTerrainPatchClass::~RenegadeTerrainPatchClass (void)
{
	REF_PTR_RELEASE (BaseMaterial);
	REF_PTR_RELEASE (LayerMaterial);
	
	Free_Rendering_Buffers ();
	Free_Grid ();
	Free_Materials ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	operator=
//
//////////////////////////////////////////////////////////////////////
const RenegadeTerrainPatchClass &
RenegadeTerrainPatchClass::operator= (const RenegadeTerrainPatchClass &)
{
	return *this;
}



//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Initialize (void)
{
	Set_Collision_Type (COLLISION_TYPE_PHYSICAL | COLLISION_TYPE_PROJECTILE | COLLISION_TYPE_CAMERA);

	//
	//	Initialize the material settings
	//
	Initialize_Material ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Allocate
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Allocate (int points_x, int points_y, float meters_per_point)
{
	Free_Grid ();

	//
	//	Calculate how many grid points we will have in the x and y directions
	//
	Density			= meters_per_point;
	GridPointsX		= points_x;
	GridPointsY		= points_y;
	GridPointsX		= std::max (1, GridPointsX);
	GridPointsY		= std::max (1, GridPointsY);	
	GridPointCount	= (GridPointsX * GridPointsY);
	BoundingBoxMin.Z	= 0.0F;
	BoundingBoxMax.Z	= 0.0F;

	//
	//	Allocate and initialize the grid and supporting data structures
	//
	Allocate_Grid ();

	Invalidate_Cached_Bounding_Volumes();

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Allocate_Grid
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Allocate_Grid (void)
{
	//
	//	Allocate the per-vertex arrays
	//
	GridNormals			= new Vector3[GridPointCount];
	Grid					= new Vector3[GridPointCount];
	VertexColors		= new Vector3[GridPointCount];
	
	//
	//	Initialize the vertex color array to white
	//
	for (int index = 0; index < GridPointCount; index ++) {
		VertexColors[index].Set (1.0F, 1.0F, 1.0F);
	}

	//
	//	Allocate and initiailze the array of quad flags
	//
	int quad_count	= (GridPointsX - 1) * (GridPointsY - 1);
	QuadFlags		= new uint8[quad_count];
	::memset (QuadFlags, 0, sizeof (uint8) * quad_count);

	AreBuffersDirty	= true;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Free_Grid
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Free_Grid (void)
{
	if (Grid != NULL) {
		delete [] Grid;
		Grid = NULL;
	}

	if (GridNormals != NULL) {
		delete [] GridNormals;
		GridNormals = NULL;
	}

	if (VertexColors != NULL) {
		delete [] VertexColors;
		VertexColors = NULL;
	}	

	if (QuadFlags != NULL) {
		delete [] QuadFlags;
		QuadFlags = NULL;
	}

	Free_Materials ();

	BoundingBoxMin.Z	= 0;
	BoundingBoxMax.Z	= 0;
	Density				= 1.0F;
	GridPointsX			= 0;
	GridPointsY			= 0;
	GridPointCount		= 0;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Render
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Render (RenderInfoClass &rinfo)
{
	//
	//	Make sure our vertex and index buffers are up to date.
	//
	if (AreBuffersDirty) {
		Update_Rendering_Buffers ();
		AreBuffersDirty = false;
	}

	if (IsPreLit) {
		BaseMaterial->Set_Ambient_Color_Source (VertexMaterialClass::COLOR1);
		BaseMaterial->Set_Diffuse_Color_Source (VertexMaterialClass::COLOR1);
	}

	//
	// Install the mesh'es transform.  NOTE, this could go wrong if someone changes the
	// transform between the time that the mesh is rendered and the time that the decal
	// mesh is rendered...  It shouldn't happen though.
	//
	DX8Wrapper::Set_Transform (D3DTS_WORLD, Get_Transform ());

	if (rinfo.light_environment != NULL) {
		DX8Wrapper::Set_Light_Environment (rinfo.light_environment);
	}

	//
	// If the object's inherent materials are not disabled, render the terrain
	//
	if ((rinfo.Current_Override_Flags() & RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY) == 0)
	{
		//
		//	Render the base passes first
		//
		int index;
		for (index = 0; index < MaterialPassList.Count (); index ++) {
			Render_By_Texture (index, RenegadeTerrainMaterialPassClass::PASS_BASE);
		}

		//
		//	Do a "z-bias" to offset the alpha polys by just a teeny bit.  This
		// avoids any z-fighting issues with the different passes.
		//
		//DX8Wrapper::Set_Pseudo_ZBias (1);

		//
		//	Next render the alpha passes
		//
		for (index = 0; index < MaterialPassList.Count (); index ++) {
			Render_By_Texture (index, RenegadeTerrainMaterialPassClass::PASS_ALPHA);
		}
	}

	//
	// Render the procedural material passes
	//
	for (int i=0; i<rinfo.Additional_Pass_Count(); i++) {
		
		MaterialPassClass * matpass = rinfo.Peek_Additional_Pass(i);
		Render_Procedural_Material_Pass(matpass);
	}

	//
	//	Reset the z-bias
	//
	//DX8Wrapper::Set_Pseudo_ZBias (0);

	//
	//	Reset the z-bias
	//
	//DX8Wrapper::Set_DX8_ZBias (0);

	return ;
}

//////////////////////////////////////////////////////////////////////
//
//	Render_Procedural_Material_Pass
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Render_Procedural_Material_Pass(MaterialPassClass * matpass)
{
#if 0
	if ((pass->Get_Cull_Volume() != NULL) && (MaterialPassClass::Is_Per_Polygon_Culling_Enabled())) {
		
		/*
		** Generate the APT 
		*/
		temp_apt.Delete_All(false);
			
		Matrix3D modeltminv;
		Get_Transform().Get_Orthogonal_Inverse(modeltminv);
		
		OBBoxClass localbox;
		OBBoxClass::Transform(modeltminv,*(pass->Get_Cull_Volume()),&localbox);

		Vector3 view_dir;
		localbox.Basis.Get_Z_Vector(&view_dir);
		view_dir = -view_dir;
			
		if (Model->Has_Cull_Tree()) {
			Model->Generate_Rigid_APT(localbox,view_dir,temp_apt);
		} else {
			Model->Generate_Rigid_APT(view_dir,temp_apt);
		}
	
		if (temp_apt.Count() > 0) {

			int buftype = BUFFER_TYPE_DYNAMIC_DX8;
			if (Model->Get_Flag(MeshGeometryClass::SORT) && WW3D::Is_Sorting_Enabled()) {
				buftype = BUFFER_TYPE_DYNAMIC_SORTING;
			}

			/*
			** Spew triangles in the APT into the dynamic index buffer
			*/
			int min_v = Model->Get_Vertex_Count();
			int max_v = 0;

			DynamicIBAccessClass dynamic_ib(buftype,temp_apt.Count() * 3);
			{
				DynamicIBAccessClass::WriteLockClass lock(&dynamic_ib);
				unsigned short * indices = lock.Get_Index_Array();
				const TriIndex * polys = Model->Get_Polygon_Array();

				for (int i=0; i < temp_apt.Count(); i++)
				{
					unsigned v0 = polys[temp_apt[i]].I;
					unsigned v1 = polys[temp_apt[i]].J;
					unsigned v2 = polys[temp_apt[i]].K;

					indices[i*3 + 0] = (unsigned short)v0;
					indices[i*3 + 1] = (unsigned short)v1;
					indices[i*3 + 2] = (unsigned short)v2;

					min_v = WWMath::Min(v0,min_v);
					min_v = WWMath::Min(v1,min_v);
					min_v = WWMath::Min(v2,min_v);

					max_v = WWMath::Max(v0,max_v);
					max_v = WWMath::Max(v1,max_v);
					max_v = WWMath::Max(v2,max_v);
				}
			}

			/*
			** Render
			*/
			int vertex_offset = PolygonRendererList.Peek_Head()->Get_Vertex_Offset();
			pass->Install_Materials();
			
			DX8Wrapper::Set_Transform(D3DTS_WORLD,Get_Transform());
			DX8Wrapper::Set_Index_Buffer(dynamic_ib,vertex_offset);

			DX8Wrapper::Draw_Triangles(
				0,
				temp_apt.Count(),
				min_v,
				max_v-min_v+1);
		}
	} else {		
#endif		
		/*
		** Normal mesh case, render polys with this mesh's transform
		*/
		matpass->Install_Materials();		

		//
		//	Render the base passes first
		//
		for (int index = 0; index < MaterialPassList.Count (); index ++) {
			Submit_Rendering_Buffers (index, RenegadeTerrainMaterialPassClass::PASS_BASE);

			//
			//	Alias some data
			//
			DynamicVectorClass<int> &vert_list  = MaterialPassList[index]->VertexRenderList[RenegadeTerrainMaterialPassClass::PASS_BASE];
			DynamicVectorClass<int> &quad_list	= MaterialPassList[index]->QuadList[RenegadeTerrainMaterialPassClass::PASS_BASE];

			//
			//	Determine how many polygons and verts to render
			//
			int quad_count = quad_list.Count ();
			int vert_count = vert_list.Count ();
			int poly_count	= quad_count * 2;

			//
			//	Draw the mesh!
			//
			DX8Wrapper::Draw_Triangles (BUFFER_TYPE_DYNAMIC_DX8, 0, poly_count, 0, vert_count);
		}
//	}
}


//////////////////////////////////////////////////////////////////////
//
//	Submit_Rendering_Buffers
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Submit_Rendering_Buffers (int texture_index, int pass_type)
{
	//
	//	Don't render this layer if there isn't anything to render!
	//
	if (	MaterialPassList[texture_index]->VertexBuffers[pass_type] == NULL ||
			MaterialPassList[texture_index]->IndexBuffers[pass_type] == NULL)
	{
		return ;
	}

	//
	// Set vertex and index buffers
	//
	DX8Wrapper::Set_Vertex_Buffer (MaterialPassList[texture_index]->VertexBuffers[pass_type]);
	DX8Wrapper::Set_Index_Buffer (MaterialPassList[texture_index]->IndexBuffers[pass_type], 0);	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Render_By_Texture
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Render_By_Texture (int texture_index, int pass_type)
{
	//
	//	Don't render this layer if there isn't anything to render!
	//
	if (	MaterialPassList[texture_index]->VertexBuffers[pass_type] == NULL ||
			MaterialPassList[texture_index]->IndexBuffers[pass_type] == NULL)
	{
		return ;
	}

	//
	//	Pass the vertex and index buffers onto DX8
	//
	Submit_Rendering_Buffers (texture_index, pass_type);

	//
	//	Configure the texture
	//
	DX8Wrapper::Set_Texture (0, MaterialPassList[texture_index]->Material->Peek_Texture ());
	DX8Wrapper::Set_Texture (1, NULL);

	//
	//	Configure the material and shader
	//
	if (pass_type == RenegadeTerrainMaterialPassClass::PASS_BASE) {
		DX8Wrapper::Set_Material (BaseMaterial);
		DX8Wrapper::Set_Shader (BaseShader);
	} else {
		DX8Wrapper::Set_Material (LayerMaterial);
		DX8Wrapper::Set_Shader (LayerShader);
	}

	//
	//	Alias some data
	//
	DynamicVectorClass<int> &vert_list  = MaterialPassList[texture_index]->VertexRenderList[pass_type];
	DynamicVectorClass<int> &quad_list	= MaterialPassList[texture_index]->QuadList[pass_type];

	//
	//	Determine how many polygons and verts to render
	//
	int quad_count = quad_list.Count ();
	int vert_count = vert_list.Count ();
	int poly_count	= quad_count * 2;

	//
	//	Draw the mesh!
	//
	DX8Wrapper::Draw_Triangles (BUFFER_TYPE_DYNAMIC_DX8, 0, poly_count, 0, vert_count);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Free_Rendering_Buffers
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Free_Rendering_Buffers (void)
{
	//
	//	Free each rendering layer
	//
	for (int index = 0; index < MaterialPassList.Count (); index ++) {
		for (int pass = 0; pass < RenegadeTerrainMaterialPassClass::PASS_COUNT; pass ++) {
			REF_PTR_RELEASE (MaterialPassList[index]->IndexBuffers[pass]);
			REF_PTR_RELEASE (MaterialPassList[index]->VertexBuffers[pass]);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Update_Rendering_Buffers
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Update_Rendering_Buffers (void)
{
	Free_Rendering_Buffers ();

	//
	//	Build the rendering buffers for each layer
	//
	for (int index = 0; index < MaterialPassList.Count (); index ++) {
		Build_Rendering_Buffers (index, RenegadeTerrainMaterialPassClass::PASS_BASE);
		Build_Rendering_Buffers (index, RenegadeTerrainMaterialPassClass::PASS_ALPHA);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Build_Rendering_Buffers
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Build_Rendering_Buffers (int texture_index, int pass_type)
{
	//
	//	Alias some data
	//
	RenegadeTerrainMaterialPassClass *material_pass = MaterialPassList[texture_index];
	DynamicVectorClass<int> &vert_list  = material_pass->VertexRenderList[pass_type];
	DynamicVectorClass<int> &quad_list	= material_pass->QuadList[pass_type];
	int *vertex_index_map					= material_pass->VertexIndexMap[pass_type];

	//
	//	Determine the size of our data
	//
	int quad_count = quad_list.Count ();
	int vert_count = vert_list.Count ();
	int poly_count	= quad_count * 2;

	//
	//	Don't build the buffers if there's nothing to render in this layer
	//
	if (poly_count == 0 || vert_count == 0) {
		return ;
	}

	//
	//	Allocate the vertex and index buffers
	//
	material_pass->IndexBuffers[pass_type]		= new DX8IndexBufferClass (poly_count * 3);
	material_pass->VertexBuffers[pass_type]	= new DX8VertexBufferClass (DX8_FVF_XYZNDUV1, vert_count);

	//
	// Write index data to index buffers
	//
	{ // scope for lock

		int col_count = (GridPointsX - 1);

		//
		//	Lock the index buffer
		//
		IndexBufferClass::WriteLockClass lock (material_pass->IndexBuffers[pass_type]);
		unsigned short * indices = lock.Get_Index_Array();

		//
		//	Now, compose the triangles by indexing the verts into the vertex buffer
		//
		int ib_index			= 0;
		for (int index = 0; index < quad_count; index ++) {

			//
			//	Determine which quad we're rendering
			//
			int quad_index = quad_list[index];
			int quad_y_pos = (quad_index / col_count);
			int quad_x_pos = quad_index - (quad_y_pos * col_count);
			
			//
			//	Determine the "starting" vertex index from the current quad
			//
			int curr_src_index = (quad_y_pos * GridPointsX) + quad_x_pos;
			
			//
			//	Calculate the 4 vertex indices that compose this quad
			//
			int v0_index = curr_src_index;
			int v1_index = curr_src_index + 1;
			int v2_index = curr_src_index + GridPointsX + 1;
			int v3_index = curr_src_index + GridPointsX;

			//
			//	Add the current quad to the index buffer
			//
			indices[ib_index ++] = (unsigned short)vertex_index_map[v0_index];
			indices[ib_index ++] = (unsigned short)vertex_index_map[v2_index];
			indices[ib_index ++] = (unsigned short)vertex_index_map[v3_index];

			indices[ib_index ++] = (unsigned short)vertex_index_map[v2_index];
			indices[ib_index ++] = (unsigned short)vertex_index_map[v0_index];
			indices[ib_index ++] = (unsigned short)vertex_index_map[v1_index];
		}

	} // end scope for lock

	const Vector3 white (1.0F, 1.0F, 1.0F);

	{
		//
		//	Lock the vertex buffer
		//
		VertexBufferClass::WriteLockClass lock (material_pass->VertexBuffers[pass_type]);
		VertexFormatXYZNDUV1 *vertices = (VertexFormatXYZNDUV1 *)lock.Get_Vertex_Array ();

		//
		//	Specify some default values
		//
		const static Vector3 default_normal (0.0F, 0.0F, 0.0F);
		const static Vector2 default_uv (0.0F, 0.0F);
		
		//
		//	Write each vertex's definition to the dynamic vertex buffer
		//
		for (int index = 0; index < vert_count; index ++) {

			int vert_index = vert_list[index];

			//
			//	Set the vertex position and normal
			//
			vertices[index].x	= Grid[vert_index].X;
			vertices[index].y	= Grid[vert_index].Y;
			vertices[index].z	= Grid[vert_index].Z;
			vertices[index].nx	= GridNormals[vert_index].X;
			vertices[index].ny	= GridNormals[vert_index].Y;
			vertices[index].nz	= GridNormals[vert_index].Z;

			//
			//	Set the UV mapping
			//
			vertices[index].u1	= material_pass->GridUVs[vert_index].X;
			vertices[index].v1	= material_pass->GridUVs[vert_index].Y;

			//
			//	Set the vertex color
			//
			if (pass_type == RenegadeTerrainMaterialPassClass::PASS_BASE) {
				vertices[index].diffuse = DX8Wrapper::Convert_Color (VertexColors[vert_index], 1.0F);
			} else {

				//
				//	Compose a vertex color using the vertex alpha
				//
				float alpha					= material_pass->VertexAlpha[vert_index];
				vertices[index].diffuse	= DX8Wrapper::Convert_Color (VertexColors[vert_index], alpha);
			}
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
RenegadeTerrainPatchClass::Initialize_Material (void)
{
	//
	//	Allocate the vertex material
	//
	WWASSERT (BaseMaterial == NULL);
	BaseMaterial = NEW_REF(VertexMaterialClass, ());
	BaseMaterial->Set_Ambient (1.0F, 1.0F, 1.0F);
	BaseMaterial->Set_Diffuse (1.0F, 1.0F, 1.0F);
	BaseMaterial->Set_Specular (0, 0, 0);
	BaseMaterial->Set_Emissive (0.0F, 0.0F, 0.0F);
	BaseMaterial->Set_Opacity (1.0F);
	BaseMaterial->Set_Shininess (0.0F);
	BaseMaterial->Set_Lighting (true);

	LayerMaterial = NEW_REF(VertexMaterialClass, ());
	LayerMaterial->Set_Ambient (1.0F, 1.0F, 1.0F);
	LayerMaterial->Set_Diffuse (1.0F, 1.0F, 1.0F);
	LayerMaterial->Set_Specular (0, 0, 0);
	LayerMaterial->Set_Emissive (0.0F, 0.0F, 0.0F);
	LayerMaterial->Set_Opacity (1.0F);
	LayerMaterial->Set_Shininess (0.0F);
	LayerMaterial->Set_Lighting (true);
	LayerMaterial->Set_Ambient_Color_Source (VertexMaterialClass::COLOR1);
	LayerMaterial->Set_Diffuse_Color_Source (VertexMaterialClass::COLOR1);

	//
	//	Initialize the shader
	//
	BaseShader	= ShaderClass::_PresetOpaqueShader;
	LayerShader	= ShaderClass::_PresetAlphaShader;

	BaseShader.Set_Fog_Func(ShaderClass::FOG_ENABLE);
	LayerShader.Set_Fog_Func(ShaderClass::FOG_ENABLE);

	return ;	
}


//////////////////////////////////////////////////////////////////////
//
//	Free_Materials
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Free_Materials (void)
{
	//
	//	Free each material pass
	//
	for (int index = 0; index < MaterialPassList.Count (); index ++) {
		delete MaterialPassList[index];
	}
	
	MaterialPassList.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Obj_Space_Bounding_Sphere
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Get_Obj_Space_Bounding_Sphere (SphereClass &sphere) const
{	
	float delta_x = BoundingBoxMax.X - BoundingBoxMin.X;
	float delta_y = BoundingBoxMax.Y - BoundingBoxMin.Y;
	float delta_z = BoundingBoxMax.Z - BoundingBoxMin.Z;
	sphere.Center.X = BoundingBoxMin.X + (delta_x * 0.5F);
	sphere.Center.Y = BoundingBoxMin.Y + (delta_y * 0.5F);
	sphere.Center.Z = BoundingBoxMin.Z + (delta_z * 0.5F);

	//
	//	Determine which radius to use as the largest delta
	//
	sphere.Radius = std::max (delta_x, delta_y);
	sphere.Radius = std::max (sphere.Radius, delta_z);
	sphere.Radius += 1.0F;
	//sphere.Radius *= 2.0F;

//	sphere.Center.Set (0, 0, 0);
//	sphere.Radius = 1000.0F;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Obj_Space_Bounding_Box
//
//////////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Get_Obj_Space_Bounding_Box (AABoxClass &box) const
{
	float delta_x = BoundingBoxMax.X - BoundingBoxMin.X;
	float delta_y = BoundingBoxMax.Y - BoundingBoxMin.Y;
	float delta_z = BoundingBoxMax.Z - BoundingBoxMin.Z;
	box.Center.X = BoundingBoxMin.X + (delta_x * 0.5F);
	box.Center.Y = BoundingBoxMin.Y + (delta_y * 0.5F);
	box.Center.Z = BoundingBoxMin.Z + (delta_z * 0.5F);

	//
	//	Fill in the extents of this box (really they are half-extents)
	//
	box.Extent.X = (delta_x) + 1.0F;
	box.Extent.Y = (delta_y) + 1.0F;
	box.Extent.Z = (delta_z) + 1.0F;
	
//	box.Center.Set (0, 0, 0);;
//	box.Extent.Set (500.0F, 500.0F, 500.0F);	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Cast_OBBox
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainPatchClass::Cast_OBBox (OBBoxCollisionTestClass &boxtest)
{
	//
	//	Skip this object if it doesn't match the collision type
	//
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) {
		return false;
	}

	if (boxtest.Result->StartBad) {
		return false;
	}	

	//
	//	Get the world to object space transform
	//
	Matrix3D world_to_obj_tm;
	Get_Inverse_Transform (world_to_obj_tm);

	//
	//	Transform the OBBox into heightfield space
	//
	OBBoxClass obj_space_box;
	OBBoxClass::Transform (world_to_obj_tm, boxtest.Box, &obj_space_box);

	//
	//	Transform the movement vector into heightfield space
	//
	Vector3 obj_space_move = world_to_obj_tm * boxtest.Move;

	//
	//	Create a new coliision box test object in heightfield space
	//
	OBBoxCollisionTestClass obj_space_test (obj_space_box, obj_space_move,
										boxtest.Result, boxtest.CollisionType);
	
	//
	//	Calculate what grid cells this box is "possibly" intersecting
	//
	int min_x = Get_Quad_Index_X (obj_space_test.SweepMin.X);
	int min_y = Get_Quad_Index_Y (obj_space_test.SweepMin.Y);
	int max_x = Get_Quad_Index_X (obj_space_test.SweepMax.X);
	int max_y = Get_Quad_Index_Y (obj_space_test.SweepMax.Y);

	int closest_index = 0;

	//
	//	Now check all the quads in this region
	//
	bool retval = false;
	for (int y_pos = min_y; y_pos <= max_y; y_pos ++) {
		for (int x_pos = min_x; x_pos <= max_x; x_pos ++) {

			int start_index = Grid_Index (x_pos, y_pos);
			
			int v0_index = start_index;
			int v1_index = start_index + 1;
			int v2_index = start_index + GridPointsX + 1;
			int v3_index = start_index + GridPointsX;
			
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
			//	Do the collision test to determine if the box intersects either of these triangles
			//
			float percent = obj_space_test.Result->Fraction;
			retval |= CollisionMath::Collide (obj_space_test.Box, obj_space_test.Move, tri1, Vector3 (0, 0, 0), obj_space_test.Result);
			retval |= CollisionMath::Collide (obj_space_test.Box, obj_space_test.Move, tri2, Vector3 (0, 0, 0), obj_space_test.Result);

			//
			//	Is this the closest collision yet?
			//
			if (obj_space_test.Result->Fraction < percent) {
				closest_index = v0_index;
			}
		}
	}

	//
	//	Make sure to return our pointer to the caller
	//
	if (retval) {

		//
		//	Determine which surface type was hit
		//
		if (MaterialPassList.Count () > 0) {
			int best_pass		= 0;
			float best_alpha	= 0;
			for (int index = 0; index < MaterialPassList.Count (); index ++) {
				float alpha = MaterialPassList[index]->VertexAlpha[closest_index];
				if (alpha > best_alpha && alpha != 1.0F) {
					best_alpha	= alpha;
					best_pass	= index;
				}
			}
			if (MaterialPassList[best_pass]->Material != NULL) {
				boxtest.Result->SurfaceType = MaterialPassList[best_pass]->Material->Get_Surface_Type ();
			}
		}

		
		//
		//	Transform the result back into world-space
		//
		if (boxtest.Result->ComputeContactPoint) {
			boxtest.Result->ContactPoint = Get_Transform () * boxtest.Result->ContactPoint;
		}

		boxtest.CollidedRenderObj = this;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Intersect_AABox
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainPatchClass::Intersect_AABox (AABoxIntersectionTestClass &boxtest)
{
	//
	//	Skip this object if it doesn't match the collision type
	//
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) {
		return false;
	}

	//
	//	First, transform the box into heightfield (object) space
	//
	Matrix3D world_to_obj_tm;
	Get_Inverse_Transform (world_to_obj_tm);

	AABoxClass obj_space_box;
	AABoxClass::Transform (world_to_obj_tm, boxtest.Box, &obj_space_box);
	
	//
	//	Calculate what grid cells this box is "possibly" intersecting
	//
	int min_x = Get_Quad_Index_X (obj_space_box.Center.X - obj_space_box.Extent.X);
	int min_y = Get_Quad_Index_Y (obj_space_box.Center.Y - obj_space_box.Extent.Y);
	int max_x = Get_Quad_Index_X (obj_space_box.Center.X + obj_space_box.Extent.X);
	int max_y = Get_Quad_Index_Y (obj_space_box.Center.Y + obj_space_box.Extent.Y);

	//
	//	Now check all the quads in this region
	//
	bool retval = false;
	for (int y_pos = min_y; y_pos <= max_y; y_pos ++) {
		for (int x_pos = min_x; x_pos <= max_x; x_pos ++) {

			int start_index	= Grid_Index (x_pos, y_pos);
			int quad_index		= y_pos * (GridPointsX - 1) + x_pos;

			//
			//	Skip this quad if its hidden
			//
			if (QuadFlags[quad_index] & QF_HIDDEN) {
				continue;				
			}
			
			int v0_index = start_index;
			int v1_index = start_index + 1;
			int v2_index = start_index + GridPointsX + 1;
			int v3_index = start_index + GridPointsX;
			
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
			//	Do the collision test to determine if the box intersects either of these triangles
			//
			retval |= (CollisionMath::Overlap_Test (obj_space_box, tri1) != CollisionMath::OUTSIDE);
			retval |= (CollisionMath::Overlap_Test (obj_space_box, tri2) != CollisionMath::OUTSIDE);
		}
	}

	//
	//	Make sure to return our pointer to the caller
	//
	/*if (retval) {
		
		boxtest.CollidedRenderObj = this;
	}*/

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Intersect_OBBox
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainPatchClass::Intersect_OBBox (OBBoxIntersectionTestClass &boxtest)
{
	//
	//	Skip this object if it doesn't match the collision type
	//
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) {
		return false;
	}

	//
	//	First, transform the box into heightfield (object) space
	//
	Matrix3D world_to_obj_tm;
	Get_Inverse_Transform (world_to_obj_tm);

	OBBoxClass obj_space_box;
	OBBoxClass::Transform (world_to_obj_tm, boxtest.Box, &obj_space_box);

	OBBoxIntersectionTestClass obj_space_test (obj_space_box, boxtest.CollisionType);
	const AABoxClass &obj_space_aabox = obj_space_test.BoundingBox;
	
	//
	//	Calculate what grid cells this box is "possibly" intersecting
	//
	int min_x = Get_Quad_Index_X (obj_space_aabox.Center.X - obj_space_aabox.Extent.X);
	int min_y = Get_Quad_Index_Y (obj_space_aabox.Center.Y - obj_space_aabox.Extent.Y);
	int max_x = Get_Quad_Index_X (obj_space_aabox.Center.X + obj_space_aabox.Extent.X);
	int max_y = Get_Quad_Index_Y (obj_space_aabox.Center.Y + obj_space_aabox.Extent.Y);

	//
	//	Now check all the quads in this region
	//
	bool retval = false;
	for (int y_pos = min_y; y_pos <= max_y; y_pos ++) {
		for (int x_pos = min_x; x_pos <= max_x; x_pos ++) {

			int start_index = Grid_Index (x_pos, y_pos);
			
			int v0_index = start_index;
			int v1_index = start_index + 1;
			int v2_index = start_index + GridPointsX + 1;
			int v3_index = start_index + GridPointsX;
			
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
			//	Do the collision test to determine if the box intersects either of these triangles
			//
			retval |= (CollisionMath::Overlap_Test (obj_space_box, tri1) != CollisionMath::OUTSIDE);
			retval |= (CollisionMath::Overlap_Test (obj_space_box, tri2) != CollisionMath::OUTSIDE);
		}
	}

	//
	//	Make sure to return our pointer to the caller
	//
	/*if (retval) {
		
		boxtest.CollidedRenderObj = this;
	}*/

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Cast_AABox
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainPatchClass::Cast_AABox (AABoxCollisionTestClass &boxtest)
{
	//
	//	Skip this object if it doesn't match the collision type
	//
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) {
		return false;
	}

	if (boxtest.Result->StartBad) {
		return false;
	}	

	//
	//	First, transform the boxtest into heightfield (object) space
	//
	Matrix3D world_to_obj_tm;
	Get_Inverse_Transform (world_to_obj_tm);
	boxtest.Transform (world_to_obj_tm);
	
	//
	//	Calculate what grid cells this box is "possibly" intersecting
	//
	int min_x = Get_Quad_Index_X (boxtest.SweepMin.X);
	int min_y = Get_Quad_Index_Y (boxtest.SweepMin.Y);
	int max_x = Get_Quad_Index_X (boxtest.SweepMax.X);
	int max_y = Get_Quad_Index_Y (boxtest.SweepMax.Y);

	int closest_index = 0;

	//
	//	Now check all the quads in this region
	//
	bool retval = false;
	for (int y_pos = min_y; y_pos <= max_y; y_pos ++) {
		for (int x_pos = min_x; x_pos <= max_x; x_pos ++) {

			int start_index = Grid_Index (x_pos, y_pos);
			int quad_index		= y_pos * (GridPointsX - 1) + x_pos;

			//
			//	Skip this quad if its hidden
			//
			if (QuadFlags[quad_index] & QF_HIDDEN) {
				continue;				
			}
			
			int v0_index = start_index;
			int v1_index = start_index + 1;
			int v2_index = start_index + GridPointsX + 1;
			int v3_index = start_index + GridPointsX;
			
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
			//	Do the collision test to determine if the box intersects either of these triangles
			//
			float percent = boxtest.Result->Fraction;
			retval |= CollisionMath::Collide (boxtest.Box, boxtest.Move, tri1, boxtest.Result);
			retval |= CollisionMath::Collide (boxtest.Box, boxtest.Move, tri2, boxtest.Result);

			//
			//	Is this the closest collision yet?
			//
			if (boxtest.Result->Fraction < percent) {
				closest_index = v0_index;
			}
		}
	}

	//
	//	Make sure to return our pointer to the caller
	//
	if (retval) {

		//
		//	Determine which surface type was hit
		//
		if (MaterialPassList.Count () > 0) {
			int best_pass		= 0;
			float best_alpha	= 0;
			for (int index = 0; index < MaterialPassList.Count (); index ++) {
				float alpha = MaterialPassList[index]->VertexAlpha[closest_index];
				if (alpha > best_alpha && alpha != 1.0F) {
					best_alpha	= alpha;
					best_pass	= index;
				}
			}
			if (MaterialPassList[best_pass]->Material != NULL) {
				boxtest.Result->SurfaceType = MaterialPassList[best_pass]->Material->Get_Surface_Type ();
			}
		}
		
		//
		//	Transform the result back into world-space
		//
		boxtest.Transform (Get_Transform ());
		boxtest.CollidedRenderObj = this;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Cast_Ray
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainPatchClass::Cast_Ray (RayCollisionTestClass &raytest)
{
	//
	//	Skip this object if it doesn't match the collision type
	//
	if (	(Get_Collision_Type() & raytest.CollisionType) == 0 ||
			raytest.Result->StartBad)
	{
		return false;
	}

	bool retval = false;

	
#if 0
	CastResultStruct temp_result = (*raytest.Result);

	retval = Brute_Force_Cast_Ray (raytest);
	
	static bool do_it = false;
	if (do_it) {

do_it_start:

		CastResultStruct brute_result = (*raytest.Result);
		(*raytest.Result) = temp_result;
		bool retval2 = Cast_Non_Vertical_Ray (raytest);
		if (do_it && (retval != retval2 || brute_result.Fraction != raytest.Result->Fraction)) {
			(*raytest.Result) = temp_result;
			goto do_it_start;
		}
	}
#else

	retval = Cast_Non_Vertical_Ray (raytest);

#endif


	//
	//	Test for completely vertical ray
	//
	/*if (	raytest.Ray.Get_DP ().X == 0.0F &&
			raytest.Ray.Get_DP ().Y == 0.0F)
	{
		retval = Cast_Vertical_Ray (raytest);
	} else {
		retval = Cast_Non_Vertical_Ray (raytest);
	}*/
	
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Cast_Vertical_Ray
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainPatchClass::Cast_Vertical_Ray (RayCollisionTestClass &raytest)
{
	//
	//	Sanity check
	//
	if (raytest.Ray.Get_DP ().Z == 0) {
		return false;
	}

	bool retval = false;

	//
	//	First, transform the ray into heightfield (object) space
	//
	Matrix3D world_to_obj_tm;
	Get_Inverse_Transform (world_to_obj_tm);

	const Vector3 &world_p0 = raytest.Ray.Get_P0 ();
	const Vector3 &world_p1 = raytest.Ray.Get_P1 ();

	Vector3 p0;
	Vector3 p1;
	Matrix3D::Transform_Vector (world_to_obj_tm, world_p0, &p0);
	Matrix3D::Transform_Vector (world_to_obj_tm, world_p1, &p1);

	//
	//	Build a line segement from the transformed points that we'll use later
	// for collision detection.
	//
	LineSegClass line_seg (p0, p1);

	//
	//	Is this point somewhere over or under the grid?
	//
	if (	p0.X >= BoundingBoxMin.X && p0.Y >= BoundingBoxMin.Y &&
			p0.X <= BoundingBoxMax.X && p0.Y <= BoundingBoxMax.Y)
	{
		int quad_index_x = Get_Quad_Index_X (p0.X);
		int quad_index_y = Get_Quad_Index_Y (p0.Y);

		//
		//	Simply test this quad to see if the ray collides with it...
		//
		retval = Collide_Quad (line_seg, quad_index_x, quad_index_y, *raytest.Result);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Brute_Force_Cast_Ray
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainPatchClass::Brute_Force_Cast_Ray (RayCollisionTestClass &raytest)
{
	bool retval = false;

	//
	//	First, transform the ray into heightfield (object) space
	//
	Matrix3D world_to_obj_tm;
	Get_Inverse_Transform (world_to_obj_tm);

	const Vector3 &world_p0 = raytest.Ray.Get_P0 ();
	const Vector3 &world_p1 = raytest.Ray.Get_P1 ();

	Vector3 p0;
	Vector3 p1;
	Matrix3D::Transform_Vector (world_to_obj_tm, world_p0, &p0);
	Matrix3D::Transform_Vector (world_to_obj_tm, world_p1, &p1);

	//
	//	Build a line segement from the transformed points that we'll use later
	// for collision detection.
	//
	LineSegClass line_seg (p0, p1);

	//
	//	Now check all the quads in this region
	//
	for (int y_pos = 0; y_pos < (GridPointsY-1); y_pos ++) {
		for (int x_pos = 0; x_pos < (GridPointsX-1); x_pos ++) {

			int quad_index		= y_pos * (GridPointsX - 1) + x_pos;

			//
			//	Skip this quad if its hidden
			//
			if (QuadFlags[quad_index] & QF_HIDDEN) {
				continue;				
			}
			
			Collide_Quad (line_seg, x_pos,  y_pos, *raytest.Result);
		}
	}

	if (retval) {
		raytest.CollidedRenderObj = this;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Cast_Non_Vertical_Ray
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainPatchClass::Cast_Non_Vertical_Ray (RayCollisionTestClass &raytest)
{
	bool retval = false;

	//
	//	First, transform the ray into heightfield (object) space
	//
	Matrix3D world_to_obj_tm;
	Get_Inverse_Transform (world_to_obj_tm);

	const Vector3 &world_p0 = raytest.Ray.Get_P0 ();
	const Vector3 &world_p1 = raytest.Ray.Get_P1 ();

	Vector3 p0;
	Vector3 p1;
	Matrix3D::Transform_Vector (world_to_obj_tm, world_p0, &p0);
	Matrix3D::Transform_Vector (world_to_obj_tm, world_p1, &p1);

	//
	//	Build a line segement from the transformed points that we'll use later
	// for collision detection.
	//
	LineSegClass line_seg (p0, p1);
	const Vector3 &delta_p = line_seg.Get_DP ();

	//
	//	Calculate where on the grid we need to search
	//
	int start_cell_x	= Get_Quad_Index_X (p0.X);
	int start_cell_y	= Get_Quad_Index_Y (p0.Y);
	int end_cell_x		= Get_Quad_Index_X (p1.X);
	int end_cell_y		= Get_Quad_Index_Y (p1.Y);

	//
	//	Determine how many vertical grid lines to search
	//	
	int x_inc = 1;
	if (start_cell_x > end_cell_x) {
		x_inc = -1;
	}

	//
	//	Determine how many horizontal grid lines to search
	//	
	int y_inc = 1;
	if (start_cell_y > end_cell_y) {
		y_inc = -1;
	}

	int cells_x = ::abs (end_cell_x - start_cell_x);
	int cells_y = ::abs (end_cell_y - start_cell_y);

	//
	//	Determine if we need to offset the cell we're searching each
	// time we stop at a vertical or horizontal grid intersection
	//
	int cell_x_offset = 0;
	int cell_y_offset = 0;
	if (x_inc > 0) {
		cell_x_offset = -1;
	}
	if (y_inc > 0) {
		cell_y_offset = -1;
	}


	CastResultStruct vertical_result		= *(raytest.Result);
	CastResultStruct horizontal_result	= *(raytest.Result);
	bool did_vertical_collide				= false;
	bool did_horizontal_collide			= false;

	//
	//	Now, determine what the actual starting and ending cell coordinates are...
	//
	Vector2i cell_coord_p0 (start_cell_x, start_cell_y);
	Vector2i cell_coord_p1 (end_cell_x, end_cell_x);

	if (delta_p.X != 0.0F) {
		float clamped_p0_x = WWMath::Clamp (p0.X, BoundingBoxMin.X, BoundingBoxMax.X);
		float clamped_p1_x = WWMath::Clamp (p1.X, BoundingBoxMin.X, BoundingBoxMax.X);
		float clamped_p0_y = p0.Y + (delta_p.Y * ((clamped_p0_x - p0.X) / delta_p.X));
		float clamped_p1_y = p0.Y + (delta_p.Y * ((clamped_p1_x - p0.X) / delta_p.X));

		cell_coord_p0.Set (Get_Quad_Index_X (clamped_p0_x), Get_Quad_Index_Y (clamped_p0_y));
		cell_coord_p1.Set (Get_Quad_Index_X (clamped_p1_x), Get_Quad_Index_Y (clamped_p1_y));
	} else if (delta_p.Y != 0.0F) {
		float clamped_p0_y = WWMath::Clamp (p0.Y, BoundingBoxMin.Y, BoundingBoxMax.Y);
		float clamped_p1_y = WWMath::Clamp (p1.Y, BoundingBoxMin.Y, BoundingBoxMax.Y);
		float clamped_p0_x = p0.X + (delta_p.X * ((clamped_p0_y - p0.Y) / delta_p.Y));
		float clamped_p1_x = p0.X + (delta_p.X * ((clamped_p1_y - p0.Y) / delta_p.Y));

		cell_coord_p0.Set (Get_Quad_Index_X (clamped_p0_x), Get_Quad_Index_Y (clamped_p0_y));
		cell_coord_p1.Set (Get_Quad_Index_X (clamped_p1_x), Get_Quad_Index_Y (clamped_p1_y));
	}

	//
	//	Test quad of triangles in the starting cell to see if you need go no further.
	//
	if (Collide_Quad (line_seg, cell_coord_p0.I, cell_coord_p0.J, vertical_result)) {
		did_vertical_collide = true;
	} else {

		//
		//	Test vertical grid lines first
		//
		if (delta_p.X != 0.0F) {
			
			//
			//	Find the first intersection point moving along vertical grid lines
			//
			for (int curr_x = start_cell_x; cells_x >= 0; cells_x --, curr_x += x_inc) {
				
				//
				//	Determine where the ray intersects this grid line
				//
				float x_pos		= Get_Grid_Line_Pos_X (curr_x);
				float percent	= (x_pos - p0.X) / delta_p.X;
				
				//
				//	Don't test the cell if its outside the range of the line segment
				//
				if (percent >= 0 && percent <= 1.0F) {
					float y_pos	= p0.Y + (delta_p.Y * percent);

					//
					//	Now determine what grid cell this is
					//
					int cell_x	= curr_x + cell_x_offset;
					int cell_y	= Get_Quad_Index_Y (y_pos, false);
					if (Is_Valid_Quad (cell_x, cell_y)) {

						//
						//	Test this quad to see if either of its triangles intersect
						//
						if (Collide_Quad (line_seg, cell_x, cell_y, vertical_result)) {
							did_vertical_collide = true;
							break;
						}
					}
				}
			}
		}

		//
		//	Test horizontal grid lines next
		//
		if (delta_p.Y != 0.0F) {
			
			//
			//	Find the first intersection point moving along horizontal grid lines
			//
			for (int curr_y = start_cell_y; cells_y >= 0; cells_y --, curr_y += y_inc) {
				
				//
				//	Determine where the ray intersects this grid line
				//
				float y_pos		= Get_Grid_Line_Pos_Y (curr_y);
				float percent	= (y_pos - p0.Y) / delta_p.Y;

				//
				//	Don't test the cell if its outside the range of the line segment
				//
				if (percent >= 0 && percent <= 1.0F) {
					float x_pos	= p0.X + (delta_p.X * percent);

					//
					//	Now determine what grid cell this is
					//
					int cell_y	= curr_y + cell_y_offset;
					int cell_x	= Get_Quad_Index_X (x_pos, false);
					if (Is_Valid_Quad (cell_x, cell_y)) {

						//
						//	Test this quad to see if either of its triangles intersect
						//
						if (Collide_Quad (line_seg, cell_x, cell_y, horizontal_result)) {
							did_horizontal_collide = true;
							break;
						}
					}
				}
			}
		}

		//
		//	Make sure to test the ending cell
		//
		if (did_vertical_collide == false && did_horizontal_collide == false) {
			if (Collide_Quad (line_seg, cell_coord_p1.I, cell_coord_p1.J, vertical_result)) {
				did_vertical_collide = true;
			}
		}
	}

	//
	//	Did either collide?
	//		
	if (did_vertical_collide || did_horizontal_collide) {
		retval = true;

		//
		//	Determine which result was hit first
		//
		if (did_vertical_collide && did_horizontal_collide) {
			
			//
			//	Both triangles intersected, so find the one that hit first.
			//
			if (vertical_result.Fraction < horizontal_result.Fraction) {
				*(raytest.Result) = vertical_result;
			} else {
				*(raytest.Result) = horizontal_result;
			}

		} else if (did_vertical_collide) {
			*(raytest.Result) = vertical_result;
		} else {
			*(raytest.Result) = horizontal_result;
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
//////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
RenegadeTerrainPatchClass::Get_Factory (void) const
{
	return _TerrainPatchFactory;
}


//////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainPatchClass::Save (ChunkSaveClass &csave)
{
	//
	//	Write the variables
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_PTS_X,		GridPointsX);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_PTS_Y,		GridPointsY);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_PT_COUNT,	GridPointCount);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_BBOX_MIN,	BoundingBoxMin);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_BBOX_MAX,	BoundingBoxMax);
		WRITE_MICRO_CHUNK (csave, VARID_GRID_DENSITY,	Density);
		WRITE_MICRO_CHUNK (csave, VARID_IS_PRELIT,		IsPreLit);
	csave.End_Chunk ();

	//
	//	Now, write out the "array" of heights
	//
	csave.Begin_Chunk (CHUNKID_HEIGHTS);

		int index;
		for (index = 0; index < GridPointCount; index ++) {
			csave.Write (&Grid[index].X, sizeof (float) * 3);
		}
				
	csave.End_Chunk ();

	//
	//	Now, write out the "array" of normals
	//
	csave.Begin_Chunk (CHUNKID_NORMALS);

		for (index = 0; index < GridPointCount; index ++) {
			csave.Write (&GridNormals[index].X, sizeof (float) * 3);
		}
				
	csave.End_Chunk ();

	//
	//	Now, write out the "array" of vertex colors
	//
	csave.Begin_Chunk (CHUNKID_VERTEX_COLORS);

		for (index = 0; index < GridPointCount; index ++) {
			csave.Write (&VertexColors[index].X, sizeof (float) * 3);
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
	//	Now, save each material layer
	//
	csave.Begin_Chunk (CHUNKID_MATERIAL_LAYERS);

		for (index = 0; index < MaterialPassList.Count (); index ++) {
			
			//
			//	Don't save the material information if there' no material configured...
			//
			if (MaterialPassList[index]->Material != NULL) {

				//
				//	Save this material layer to its own chunk
				//
				csave.Begin_Chunk (CHUNKID_MATERIAL_LAYER);
					MaterialPassList[index]->Save (csave);
				csave.End_Chunk ();
			}
		}
				
	csave.End_Chunk ();

	return true;
}


//////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////
bool
RenegadeTerrainPatchClass::Load (ChunkLoadClass &cload)
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
					cload.Read (&Grid[index].X, sizeof (float) * 3);
				}

				break;
			}
			
			case CHUNKID_NORMALS:
			{
				//
				//	Read the array of normals
				//
				for (int index = 0; index < GridPointCount; index ++) {
					cload.Read (&GridNormals[index].X, sizeof (float) * 3);
				}

				break;
			}

			case CHUNKID_VERTEX_COLORS:
			{
				//
				//	Read the array of vertex colors
				//
				for (int index = 0; index < GridPointCount; index ++) {
					cload.Read (&VertexColors[index].X, sizeof (float) * 3);
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

			case CHUNKID_MATERIAL_LAYERS:
				Load_Materials (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	Invalidate_Cached_Bounding_Volumes();

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			//
			//	Read each of the microchunks
			//
			READ_MICRO_CHUNK (cload, VARID_GRID_PTS_X,		GridPointsX);
			READ_MICRO_CHUNK (cload, VARID_GRID_PTS_Y,		GridPointsY);
			READ_MICRO_CHUNK (cload, VARID_GRID_PT_COUNT,	GridPointCount);
			READ_MICRO_CHUNK (cload, VARID_GRID_BBOX_MIN,	BoundingBoxMin);
			READ_MICRO_CHUNK (cload, VARID_GRID_BBOX_MAX,	BoundingBoxMax);
			READ_MICRO_CHUNK (cload, VARID_GRID_DENSITY,		Density);			
			READ_MICRO_CHUNK (cload, VARID_IS_PRELIT,			IsPreLit);
		}

		cload.Close_Micro_Chunk ();
	}

	Set_Has_User_Lighting(IsPreLit);

	//
	//	Allocate and initialize the grid and supporting data structures
	//
	Allocate_Grid ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load_Materials
//
////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Load_Materials (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_MATERIAL_LAYER:
			{ 
				//
				//	Create and load the material
				//
				RenegadeTerrainMaterialPassClass *material = new RenegadeTerrainMaterialPassClass;
				material->Load (cload);

				//
				//	Add the material to our list
				//
				MaterialPassList.Add (material);
				break;
			}
		}

		cload.Close_Chunk ();
	}

	return ;
}



////////////////////////////////////////////////////////////////
//
//	Add_Material
//
////////////////////////////////////////////////////////////////
int
RenegadeTerrainPatchClass::Add_Material (TerrainMaterialClass *material)
{
	//
	//	Add a reference to the material
	//
	if (material != NULL) {
		material->Add_Ref ();
	}	

	//
	//	Allocate a new pass for this material
	//
	RenegadeTerrainMaterialPassClass *material_pass = new RenegadeTerrainMaterialPassClass;
	material_pass->Allocate (GridPointCount);
	material_pass->Material = material;

	//
	//	Add this material
	//
	int retval = MaterialPassList.Count ();
	MaterialPassList.Add (material_pass);

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Material_Passes
//
////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Reset_Material_Passes (void)
{
	//
	//	Reset each of the materials
	//
	for (int index = 0; index < MaterialPassList.Count (); index ++) {
		MaterialPassList[index]->Reset ();
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_UVs
//
////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Update_UVs (void)
{
	for (int y_pos = 0; y_pos < GridPointsY; y_pos ++) {
		int start_index = (y_pos * GridPointsX);
		for (int x_pos = 0; x_pos < GridPointsX; x_pos ++) {
			
			//
			//	Calculate the UV coordinate for this texture at this vertex
			//			
			for (int index = 0; index < MaterialPassList.Count (); index ++) {
				if (MaterialPassList[index]->Material != NULL) {
					float meters_per_texture = MaterialPassList[index]->Material->Get_Meters_Per_Tile ();

					
					float u_value = Grid[start_index + x_pos].X / meters_per_texture;
					float v_value = Grid[start_index + x_pos].Y / meters_per_texture;

					//
					//	Handle mirrored UVs
					//
					if (MaterialPassList[index]->Material->Are_UVs_Mirrored ()) {
						int u_int_value = WWMath::Float_To_Int_Floor (u_value);
						int v_int_value = WWMath::Float_To_Int_Floor (v_value);

						u_value = u_value - u_int_value;
						v_value = v_value - v_int_value;

						if (u_int_value & 1) {
							u_value = 1.0F - u_value;
						}

						if (v_int_value & 1) {
							v_value = 1.0F - v_value;
						}
					}
					
					MaterialPassList[index]->GridUVs[start_index + x_pos].Set (u_value, v_value);
				}
			}
		}
	}
	
	AreBuffersDirty = true;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Vertex_Render_Lists
//
////////////////////////////////////////////////////////////////
void
RenegadeTerrainPatchClass::Update_Vertex_Render_Lists (void)
{
	int col_count = GridPointsX - 1;

	//
	//	Loop over each material layer
	//
	for (int index = 0; index < MaterialPassList.Count (); index ++) {
		
		//
		//	Loop over each pass for this layer
		//
		RenegadeTerrainMaterialPassClass *material_pass = MaterialPassList[index];
		for (int pass = 0; pass < RenegadeTerrainMaterialPassClass::PASS_COUNT; pass ++) {

			//
			//	Loop over each quad that is rendered in this material pass
			//
			for (int quad_index = 0; quad_index < material_pass->QuadList[pass].Count (); quad_index ++) {
				
				//
				//	Determine the coordinate for this quad
				//
				int real_quad_index = material_pass->QuadList[pass][quad_index];
				int quad_y = (real_quad_index / col_count);
				int quad_x = real_quad_index - (quad_y * col_count);

				//
				//	Get the vertex indices that define the four corners of this quad
				//
				int curr_src_index = Grid_Index (quad_x, quad_y);
				int v0_index = curr_src_index;
				int v1_index = curr_src_index + 1;
				int v2_index = curr_src_index + GridPointsX + 1;
				int v3_index = curr_src_index + GridPointsX;

				//
				//	Add the verts from this quad to the render list
				//
				if (material_pass->VertexIndexMap[pass][v0_index] == -1) {
					material_pass->VertexIndexMap[pass][v0_index] = material_pass->VertexRenderList[pass].Count ();
					material_pass->VertexRenderList[pass].Add (v0_index);
				}

				if (material_pass->VertexIndexMap[pass][v1_index] == -1) {
					material_pass->VertexIndexMap[pass][v1_index] = material_pass->VertexRenderList[pass].Count ();
					material_pass->VertexRenderList[pass].Add (v1_index);
				}

				if (material_pass->VertexIndexMap[pass][v2_index] == -1) {
					material_pass->VertexIndexMap[pass][v2_index] = material_pass->VertexRenderList[pass].Count ();
					material_pass->VertexRenderList[pass].Add (v2_index);
				}

				if (material_pass->VertexIndexMap[pass][v3_index] == -1) {
					material_pass->VertexIndexMap[pass][v3_index] = material_pass->VertexRenderList[pass].Count ();
					material_pass->VertexRenderList[pass].Add (v3_index);
				}
			}
		}
	}

	AreBuffersDirty = true;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Peek_Material_Pass
//
////////////////////////////////////////////////////////////////
RenegadeTerrainMaterialPassClass *
RenegadeTerrainPatchClass::Get_Material_Pass (int index, TerrainMaterialClass *material)
{
	//
	//	Grow the list as necessary
	//
	while (index >= MaterialPassList.Count ()) {
		Add_Material (NULL);
	}

	//
	//	Ensure the material is correct at this layer
	//
	if (MaterialPassList[index]->Material != material) {
		REF_PTR_SET (MaterialPassList[index]->Material, material);
	}

	return MaterialPassList[index];
}
