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

/* $Header: /Commando/Code/ww3d2/mesh.h 17    3/14/02 2:37p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D engine                                       * 
 *                                                                                             * 
 *                    File Name : MESH.H                                                       * 
 *                                                                                             * 
 *                   Programmer : Greg Hjelstrom                                               * 
 *                                                                                             * 
 *                   Start Date : 06/11/97                                                     * 
 *                                                                                             * 
 *                  Last Update : June 11, 1997 [GH]                                           * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef MESH_H
#define MESH_H

#include "always.h"
#include "rendobj.h"
#include "bittype.h"
#include "w3derr.h"
#include "dx8polygonrenderer.h"

class MeshBuilderClass;
class HModelClass;
class AuxMeshDataClass;
class MeshLoadInfoClass;
class W3DMeshClass;
class MaterialInfoClass;
class ChunkLoadClass;
class ChunkSaveClass;
class RenderInfoClass;
class MeshModelClass;
class DecalMeshClass;
class LightEnvironmentClass;
class MaterialPassClass;
class IndexBufferClass;
struct W3dMeshHeaderStruct;
struct W3dTexCoordStruct;
class TextureClass;
class VertexMaterialClass;
struct VertexFormatXYZNDUV2;
class DX8FVFCategoryContainer;

/**
** MeshClass -- Render3DObject for rendering meshes.
*/
class MeshClass : public RenderObjClass
{
public:

	MeshClass(void);
	MeshClass(const MeshClass & src);
private:	
	MeshClass & operator = (const MeshClass &);
public:
	virtual ~MeshClass(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface 
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const override;
	virtual int						Class_ID(void) const override { return CLASSID_MESH; }
	virtual const char *			Get_Name(void) const override;
	virtual void					Set_Name(const char * name) override;
	virtual int						Get_Num_Polys(void) const override;
	virtual void					Render(RenderInfoClass & rinfo) override;
	void								Render_Material_Pass(MaterialPassClass * pass,IndexBufferClass * ib);
	virtual void					Special_Render(SpecialRenderInfoClass & rinfo) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Collision Detection
	/////////////////////////////////////////////////////////////////////////////	
	virtual bool					Cast_Ray(RayCollisionTestClass & raytest) override;
	virtual bool					Cast_AABox(AABoxCollisionTestClass & boxtest) override;
	virtual bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest) override;
	virtual bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest) override;
	virtual bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest) override;
   
	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Bounding Volumes
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const override;
   virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Attributes, Options, Properties, etc
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Scale(float scale) override;
	virtual void					Scale(float scalex, float scaley, float scalez) override;
	virtual MaterialInfoClass * Get_Material_Info(void) override;
	
   virtual int						Get_Sort_Level(void) const override;
   virtual void					Set_Sort_Level(int level) override;


	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Decals
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Create_Decal(DecalGeneratorClass * generator) override;
	virtual void					Delete_Decal(uint32 decal_id) override;
	
	/////////////////////////////////////////////////////////////////////////////
	// MeshClass Interface
	/////////////////////////////////////////////////////////////////////////////
	WW3DErrorType					Init(const MeshBuilderClass & builder,MaterialInfoClass * matinfo,const char * name,const char * hmodelname);
	WW3DErrorType					Load_W3D(ChunkLoadClass & cload);
	void								Generate_Culling_Tree(void);
	MeshModelClass *				Get_Model(void);
	MeshModelClass *				Peek_Model(void);
	uint32							Get_W3D_Flags(void);
	const char *					Get_User_Text(void) const;

	bool								Contains(const Vector3 &point);

	void								Compose_Deformed_Vertex_Buffer(
											VertexFormatXYZNDUV2* verts,
											const Vector2* uv0,
											const Vector2* uv1,
											const unsigned* diffuse);
	void								Get_Deformed_Vertices(Vector3 *dst_vert, Vector3 *dst_norm);
	void								Get_Deformed_Vertices(Vector3 *dst_vert);

	void								Set_Lighting_Environment(LightEnvironmentClass * light_env) { LightEnvironment = light_env; }
	LightEnvironmentClass *		Get_Lighting_Environment(void) { return LightEnvironment; }

	void								Set_Next_Visible_Skin(MeshClass * next_visible) { NextVisibleSkin = next_visible; }
	MeshClass *						Peek_Next_Visible_Skin(void) { return NextVisibleSkin; }

	void								Set_Base_Vertex_Offset(int base) { BaseVertexOffset = base; }
	int								Get_Base_Vertex_Offset(void) { return BaseVertexOffset; }

	// Do old .w3d mesh files get fog turned on or off?
	static bool						Legacy_Meshes_Fogged;

	void								Replace_Texture(TextureClass* texture,TextureClass* new_texture);
	void								Replace_VertexMaterial(VertexMaterialClass* vmat,VertexMaterialClass* new_vmat);

	void								Make_Unique();
	unsigned							Get_Debug_Id() const { return  MeshDebugId; }

	void								Set_Debugger_Disable(bool b) { IsDisabledByDebugger=b; }
	bool								Is_Disabled_By_Debugger() const { return IsDisabledByDebugger; }

	/*
	** User Lighting feature, meshes can have a user lighting array.
	*/
	void								Install_User_Lighting_Array(Vector4 * lighting);
	unsigned int *					Get_User_Lighting_Array(bool alloc = false);

	virtual void					Save_User_Lighting (ChunkSaveClass & csave) override;
	virtual void					Load_User_Lighting (ChunkLoadClass & cload) override;

protected:

	void								Free(void);

	virtual void					Add_Dependencies_To_List (DynamicVectorClass<StringClass> &file_list, bool textures_only = false) override;
	virtual void					Update_Cached_Bounding_Volumes(void) const override;
	DX8FVFCategoryContainer*	Peek_FVF_Category_Container(void);

	void								install_materials(MeshLoadInfoClass * loadinfo);
	void								clone_materials(const MeshClass & srcmesh);
	void								setup_materials_for_user_lighting(void);
	void								setup_material_for_user_lighting(VertexMaterialClass * mtl);

	MeshModelClass *				Model;
	DecalMeshClass *				DecalMesh;

	LightEnvironmentClass *		LightEnvironment;		// cached pointer to the light environment for this mesh
	int								BaseVertexOffset;		// offset to our first vertex in whatever vb this mesh is in.
	MeshClass *						NextVisibleSkin;		// linked list of visible skins

	unsigned							MeshDebugId;
	bool								IsDisabledByDebugger;

	unsigned int *					UserLighting;			// optional array of user lighting values

	// DX8 Mesh rendering system data
	DX8PolygonRendererList		PolygonRendererList;

	friend class MeshBuilderClass;
	friend class DX8MeshRendererClass;
	friend class DX8PolygonRendererClass;
};

inline MeshModelClass * MeshClass::Peek_Model(void)
{
	return Model;
}


// This utility function recurses throughout the subobjects of a renderobject,
// and for each MeshClass it finds it sets the given MeshModel flag on its
// model. This is useful for stuff like making a RenderObjects' polys sort.
//void Set_MeshModel_Flag(RenderObjClass *robj, MeshModelClass::FlagsType flag, int onoff);
void Set_MeshModel_Flag(RenderObjClass *robj, int flag, int onoff);

#endif /*MESH_H*/

