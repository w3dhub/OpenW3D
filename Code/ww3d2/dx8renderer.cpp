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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/dx8renderer.cpp                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/28/02 5:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 111                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//#define ENABLE_CATEGORY_LOG
//#define ENABLE_STRIPING

#include <bit>
#include "dx8renderer.h"
#include "dx8wrapper.h"
#include "dx8polygonrenderer.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8fvf.h"
#include "dx8caps.h"
#include "dx8rendererdebugger.h"
#include "wwdebug.h"
#include "wwprofile.h"
#include "wwmemlog.h"
#include "rinfo.h"
#include "statistics.h"
#include "meshmdl.h"
#include "vp.h"
#include "decalmsh.h"
#include "matpass.h"
#include "camera.h"
#include "stripoptimizer.h"
#include "meshgeometry.h"
#include "hashtemplate.h"


/*
** Global Instance of the DX8MeshRender
*/
DX8MeshRendererClass TheDX8MeshRenderer;

// ----------------------------------------------------------------------------

static DynamicVectorClass<Vector3>				_TempVertexBuffer;
static DynamicVectorClass<Vector3>				_TempNormalBuffer;

static TextureCategoryList							texture_category_delete_list;
static FVFCategoryList								fvf_category_container_delete_list;


// helper data structure
class PolyRemover : public MultiListObjectClass
{
public:
	DX8TextureCategoryClass *	src;
	DX8TextureCategoryClass *	dest;
	DX8PolygonRendererClass *  pr;
};

typedef MultiListClass<PolyRemover>			PolyRemoverList;
typedef MultiListIterator<PolyRemover>		PolyRemoverListIterator;


/**
** PolyRenderTaskClass
** This is a record of a polyrendere that needs to be rendered
** for this frame.  Since MeshClass instances can share meshmodels
** (and therefore their dx8 polygon renderers) this record contains
** a pointer to the polygon renderer and the MeshClass instance that
** it is being rendered for. 
*/
class PolyRenderTaskClass : public AutoPoolClass<PolyRenderTaskClass, 256>
{
public:
	PolyRenderTaskClass(DX8PolygonRendererClass * p_renderer,MeshClass * p_mesh) :
		Renderer(p_renderer),
		Mesh(p_mesh),
		NextVisible(NULL)
	{
		WWASSERT(Renderer != NULL);
		WWASSERT(Mesh != NULL);
		Mesh->Add_Ref();
	}

	~PolyRenderTaskClass(void)
	{
		Mesh->Release_Ref();
	}

	DX8PolygonRendererClass *	Peek_Polygon_Renderer(void)							{ return Renderer; }
	MeshClass *						Peek_Mesh(void)											{ return Mesh; }

	PolyRenderTaskClass *		Get_Next_Visible(void)									{ return NextVisible; }
	void								Set_Next_Visible(PolyRenderTaskClass * prtc)		{ NextVisible = prtc; }

protected:

	DX8PolygonRendererClass *	Renderer;
	MeshClass *						Mesh;
	PolyRenderTaskClass *		NextVisible;

};

DEFINE_AUTO_POOL(PolyRenderTaskClass, 256);

/**
** MatPassTaskClass
** This is the record of a material pass that needs to be rendered on
** a particular mesh.  These are linked into the FVF container which
** contains the mesh model.  They are also pooled to remove memory 
** allocation overhead.
*/
class MatPassTaskClass : public AutoPoolClass<MatPassTaskClass, 256>
{
public:
	MatPassTaskClass(MaterialPassClass * pass,MeshClass * mesh) :
		MaterialPass(pass),
		Mesh(mesh),
		NextVisible(NULL)
	{
		WWASSERT(MaterialPass != NULL);
		WWASSERT(Mesh != NULL);
		MaterialPass->Add_Ref();
		Mesh->Add_Ref();
	}

	~MatPassTaskClass(void)
	{
		MaterialPass->Release_Ref();
		Mesh->Release_Ref();
	}
	
	MaterialPassClass *	Peek_Material_Pass(void)							{ return MaterialPass; }
	MeshClass *				Peek_Mesh(void)										{ return Mesh; }
	
	MatPassTaskClass *	Get_Next_Visible(void)								{ return NextVisible; }
	void						Set_Next_Visible(MatPassTaskClass * mpr)		{ NextVisible = mpr; }

private:

	MaterialPassClass *	MaterialPass;
	MeshClass *				Mesh;
	MatPassTaskClass *	NextVisible;
};

DEFINE_AUTO_POOL(MatPassTaskClass, 256);


// ----------------------------------------------------------------------------


inline static bool Equal_Material(const VertexMaterialClass* mat1,const VertexMaterialClass* mat2)
{
	int crc0 = mat1 ? mat1->Get_CRC() : 0;
	int crc1 = mat2 ? mat2->Get_CRC() : 0;
	return (crc0 == crc1);
}


DX8TextureCategoryClass::DX8TextureCategoryClass(
	DX8FVFCategoryContainer* container_,
	TextureClass** texs,
	ShaderClass shd, 
	VertexMaterialClass* mat,
	int pass_)
	:
	pass(pass_),
	shader(shd),
	render_task_head(NULL),
	material(mat),
	container(container_)
{
	WWASSERT(pass>=0);
	WWASSERT(pass<DX8FVFCategoryContainer::MAX_PASSES);

	for (int a=0;a<MAX_TEXTURE_STAGES;++a) {
		textures[a]=NULL;
		REF_PTR_SET(textures[a],texs[a]);
	}

	if (material) material->Add_Ref();
}

DX8TextureCategoryClass::~DX8TextureCategoryClass()
{
	// Unregistering the mesh where polygon renderers are connected to kills all polygon renderers
	while (DX8PolygonRendererClass* p_renderer=PolygonRendererList.Get_Head()) {
		TheDX8MeshRenderer.Unregister_Mesh_Type(p_renderer->Get_Mesh_Class());
	}
	for (int a=0;a<MAX_TEXTURE_STAGES;++a) {
		REF_PTR_RELEASE(textures[a]);
	}

	REF_PTR_RELEASE(material);
}

void DX8TextureCategoryClass::Add_Render_Task(DX8PolygonRendererClass * p_renderer,MeshClass * p_mesh)
{
	PolyRenderTaskClass * new_prt = new PolyRenderTaskClass(p_renderer,p_mesh);
	new_prt->Set_Next_Visible(render_task_head);
	render_task_head = new_prt;

	container->Add_Visible_Texture_Category(this,pass);
}

void DX8TextureCategoryClass::Add_Polygon_Renderer(DX8PolygonRendererClass* p_renderer,DX8PolygonRendererClass* add_after_this)
{
	WWASSERT(p_renderer!=NULL);
	WWASSERT(!PolygonRendererList.Contains(p_renderer));

	if (add_after_this != NULL) {
		[[maybe_unused]] bool res = PolygonRendererList.Add_After(p_renderer,add_after_this,false);
		WWASSERT(res != false);
	} else {
		PolygonRendererList.Add(p_renderer);
	}

	p_renderer->Set_Texture_Category(this);
}

void DX8TextureCategoryClass::Remove_Polygon_Renderer(DX8PolygonRendererClass* p_renderer)
{
	PolygonRendererList.Remove(p_renderer);
	p_renderer->Set_Texture_Category(NULL);
	if (PolygonRendererList.Peek_Head() == NULL) {
		container->Remove_Texture_Category(this);
		texture_category_delete_list.Add_Tail(this);
	}
}


void DX8FVFCategoryContainer::Remove_Texture_Category(DX8TextureCategoryClass* tex_category)
{
	unsigned pass;
	for (pass=0;pass<passes;++pass) {
		texture_category_list[pass].Remove(tex_category);
	}
	for (pass=0; pass<passes; pass++) {
		// If any of the texture category lists has anything in it, no need to delete this container
		if (texture_category_list[pass].Peek_Head() != NULL) return;
	}
	fvf_category_container_delete_list.Add_Tail(this);
}

void DX8FVFCategoryContainer::Add_Visible_Material_Pass(MaterialPassClass * pass,MeshClass * mesh)
{
	MatPassTaskClass * new_mpr = new MatPassTaskClass(pass,mesh);

	if (visible_matpass_head == NULL) {
		WWASSERT(visible_matpass_tail == NULL);
		visible_matpass_head = new_mpr;
	} else {
		WWASSERT(visible_matpass_tail != NULL);
		visible_matpass_tail->Set_Next_Visible(new_mpr);
	}

	visible_matpass_tail = new_mpr;
	AnythingToRender=true;
}

void DX8FVFCategoryContainer::Render_Procedural_Material_Passes(void)
{
	// additional passes
	MatPassTaskClass * mpr = visible_matpass_head;
	while (mpr != NULL) {
	
		mpr->Peek_Mesh()->Render_Material_Pass(mpr->Peek_Material_Pass(),index_buffer);
		MatPassTaskClass * next_mpr = mpr->Get_Next_Visible();
		
		delete mpr;
		mpr = next_mpr;
	}

	visible_matpass_head = visible_matpass_tail = NULL;
}

void DX8RigidFVFCategoryContainer::Add_Delayed_Visible_Material_Pass(MaterialPassClass * pass, MeshClass * mesh)
{
	MatPassTaskClass * new_mpr = new MatPassTaskClass(pass,mesh);

	if (delayed_matpass_head == NULL) {
		WWASSERT(delayed_matpass_tail == NULL);
		delayed_matpass_head = new_mpr;
	} else {
		WWASSERT(delayed_matpass_tail != NULL);
		delayed_matpass_tail->Set_Next_Visible(new_mpr);
	}

	delayed_matpass_tail = new_mpr;
	AnyDelayedPassesToRender=true;
}

void DX8RigidFVFCategoryContainer::Render_Delayed_Procedural_Material_Passes(void)
{
	if (!Any_Delayed_Passes_To_Render()) return;
	AnyDelayedPassesToRender=false;

	DX8Wrapper::Set_Vertex_Buffer(vertex_buffer);
	DX8Wrapper::Set_Index_Buffer(index_buffer,0);

	SNAPSHOT_SAY(("DX8RigidFVFCategoryContainer::Render_Delayed_Procedural_Material_Passes()\n"));

	// additional passes
	MatPassTaskClass * mpr = delayed_matpass_head;
	while (mpr != NULL) {
	
		mpr->Peek_Mesh()->Render_Material_Pass(mpr->Peek_Material_Pass(),index_buffer);
		MatPassTaskClass * next_mpr = mpr->Get_Next_Visible();
		
		delete mpr;
		mpr = next_mpr;
	}

	delayed_matpass_head = delayed_matpass_tail = NULL;
}


void DX8TextureCategoryClass::Log([[maybe_unused]] bool only_visible)
{
#ifdef ENABLE_CATEGORY_LOG
	StringClass work(255,true);
	work.Format("	DX8TextureCategoryClass\n");
	WWDEBUG_SAY((work));

	StringClass work2(255,true);
	for (int stage=0;stage<MAX_TEXTURE_STAGES;++stage) {
		work2.Format("	texture[%d]: %x (%s)\n", stage, textures[stage], textures[stage] ? textures[stage]->Get_Name() : "-");
		work+=work2;
	}
	work2.Format("	material: %x (%s)\n	shader: %x\n", material, material ? material->Get_Name() : "-", shader);
	work+=work2;
	WWDEBUG_SAY((work));

	work.Format("	%8s %8s %6s %6s %6s %5s %s\n",
		"idx_cnt",
		"poly_cnt",
		"i_offs",
		"min_vi",
		"vi_rng",
		"ident",
		"name");
	WWDEBUG_SAY((work));

	DX8PolygonRendererListIterator it(&PolygonRendererList);
	while (!it.Is_Done()) {
	
		DX8PolygonRendererClass* p_renderer = it.Peek_Obj();

		PolyRenderTaskClass * prtc=render_task_head;
		while (prtc) {
			if (prtc->Peek_Polygon_Renderer()==p_renderer) break;
			prtc = prtc->Get_Next_Visible();
		}

		if (prtc != NULL) {
			WWDEBUG_SAY(("+"));
			p_renderer->Log();
		} else {
			if (!only_visible) {
				WWDEBUG_SAY(("-"));
				p_renderer->Log();
			}
		}
		it.Next();
	}
#endif
}

// ----------------------------------------------------------------------------

DX8FVFCategoryContainer::DX8FVFCategoryContainer(unsigned FVF_,bool sorting_)
	:
	FVF(FVF_),
	sorting(sorting_),
	visible_matpass_head(NULL),
	visible_matpass_tail(NULL),
	index_buffer(0),
	used_indices(0),
	passes(MAX_PASSES),
	uv_coordinate_channels(0),
	AnythingToRender(false),
	AnyDelayedPassesToRender(false)
{
	if ((FVF&D3DFVF_TEX1)==D3DFVF_TEX1) uv_coordinate_channels=1;
	if ((FVF&D3DFVF_TEX2)==D3DFVF_TEX2) uv_coordinate_channels=2;
	if ((FVF&D3DFVF_TEX3)==D3DFVF_TEX3) uv_coordinate_channels=3;
	if ((FVF&D3DFVF_TEX4)==D3DFVF_TEX4) uv_coordinate_channels=4;
	if ((FVF&D3DFVF_TEX5)==D3DFVF_TEX5) uv_coordinate_channels=5;
	if ((FVF&D3DFVF_TEX6)==D3DFVF_TEX6) uv_coordinate_channels=6;
	if ((FVF&D3DFVF_TEX7)==D3DFVF_TEX7) uv_coordinate_channels=7;
	if ((FVF&D3DFVF_TEX8)==D3DFVF_TEX8) uv_coordinate_channels=8;
}

// ----------------------------------------------------------------------------

DX8FVFCategoryContainer::~DX8FVFCategoryContainer()
{
	REF_PTR_RELEASE(index_buffer);

	for (unsigned p=0;p<passes;++p) {
		while (DX8TextureCategoryClass * tex = texture_category_list[p].Remove_Head()) {
			delete tex;
		}
	}
}

// ----------------------------------------------------------------------------

DX8TextureCategoryClass* DX8FVFCategoryContainer::Find_Matching_Texture_Category(
	TextureClass* texture,
	unsigned pass,
	unsigned stage,
	DX8TextureCategoryClass* ref_category)
{
	// Find texture category which matches ref_category's properties but has 'texture' on given pass and stage.
	DX8TextureCategoryClass* dest_tex_category=NULL;
	TextureCategoryListIterator dest_it(&texture_category_list[pass]);
	while (!dest_it.Is_Done()) {
		if (dest_it.Peek_Obj()->Peek_Texture(stage)==texture) {
			// Compare all stage's textures
			dest_tex_category=dest_it.Peek_Obj();
			bool all_textures_same = true;
			for (unsigned int s = 0; s < MeshMatDescClass::MAX_TEX_STAGES; s++) {
				if (stage!=s) {
					all_textures_same = all_textures_same && (dest_tex_category->Peek_Texture(s) == ref_category->Peek_Texture(s));
				}
			}
			if (all_textures_same &&
				Equal_Material(dest_tex_category->Peek_Material(),ref_category->Peek_Material()) &&
				dest_tex_category->Get_Shader()==ref_category->Get_Shader()) {
				return dest_tex_category;
			}
		}
		dest_it.Next();
	}
	return NULL;
}

DX8TextureCategoryClass* DX8FVFCategoryContainer::Find_Matching_Texture_Category(
		VertexMaterialClass* vmat,
		unsigned pass,		
		DX8TextureCategoryClass* ref_category)
{
	// Find texture category which matches ref_category's properties but has 'vmat' on given pass
	DX8TextureCategoryClass* dest_tex_category=NULL;
	TextureCategoryListIterator dest_it(&texture_category_list[pass]);
	while (!dest_it.Is_Done()) {
		if (Equal_Material(dest_it.Peek_Obj()->Peek_Material(),vmat)) {
			// Compare all stage's textures
			dest_tex_category=dest_it.Peek_Obj();
			bool all_textures_same = true;
			for (unsigned int s = 0; s < MeshMatDescClass::MAX_TEX_STAGES; s++)
				all_textures_same = all_textures_same && (dest_tex_category->Peek_Texture(s) == ref_category->Peek_Texture(s));			
			if (all_textures_same &&				
				dest_tex_category->Get_Shader()==ref_category->Get_Shader()) {
				return dest_tex_category;
			}
		}
		dest_it.Next();
	}
	return NULL;
}

void DX8FVFCategoryContainer::Change_Polygon_Renderer_Texture(
	DX8PolygonRendererList& polygon_renderer_list,
	TextureClass* texture,
	TextureClass* new_texture,
	unsigned pass,
	unsigned stage)
{
	WWASSERT(pass<passes);

	PolyRemoverList prl;

	bool foundtexture=false;

	if (texture==new_texture) return;

	// Find source texture category, then find all polygon renderers who belong to that category
	// and move them to destination category.
	TextureCategoryListIterator src_it(&texture_category_list[pass]);
	while (!src_it.Is_Done()) {
		DX8TextureCategoryClass* src_tex_category=src_it.Peek_Obj();		
		if (src_tex_category->Peek_Texture(stage)==texture) {
			foundtexture=true;
			DX8PolygonRendererListIterator poly_it(&polygon_renderer_list);
			while (!poly_it.Is_Done()) {
				// If source texture category contains polygon renderer, move to destination category
				DX8PolygonRendererClass* polygon_renderer=poly_it.Peek_Obj();
				DX8TextureCategoryClass *prc=polygon_renderer->Get_Texture_Category();

				if (prc==src_tex_category) {					
					DX8TextureCategoryClass* dest_tex_category=Find_Matching_Texture_Category(new_texture,pass,stage,src_tex_category);

					if (!dest_tex_category) {
						TextureClass * tmp_textures[MeshMatDescClass::MAX_TEX_STAGES];
						for (int s=0;s<MeshMatDescClass::MAX_TEX_STAGES;++s) {
							tmp_textures[s]=src_tex_category->Peek_Texture(s);
						}
						tmp_textures[stage]=new_texture;

						DX8TextureCategoryClass * new_tex_category=new DX8TextureCategoryClass(
							this,
							tmp_textures,
							src_tex_category->Get_Shader(),
							const_cast<VertexMaterialClass*>(src_tex_category->Peek_Material()),
							pass);
		
						/*
						** Add the texture category object into the list, immediately after any existing
						** texture category object which uses the same texture.  This will result in
						** the list always having matching texture categories next to each other.
						*/
						bool found_similar_category = false;
						TextureCategoryListIterator tex_it(&texture_category_list[pass]);
						while (!tex_it.Is_Done()) {
							// Categorize according to first stage's texture for now
							if (tex_it.Peek_Obj()->Peek_Texture(0) == tmp_textures[0]) {
								texture_category_list[pass].Add_After(new_tex_category,tex_it.Peek_Obj());
								found_similar_category = true;
								break;
							}
							tex_it.Next();
						}

						if (!found_similar_category) {
							texture_category_list[pass].Add_Tail(new_tex_category);
						}
						dest_tex_category=new_tex_category;
					}
					PolyRemover *rem=new PolyRemover;
					rem->src=src_tex_category;
					rem->dest=dest_tex_category;
					rem->pr=polygon_renderer;
					prl.Add(rem);					
				}
				poly_it.Next();
			} // while			
		} //if src_texture==texture
		else
			// quit loop if we've got a texture change
			if (foundtexture) break;
		src_it.Next();
	} // while

	PolyRemoverListIterator prli(&prl);

	while (!prli.Is_Done())
	{
		PolyRemover *rem=prli.Peek_Obj();
		rem->src->Remove_Polygon_Renderer(rem->pr);
		rem->dest->Add_Polygon_Renderer(rem->pr);		
		prli.Remove_Current_Object();
		delete rem;
	}
}

void DX8FVFCategoryContainer::Change_Polygon_Renderer_Material(
		DX8PolygonRendererList& polygon_renderer_list,
		VertexMaterialClass* vmat,
		VertexMaterialClass* new_vmat,
		unsigned pass)
{
	WWASSERT(pass<passes);

	PolyRemoverList prl;

	bool foundtexture=false;

	if (vmat==new_vmat) return;

	// Find source texture category, then find all polygon renderers who belong to that category
	// and move them to destination category.
	TextureCategoryListIterator src_it(&texture_category_list[pass]);
	while (!src_it.Is_Done()) {
		DX8TextureCategoryClass* src_tex_category=src_it.Peek_Obj();
		if (src_tex_category->Peek_Material()==vmat) {			
			DX8PolygonRendererListIterator poly_it(&polygon_renderer_list);
			while (!poly_it.Is_Done()) {
				// If source texture category contains polygon renderer, move to destination category
				DX8PolygonRendererClass* polygon_renderer=poly_it.Peek_Obj();
				DX8TextureCategoryClass *prc=polygon_renderer->Get_Texture_Category();
				if (prc==src_tex_category) {
					foundtexture=true;
					DX8TextureCategoryClass* dest_tex_category=Find_Matching_Texture_Category(new_vmat,pass,src_tex_category);

					if (!dest_tex_category) {
						TextureClass * tmp_textures[MeshMatDescClass::MAX_TEX_STAGES];
						for (int s=0;s<MeshMatDescClass::MAX_TEX_STAGES;++s) {
							tmp_textures[s]=src_tex_category->Peek_Texture(s);
						}						

						DX8TextureCategoryClass * new_tex_category=new DX8TextureCategoryClass(
							this,
							tmp_textures,
							src_tex_category->Get_Shader(),
							const_cast<VertexMaterialClass*>(new_vmat),
							pass);
		
						/*
						** Add the texture category object into the list, immediately after any existing
						** texture category object which uses the same texture.  This will result in
						** the list always having matching texture categories next to each other.
						*/
						bool found_similar_category = false;
						TextureCategoryListIterator tex_it(&texture_category_list[pass]);
						while (!tex_it.Is_Done()) {
							// Categorize according to first stage's texture for now
							if (tex_it.Peek_Obj()->Peek_Texture(0) == tmp_textures[0]) {
								texture_category_list[pass].Add_After(new_tex_category,tex_it.Peek_Obj());
								found_similar_category = true;
								break;
							}
							tex_it.Next();
						}

						if (!found_similar_category) {
							texture_category_list[pass].Add_Tail(new_tex_category);
						}
						dest_tex_category=new_tex_category;
					}
					PolyRemover *rem=new PolyRemover;
					rem->src=src_tex_category;
					rem->dest=dest_tex_category;
					rem->pr=polygon_renderer;
					prl.Add(rem);
				}
				poly_it.Next();
			} // while			
		} // if 
		else
			if (foundtexture) break;
		src_it.Next();
	} // while

	PolyRemoverListIterator prli(&prl);

	while (!prli.Is_Done())
	{
		PolyRemover *rem=prli.Peek_Obj();
		rem->src->Remove_Polygon_Renderer(rem->pr);
		rem->dest->Add_Polygon_Renderer(rem->pr);		
		prli.Remove_Current_Object();
		delete rem;
	}
}

// ----------------------------------------------------------------------------

unsigned DX8FVFCategoryContainer::Define_FVF(MeshModelClass* mmc,unsigned int * user_lighting,bool /*enable_lighting*/)
{
	if ((!!mmc->Get_Flag(MeshGeometryClass::SORT)) && WW3D::Is_Sorting_Enabled()) {
		return dynamic_fvf_type;
	}

	unsigned fvf=D3DFVF_XYZ;

	int tex_coord_count=mmc->Get_UV_Array_Count();

	if (mmc->Get_Color_Array(0,false) || (user_lighting != NULL)) {
		fvf|=D3DFVF_DIFFUSE;
	}
	if (mmc->Get_Color_Array(1,false)) {
		fvf|=D3DFVF_SPECULAR;
	}
	
	switch (tex_coord_count) {
	default:
	case 0:
		break;
	case 1: fvf|=D3DFVF_TEX1; break;
	case 2: fvf|=D3DFVF_TEX2; break;
	case 3: fvf|=D3DFVF_TEX3; break;
	case 4: fvf|=D3DFVF_TEX4; break;
	case 5: fvf|=D3DFVF_TEX5; break;
	case 6: fvf|=D3DFVF_TEX6; break;
	case 7: fvf|=D3DFVF_TEX7; break;
	case 8: fvf|=D3DFVF_TEX8; break;
	}

	if (!mmc->Needs_Vertex_Normals()) {  //enable_lighting || mmc->Get_Flag(MeshModelClass::PRELIT_MASK)) {
		return fvf;
	}

	fvf|=D3DFVF_NORMAL;	// Realtime-lit
	return fvf;
}

// ----------------------------------------------------------------------------

DX8RigidFVFCategoryContainer::DX8RigidFVFCategoryContainer(unsigned FVF,bool sorting_)
	:
	DX8FVFCategoryContainer(FVF,sorting_),
	vertex_buffer(0),
	used_vertices(0),
	delayed_matpass_head(NULL),
	delayed_matpass_tail(NULL)
{
}

// ----------------------------------------------------------------------------

DX8RigidFVFCategoryContainer::~DX8RigidFVFCategoryContainer()
{
	REF_PTR_RELEASE(vertex_buffer);
}

// ----------------------------------------------------------------------------

void DX8RigidFVFCategoryContainer::Log([[maybe_unused]] bool only_visible)
{
#ifdef ENABLE_CATEGORY_LOG
	StringClass work(255,true);
	work.Format("DX8RigidFVFCategoryContainer --------------\n");
	WWDEBUG_SAY((work));
	if (vertex_buffer) {
		StringClass fvfname(255,true);
		vertex_buffer->FVF_Info().Get_FVF_Name(fvfname);
		work.Format("VB size (used/total): %d/%d FVF: %s\n",used_vertices,vertex_buffer->Get_Vertex_Count(),fvfname);
		WWDEBUG_SAY((work));
	}
	else {
		WWDEBUG_SAY(("EMPTY VB\n"));
	}
	if (index_buffer) {
		work.Format("IB size (used/total): %d/%d\n",used_indices,index_buffer->Get_Index_Count());
		WWDEBUG_SAY((work));
	}
	else {
		WWDEBUG_SAY(("EMPTY IB\n"));
	}

	for (unsigned p=0;p<passes;++p) {
		WWDEBUG_SAY(("Pass: %d\n",p));

		TextureCategoryListIterator it(&texture_category_list[p]);
		while (!it.Is_Done()) {
			it.Peek_Obj()->Log(only_visible);
			it.Next();
		}
	}
#endif
}

// ----------------------------------------------------------------------------
//
// Generic render function for rigid meshes
//
// ----------------------------------------------------------------------------

void DX8RigidFVFCategoryContainer::Render(void)
{
	if (!Anything_To_Render()) return;
	AnythingToRender=false;

	DX8Wrapper::Set_Vertex_Buffer(vertex_buffer);

	DX8Wrapper::Set_Index_Buffer(index_buffer,0);

	SNAPSHOT_SAY(("DX8RigidFVFCategoryContainer::Render()\n"));
	int zbias=0;
	DX8Wrapper::Set_DX8_ZBias(zbias);
	for (unsigned p=0;p<passes;++p) {
		SNAPSHOT_SAY(("Pass: %d\n",p));
		while (DX8TextureCategoryClass * tex = visible_texture_category_list[p].Remove_Head()) {
			tex->Render();
		}
		zbias++;
		if (zbias>15) zbias=15;
		DX8Wrapper::Set_DX8_ZBias(zbias);
	}

	Render_Procedural_Material_Passes();

	DX8Wrapper::Set_DX8_ZBias(0);
}

// ----------------------------------------------------------------------------

bool DX8RigidFVFCategoryContainer::Check_If_Mesh_Fits(MeshModelClass* mmc)
{
	if (!vertex_buffer) return true;	// No VB created - mesh will fit as a new vb will be created when inserting
	unsigned required_vertices=mmc->Get_Vertex_Count();
	unsigned available_vertices=vertex_buffer->Get_Vertex_Count()-used_vertices;
	unsigned required_polygons=mmc->Get_Polygon_Count();
	if (mmc->Get_Gap_Filler()) {
		required_polygons+=mmc->Get_Gap_Filler()->Get_Polygon_Count();
	}
	unsigned required_indices=required_polygons*3*mmc->Get_Pass_Count();
	unsigned available_indices=index_buffer->Get_Index_Count()-used_indices;
	if (
		required_vertices<=available_vertices &&
		(required_indices)<=available_indices) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------

class Vertex_Split_Table
{
	MeshClass* mesh;
	MeshModelClass* mmc;
	bool npatch_enable;
	unsigned polygon_count;
	TriIndex* polygon_array;

	bool allocated_polygon_array;

public:
	Vertex_Split_Table(MeshClass* mesh_)
		:
		mesh(mesh_),
		mmc(NULL),
		npatch_enable(false),
		allocated_polygon_array(false)
	{
		mmc = mesh_->Peek_Model();
		if (DX8Wrapper::Get_Current_Caps()->Support_NPatches() && mmc->Needs_Vertex_Normals()) {
			if (mmc->Get_Flag(MeshGeometryClass::ALLOW_NPATCHES)) {
				npatch_enable=true;
			}
		}

		const GapFillerClass* gap_filler=mmc->Get_Gap_Filler();
		polygon_count=mmc->Get_Polygon_Count();
		if (gap_filler) polygon_count+=gap_filler->Get_Polygon_Count();
//		if (mmc->Get_Gap_Filler_Polygon_Count()) {
			allocated_polygon_array=true;
			polygon_array=new TriIndex[polygon_count];
			memcpy(
				polygon_array,
				mmc->Get_Polygon_Array(),
				mmc->Get_Polygon_Count()*sizeof(TriIndex));
			if (gap_filler) {
				memcpy(
					polygon_array+mmc->Get_Polygon_Count(),
					gap_filler->Get_Polygon_Array(),
					gap_filler->Get_Polygon_Count()*sizeof(TriIndex));
			}
//		}
//		else {
//			polygon_array=const_cast<Vector3i*>(mmc->Get_Polygon_Array());
//		}

	}

	~Vertex_Split_Table()
	{
		if (allocated_polygon_array) {
			delete[] polygon_array;
		}
	}

	const Vector3* Get_Vertex_Array() const
	{
		return mmc->Get_Vertex_Array();
	}

	const Vector3* Get_Vertex_Normal_Array() const
	{
		return mmc->Get_Vertex_Normal_Array();
	}

	const unsigned* Get_Color_Array(unsigned index) const
	{
		if ((index == 0) && (mesh->Get_User_Lighting_Array() != NULL)) {
			return mesh->Get_User_Lighting_Array();
		}
		return mmc->Get_Color_Array(index,false);
	}

	const Vector2* Get_UV_Array(unsigned uv_array_index) const
	{
		return mmc->Get_UV_Array_By_Index(uv_array_index);
	}

	unsigned Get_Vertex_Count() const
	{
		return mmc->Get_Vertex_Count();
	}

	unsigned Get_Polygon_Count() const
	{
		return polygon_count;
	}

	unsigned Get_Pass_Count() const
	{
		return mmc->Get_Pass_Count();
	}

	TextureClass* Peek_Texture(unsigned idx,unsigned pass,unsigned stage)
	{
		if (mmc->Has_Texture_Array(pass,stage)) {
			if (idx>=unsigned(mmc->Get_Polygon_Count())) {
				WWASSERT(mmc->Get_Gap_Filler());
				return mmc->Get_Gap_Filler()->Get_Texture_Array(pass,stage)[idx-mmc->Get_Polygon_Count()];
			}
			return mmc->Peek_Texture(idx,pass,stage);
		}
		return mmc->Peek_Single_Texture(pass,stage);
	}

	VertexMaterialClass* Peek_Material(unsigned idx,unsigned pass)
	{
		if (mmc->Has_Material_Array(pass)) {
			if (idx>=unsigned(mmc->Get_Polygon_Count())) {
				WWASSERT(mmc->Get_Gap_Filler());
				return mmc->Get_Gap_Filler()->Get_Material_Array(pass)[idx-mmc->Get_Polygon_Count()];
			}
			return mmc->Peek_Material(mmc->Get_Polygon_Array()[idx][0],pass);
		}
		return mmc->Peek_Single_Material(pass);
	}

	ShaderClass Peek_Shader(unsigned idx,unsigned pass)
	{
		if (mmc->Has_Shader_Array(pass)) {
			ShaderClass shader;
			
			if (idx>=unsigned(mmc->Get_Polygon_Count())) {
				WWASSERT(mmc->Get_Gap_Filler());
				shader=mmc->Get_Gap_Filler()->Get_Shader_Array(pass)[idx-mmc->Get_Polygon_Count()];
			}
			else shader=mmc->Get_Shader(idx,pass);

			if (npatch_enable) {
				shader.Set_NPatch_Enable(ShaderClass::NPATCH_ENABLE);
			}

			return shader;
		}
		if (!npatch_enable) return mmc->Get_Single_Shader(pass);
		ShaderClass shader=mmc->Get_Single_Shader(pass);
		shader.Set_NPatch_Enable(ShaderClass::NPATCH_ENABLE);
		return shader;

	}

	MeshModelClass* Get_Mesh_Model_Class()
	{
		return mmc;
	}

	MeshClass * Get_Mesh_Class()
	{
		return mesh;
	}

	unsigned short* Get_Polygon_Array(unsigned /*pass*/)
	{
		return (unsigned short*)polygon_array;
	}
};

// ----------------------------------------------------------------------------

void DX8RigidFVFCategoryContainer::Add_Mesh(MeshClass* mesh_)
{
	[[maybe_unused]] MeshModelClass * mmc_ = mesh_->Peek_Model();
	WWASSERT(Check_If_Mesh_Fits(mmc_));

	Vertex_Split_Table split_table(mesh_);
	int needed_vertices=split_table.Get_Vertex_Count();

	/*
	** This FVFCategoryContainer doesn't have a vertex buffer yet so allocate one big
	** enough to contain this mesh.
	*/
	if (!vertex_buffer) {
		int vb_size=4000;
		if (vb_size<needed_vertices) vb_size=needed_vertices;
		if (sorting) {
			vertex_buffer=NEW_REF(SortingVertexBufferClass,(vb_size));
			WWASSERT(vertex_buffer->FVF_Info().Get_FVF()==FVF);	// Only one sorting FVF type!
		}
		else {
			vertex_buffer=NEW_REF(DX8VertexBufferClass,(
				FVF,
				vb_size,
				(DX8Wrapper::Get_Current_Caps()->Support_NPatches() && WW3D::Get_NPatches_Level()>1) ? DX8VertexBufferClass::USAGE_NPATCHES : DX8VertexBufferClass::USAGE_DEFAULT));
		}
	}

	/*
	** Append this mesh's vertices to the vertex buffer.
	*/

	VertexBufferClass::AppendLockClass l(vertex_buffer,used_vertices,split_table.Get_Vertex_Count());
	const FVFInfoClass fi=vertex_buffer->FVF_Info();
	unsigned char *vb=(unsigned char*) l.Get_Vertex_Array();
	unsigned int i;
	const Vector3 *locs=split_table.Get_Vertex_Array();
	const Vector3 *norms=split_table.Get_Vertex_Normal_Array();
	const unsigned *diffuse=split_table.Get_Color_Array(0);
	const unsigned *specular=split_table.Get_Color_Array(1);
	for (i=0; i<split_table.Get_Vertex_Count(); i++)
	{
		*(Vector3*)(vb+fi.Get_Location_Offset())=locs[i];
		
		if ((FVF&D3DFVF_NORMAL)==D3DFVF_NORMAL && norms) {
			*(Vector3*)(vb+fi.Get_Normal_Offset())=norms[i];
		}

		if ((FVF&D3DFVF_DIFFUSE)==D3DFVF_DIFFUSE) {
			if (diffuse) {
				*(unsigned int*)(vb+fi.Get_Diffuse_Offset())=diffuse[i];
			} else {
				*(unsigned int*)(vb+fi.Get_Diffuse_Offset()) = 0xFFFFFFFF;
			}
		}
		
		if ((FVF&D3DFVF_SPECULAR)==D3DFVF_SPECULAR) {
			if (specular) {
				*(unsigned int*)(vb+fi.Get_Specular_Offset())=specular[i];
			} else {
				*(unsigned int*)(vb+fi.Get_Specular_Offset()) = 0xFFFFFFFF;
			}
		}

		vb+=fi.Get_FVF_Size();
	}
	

	/*
	** Append the UV coordinates to the vertex buffer
	*/
	int uvcount = 0;
	if ((FVF&D3DFVF_TEX1) == D3DFVF_TEX1) {
		uvcount = 1;
	}
	if ((FVF&D3DFVF_TEX2) == D3DFVF_TEX2) {
		uvcount = 2;
	}
	if ((FVF&D3DFVF_TEX3) == D3DFVF_TEX3) {
		uvcount = 3;
	}
	if ((FVF&D3DFVF_TEX4) == D3DFVF_TEX4) {
		uvcount = 4;
	}
	if ((FVF&D3DFVF_TEX5) == D3DFVF_TEX5) {
		uvcount = 5;
	}
	if ((FVF&D3DFVF_TEX6) == D3DFVF_TEX6) {
		uvcount = 6;
	}
	if ((FVF&D3DFVF_TEX7) == D3DFVF_TEX7) {
		uvcount = 7;
	}
	if ((FVF&D3DFVF_TEX8) == D3DFVF_TEX8) {
		uvcount = 8;
	}
	
	for (int j=0; j<uvcount; j++) {
		unsigned char *vb=(unsigned char*) l.Get_Vertex_Array();
		const Vector2*uvs=split_table.Get_UV_Array(j);
		if (uvs) {
			for (i=0; i<split_table.Get_Vertex_Count(); i++)
			{
				*(Vector2*)(vb+fi.Get_Tex_Offset(j))=uvs[i];
				vb+=fi.Get_FVF_Size();
			}		
		}
	}

	Generate_Texture_Categories(split_table,used_vertices);

	used_vertices+=needed_vertices;//vertex_count;
}

void DX8FVFCategoryContainer::Insert_To_Texture_Category(
	Vertex_Split_Table& split_table,
	TextureClass** texs,
	VertexMaterialClass* mat,
	ShaderClass shader,
	int pass,
	unsigned vertex_offset)
{
	/*
	** Try to find a DX8TextureCategoryClass in this FVF container which matches the
	** given textures(one per stage), material and shader combination.
	*/
	bool fit_in_existing_category = false;
	TextureCategoryListIterator it(&texture_category_list[pass]);
	while (!it.Is_Done()) {
		DX8TextureCategoryClass * tex_category=it.Peek_Obj();
		// Compare all stage's textures
		bool all_textures_same = true;
		for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
			all_textures_same = all_textures_same && (tex_category->Peek_Texture(stage) == texs[stage]);
		}
		if (all_textures_same && Equal_Material(tex_category->Peek_Material(),mat) && tex_category->Get_Shader()==shader) {
			used_indices+=tex_category->Add_Mesh(split_table,vertex_offset,used_indices,index_buffer,pass);
			fit_in_existing_category = true;
			break;
		}
		it.Next();
	}

	if (!fit_in_existing_category) {
		
		DX8TextureCategoryClass * new_tex_category=new DX8TextureCategoryClass(this,texs,shader,mat,pass);
		used_indices+=new_tex_category->Add_Mesh(split_table,vertex_offset,used_indices,index_buffer,pass);
		
		/*
		** Add the texture category object into the list, immediately after any existing
		** texture category object which uses the same texture.  This will result in
		** the list always having matching texture categories next to each other.
		*/
		bool found_similar_category = false;
		TextureCategoryListIterator it(&texture_category_list[pass]);
		while (!it.Is_Done()) {
			// Categorize according to first stage's texture for now
			if (it.Peek_Obj()->Peek_Texture(0) == texs[0]) {
				texture_category_list[pass].Add_After(new_tex_category,it.Peek_Obj());
				found_similar_category = true;
				break;
			}
			it.Next();
		}

		if (!found_similar_category) {
			texture_category_list[pass].Add_Tail(new_tex_category);
		}
	}
}

const unsigned MAX_ADDED_TYPE_COUNT=64;
struct Textures_Material_And_Shader_Booking_Struct
{
	TextureClass* added_textures[MeshMatDescClass::MAX_TEX_STAGES][MAX_ADDED_TYPE_COUNT];
	VertexMaterialClass* added_materials[MAX_ADDED_TYPE_COUNT];
	ShaderClass added_shaders[MAX_ADDED_TYPE_COUNT];
	unsigned added_type_count;

	Textures_Material_And_Shader_Booking_Struct() : added_type_count(0) {}

	bool Add_Textures_Material_And_Shader(TextureClass** texs, VertexMaterialClass* mat, ShaderClass shd)
	{
		for (unsigned a=0;a<added_type_count;++a) {
			// Compare textures
			bool all_textures_same = true;
			for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
				all_textures_same = all_textures_same && (texs[stage] == added_textures[stage][a]);
			}
			if (all_textures_same && Equal_Material(mat,added_materials[a]) && shd==added_shaders[a]) {
				return false;
			}
		}
		WWASSERT(added_type_count<MAX_ADDED_TYPE_COUNT);
		for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
			added_textures[stage][added_type_count]=texs[stage];
		}
		added_materials[added_type_count]=mat;
		added_shaders[added_type_count]=shd;
		added_type_count++;
		return true;
	}
};

void DX8FVFCategoryContainer::Generate_Texture_Categories(Vertex_Split_Table& split_table,unsigned vertex_offset)
{
	int polygon_count=split_table.Get_Polygon_Count();
	int index_count=polygon_count*3*split_table.Get_Pass_Count(); 

	/*
	** If we don't have an index buffer yet, allocate one.  Make it hold at least 12000 entries,
	** more if the mesh requires it.  
	*/
	if (!index_buffer) {
		int ib_size=12000;  
		if (ib_size<index_count) ib_size=index_count;
		if (sorting) {
			index_buffer=NEW_REF(SortingIndexBufferClass,(ib_size));
		}
		else {
			index_buffer=NEW_REF(DX8IndexBufferClass,(
				ib_size,
				(DX8Wrapper::Get_Current_Caps()->Support_NPatches() && WW3D::Get_NPatches_Level()>1) ? DX8IndexBufferClass::USAGE_NPATCHES : DX8IndexBufferClass::USAGE_DEFAULT));
		}
	}

	for (unsigned pass=0;pass<split_table.Get_Pass_Count();++pass) {
		Textures_Material_And_Shader_Booking_Struct textures_material_and_shader_booking;

		unsigned old_used_indices=used_indices;

		for (int i=0;i<polygon_count;++i) {
			TextureClass* textures[MeshMatDescClass::MAX_TEX_STAGES];
			WWASSERT(MAX_TEXTURE_STAGES==MeshMatDescClass::MAX_TEX_STAGES);
			for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;stage++) {
				textures[stage]=split_table.Peek_Texture(i,pass,stage);
			}
			VertexMaterialClass* mat=split_table.Peek_Material(i,pass);
			ShaderClass shader=split_table.Peek_Shader(i,pass);
			if (!textures_material_and_shader_booking.Add_Textures_Material_And_Shader(textures,mat,shader)) continue;

			Insert_To_Texture_Category(split_table,textures,mat,shader,pass,vertex_offset);
		}

		[[maybe_unused]] int new_inds=used_indices-old_used_indices;
		WWASSERT(new_inds<=polygon_count*3);
	}
}

// ----------------------------------------------------------------------------

DX8SkinFVFCategoryContainer::DX8SkinFVFCategoryContainer(bool sorting)
	:
	DX8FVFCategoryContainer(DX8_FVF_XYZNUV1,sorting),
	VisibleVertexCount(0),
	VisibleSkinHead(NULL)
{
}

// ----------------------------------------------------------------------------

DX8SkinFVFCategoryContainer::~DX8SkinFVFCategoryContainer()
{
}

// ----------------------------------------------------------------------------

void DX8SkinFVFCategoryContainer::Log([[maybe_unused]] bool only_visible)
{
#ifdef ENABLE_CATEGORY_LOG
	StringClass work(255,true);
	work.Format("DX8SkinFVFCategoryContainer --------------\n");
	WWDEBUG_SAY((work));

	if (index_buffer) {
		work.Format("IB size (used/total): %d/%d\n",used_indices,index_buffer->Get_Index_Count());
		WWDEBUG_SAY((work));
	}
	else {
		WWDEBUG_SAY(("EMPTY IB\n"));
	}

	for (unsigned pass=0;pass<passes;++pass) {
		TextureCategoryListIterator it(&texture_category_list[pass]);
		while (!it.Is_Done()) {
			it.Peek_Obj()->Log(only_visible);
			it.Next();
		}
	}
#endif
}

// ----------------------------------------------------------------------------

void DX8SkinFVFCategoryContainer::Render(void)
{
	SNAPSHOT_SAY(("DX8SkinFVFCategoryContainer::Render()\n"));
	if (!Anything_To_Render()) {
		SNAPSHOT_SAY(("Nothing to render\n"));
		return;
	}
	AnythingToRender=false;

	DX8Wrapper::Set_Vertex_Buffer(NULL);	// Free up the reference to the current vertex buffer
														// (in case it is the dynamic, which may have to be resized)

	DynamicVBAccessClass vb(
		sorting ? BUFFER_TYPE_DYNAMIC_SORTING : BUFFER_TYPE_DYNAMIC_DX8,
		dynamic_fvf_type,
		VisibleVertexCount);
	SNAPSHOT_SAY(("DynamicVBAccess - %s - %d vertices\n",sorting ? "sorting" : "non-sorting",VisibleVertexCount));

	unsigned vertex_offset=0;

	{
		DynamicVBAccessClass::WriteLockClass l(&vb);
		VertexFormatXYZNDUV2 * dest_verts = l.Get_Formatted_Vertex_Array();

		MeshClass * mesh = VisibleSkinHead;
		while (mesh != NULL) {

			MeshModelClass * mmc = mesh->Peek_Model();
			int mesh_vertex_count=mmc->Get_Vertex_Count();

			WWASSERT((vertex_offset+mesh_vertex_count)<=VisibleVertexCount);

			DX8_RECORD_SKIN_RENDER(mesh->Get_Num_Polys(),mesh_vertex_count);

			if (_TempVertexBuffer.Length() < mesh_vertex_count) _TempVertexBuffer.Resize(mesh_vertex_count); 
			if (_TempNormalBuffer.Length() < mesh_vertex_count) _TempNormalBuffer.Resize(mesh_vertex_count);

			Vector3* loc=&(_TempVertexBuffer[0]);
			Vector3* norm=&(_TempNormalBuffer[0]);
			const Vector2* uv0=mmc->Get_UV_Array_By_Index(0);
			const Vector2* uv1=mmc->Get_UV_Array_By_Index(1);
			const unsigned* diffuse=mmc->Get_Color_Array(0,false);

			VertexFormatXYZNDUV2* verts=dest_verts+vertex_offset;

//			mesh->Compose_Deformed_Vertex_Buffer(verts,uv0,uv1,diffuse);
			mesh->Get_Deformed_Vertices(loc,norm);

			for (int v=0;v<mesh_vertex_count;++v) {
				verts[v].x=(*loc)[0];
				verts[v].y=(*loc)[1];
				verts[v].z=(*loc)[2];
				verts[v].nx=(*norm)[0];
				verts[v].ny=(*norm)[1];
				verts[v].nz=(*norm)[2];
				if (diffuse) {
					verts[v].diffuse=*diffuse++;
				}
				else {
					verts[v].diffuse=0;
				}
				if (uv0) {
					verts[v].u1=(*uv0)[0];
					verts[v].v1=(*uv0)[1];
					uv0++;
				}
				else {
					verts[v].u1=0.0f;
					verts[v].v1=0.0f;
				}
				if (uv1) {
					verts[v].u2=(*uv1)[0];
					verts[v].v2=(*uv1)[1];
					uv1++;
				}
				else {
					verts[v].u2=0.0f;
					verts[v].v2=0.0f;
				}

				loc++;
				norm++;
			}


			mesh->Set_Base_Vertex_Offset(vertex_offset);
			vertex_offset+=mesh_vertex_count;
			
			mesh = mesh->Peek_Next_Visible_Skin();
		}
	}
	WWASSERT(vertex_offset==VisibleVertexCount);

	SNAPSHOT_SAY(("Set vb: %x ib: %x\n",vb,index_buffer));

	DX8Wrapper::Set_Vertex_Buffer(vb);
	DX8Wrapper::Set_Index_Buffer(index_buffer,0);

	for (unsigned pass=0;pass<passes;++pass) {
		SNAPSHOT_SAY(("Pass: %d\n",pass));
		while (DX8TextureCategoryClass * tex = visible_texture_category_list[pass].Remove_Head()) {
			tex->Render();
		}
	}

	Render_Procedural_Material_Passes();

	VisibleSkinHead = NULL;
	VisibleVertexCount = 0;
}

bool DX8SkinFVFCategoryContainer::Check_If_Mesh_Fits(MeshModelClass* mmc)
{
	if (!index_buffer) return true;	// No IB created - mesh will fit as a new ib will be created when inserting
	int required_polygons=mmc->Get_Polygon_Count();
	if (mmc->Get_Gap_Filler()) {
		required_polygons+=mmc->Get_Gap_Filler()->Get_Polygon_Count();
	}

	if ((required_polygons*3*mmc->Get_Pass_Count())<=index_buffer->Get_Index_Count()-used_indices) {
		return true;
	}
	return false;
}

void DX8SkinFVFCategoryContainer::Add_Visible_Skin(MeshClass * mesh) 
{
	mesh->Set_Next_Visible_Skin(VisibleSkinHead);
	VisibleSkinHead = mesh;
	VisibleVertexCount += mesh->Peek_Model()->Get_Vertex_Count();
}


// ----------------------------------------------------------------------------

void DX8SkinFVFCategoryContainer::Reset()
{
	VisibleVertexCount = 0;
	VisibleSkinHead = NULL;
	
	for (unsigned pass=0;pass<passes;++pass) {
		while (DX8TextureCategoryClass* texture_category=texture_category_list[pass].Peek_Head()) {
			delete texture_category;
		}
	}

	REF_PTR_RELEASE(index_buffer);
	used_indices=0;
}

// ----------------------------------------------------------------------------

void DX8SkinFVFCategoryContainer::Add_Mesh(MeshClass* mesh)
{
	Vertex_Split_Table split_table(mesh);

	Generate_Texture_Categories(split_table,0);
}

// ----------------------------------------------------------------------------

unsigned DX8TextureCategoryClass::Add_Mesh(
	Vertex_Split_Table& split_table,
	unsigned vertex_offset,
	unsigned index_offset,
	IndexBufferClass* index_buffer,
	unsigned texpass)
{
	int poly_count=split_table.Get_Polygon_Count();

	unsigned index_count=0;

	/*
	** Count the polygons in the given mesh in the given pass which match this texture category
	*/
	unsigned polygons=0;

	for (int i=0;i<poly_count;++i) {
		bool all_textures_same = true;
		for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
			all_textures_same = all_textures_same && (split_table.Peek_Texture(i, texpass, stage) == textures[stage]);
		}
		VertexMaterialClass* mat=split_table.Peek_Material(i,texpass);
		ShaderClass shd=split_table.Peek_Shader(i,texpass);

		if (all_textures_same && Equal_Material(mat,material) && shd==shader) {
			polygons++;
		}
	}

	/*
	** Add the indices for the polygons that match into this renderer's dx8 index table.
	*/
	if (polygons) {

		index_count=polygons*3;
#ifndef ENABLE_STRIPING
		bool stripify=false;
#else
		bool stripify=true;
		if (index_buffer->Type()==BUFFER_TYPE_SORTING || index_buffer->Type()==BUFFER_TYPE_DYNAMIC_SORTING) {
			stripify=false;
		}
#endif
		const TriIndex* src_indices=(const TriIndex*)split_table.Get_Polygon_Array(texpass);//mmc->Get_Polygon_Array();

		if (stripify) {
			int* triangles=new int[index_count];
			int triangle_index_count=0;
			for (int i=0;i<poly_count;++i) {
				bool all_textures_same = true;
				for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
					all_textures_same = all_textures_same && (split_table.Peek_Texture(i, texpass, stage) == textures[stage]);
				}
				VertexMaterialClass* mat=split_table.Peek_Material(i,texpass);
				ShaderClass shd=split_table.Peek_Shader(i,texpass);

				if (all_textures_same && Equal_Material(mat,material) && shd==shader) {
					triangles[triangle_index_count++]=src_indices[i][0]+vertex_offset;
					triangles[triangle_index_count++]=src_indices[i][1]+vertex_offset;
					triangles[triangle_index_count++]=src_indices[i][2]+vertex_offset;
				}
			}

			int* strips=StripOptimizerClass::Stripify(triangles, triangle_index_count/3);
			delete[] triangles;
			int* strip=StripOptimizerClass::Combine_Strips(strips+1,strips[0]);
			delete[] strips;

			if (index_count<unsigned(strip[0])) {
				stripify=false;
			}
			else {
				index_count=strip[0];

				DX8PolygonRendererClass* p_renderer=new DX8PolygonRendererClass(
					index_count,
					split_table.Get_Mesh_Class(),
					this,
					vertex_offset,
					index_offset,
					true);
				PolygonRendererList.Add_Tail(p_renderer);

				{
					IndexBufferClass::AppendLockClass l(index_buffer,index_offset,index_count);
					unsigned short* dst_indices=l.Get_Index_Array();

					unsigned short vmin=0xffff;
					unsigned short vmax=0;

					/*
					** Iterate over the polys for this pass, adding each one that matches this texture+material+shader
					*/
					for (unsigned i=0;i<index_count;++i) {
						unsigned short idx;

						idx=static_cast<unsigned short>(strip[i+1]);
						vmin=MIN(vmin,idx);
						vmax=MAX(vmax,idx);
						*dst_indices++=idx;
					}
					
					/*
					** Remember the min and max vertex indices that these polygons used (for optimization)
					*/
					p_renderer->Set_Vertex_Index_Range(vmin,vmax-vmin+1);
				}
			}
			delete[] strip;
		}

		// Need to check stripify again as it may be changed to false by the previous statement
		if (!stripify ) {
			DX8PolygonRendererClass* p_renderer=new DX8PolygonRendererClass(
				index_count,
				split_table.Get_Mesh_Class(),
				this,
				vertex_offset,
				index_offset,
				false);
			PolygonRendererList.Add_Tail(p_renderer);

			IndexBufferClass::AppendLockClass l(index_buffer,index_offset,index_count);
			unsigned short* dst_indices=l.Get_Index_Array();

			unsigned short vmin=0xffff;
			unsigned short vmax=0;

			/*
			** Iterate over the polys for this pass, adding each one that matches this texture+material+shader
			*/
			for (int i=0;i<poly_count;++i) {
				bool all_textures_same = true;
				for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
					all_textures_same = all_textures_same && (split_table.Peek_Texture(i, texpass, stage) == textures[stage]);
				}
				VertexMaterialClass* mat=split_table.Peek_Material(i,texpass);
				ShaderClass shd=split_table.Peek_Shader(i,texpass);

				if (all_textures_same && Equal_Material(mat,material) && shd==shader) {
					unsigned short idx;

					idx=static_cast<unsigned short>(src_indices[i][0]+vertex_offset);
					vmin=MIN(vmin,idx);
					vmax=MAX(vmax,idx);
					*dst_indices++=idx;
//					WWDEBUG_SAY(("%d, ",idx));

					idx=static_cast<unsigned short>(src_indices[i][1]+vertex_offset);
					vmin=MIN(vmin,idx);
					vmax=MAX(vmax,idx);
					*dst_indices++=idx;
//					WWDEBUG_SAY(("%d, ",idx));

					idx=static_cast<unsigned short>(src_indices[i][2]+vertex_offset);
					vmin=MIN(vmin,idx);
					vmax=MAX(vmax,idx);
					*dst_indices++=idx;
//					WWDEBUG_SAY(("%d\n",idx));
				}
			}

			WWASSERT((vmax-vmin)<split_table.Get_Mesh_Model_Class()->Get_Vertex_Count());
			
			/*
			** Remember the min and max vertex indices that these polygons used (for optimization)
			*/
			p_renderer->Set_Vertex_Index_Range(vmin,vmax-vmin+1);
			WWASSERT(index_count<=static_cast<unsigned>(split_table.Get_Polygon_Count()*3));
		}
	}

	return index_count;
}

// ----------------------------------------------------------------------------

void DX8TextureCategoryClass::Render(void)
{
	#ifdef WWDEBUG
	if (!WW3D::Expose_Prelit()) {
	#endif

		for (unsigned i=0;i<MAX_TEXTURE_STAGES;++i) {
			SNAPSHOT_SAY(("Set_Texture(%d,%s)\n",i,Peek_Texture(i) ? static_cast<const char *>(Peek_Texture(i)->Get_Texture_Name()) : "NULL"));
			DX8Wrapper::Set_Texture(i,Peek_Texture(i));
		}

	#ifdef WWDEBUG
	}
	#endif

	SNAPSHOT_SAY(("Set_Material(%s)\n",Peek_Material() ? static_cast<const char *>(Peek_Material()->Get_Name()) : "NULL"));
	DX8Wrapper::Set_Material(Peek_Material());

	SNAPSHOT_SAY(("Set_Shader(0x%x)\n",Get_Shader()));
	DX8Wrapper::Set_Shader(Get_Shader());
	
	PolyRenderTaskClass * prt = render_task_head;
	while (prt) {

		/*
		** Dig out the parameters for this render task
		*/
		DX8PolygonRendererClass * renderer = prt->Peek_Polygon_Renderer();
		MeshClass * mesh = prt->Peek_Mesh();

		SNAPSHOT_SAY(("mesh = %s\n",mesh->Get_Name()));

		#ifdef WWDEBUG	
		// Debug rendering: if it exists, expose prelighting on this mesh by disabling all base textures.
		if (WW3D::Expose_Prelit()) {
			switch (mesh->Peek_Model()->Get_Flag (MeshGeometryClass::PRELIT_MASK)) {

				unsigned i;

				case MeshGeometryClass::PRELIT_VERTEX:
					
					// Disable texturing on all stages and passes.
					for (i = 0; i < MAX_TEXTURE_STAGES; i++) {
						DX8Wrapper::Set_Texture (i, NULL);
					}
					break;

				case MeshGeometryClass::PRELIT_LIGHTMAP_MULTI_PASS:
					
					// Disable texturing on all but the last pass.
					if (pass == mesh->Peek_Model()->Get_Pass_Count() - 1) {
						for (i = 0; i < MAX_TEXTURE_STAGES; i++) {
							DX8Wrapper::Set_Texture (i, Peek_Texture (i));
						}
					} else {
						for (i = 0; i < MAX_TEXTURE_STAGES; i++) {
							DX8Wrapper::Set_Texture (i, NULL);
						}
					}
					break;

				case MeshGeometryClass::PRELIT_LIGHTMAP_MULTI_TEXTURE:
					
					// Disable texturing on all but the zeroth stage of each pass.
					DX8Wrapper::Set_Texture (0, Peek_Texture (0));
					for (i = 1; i < MAX_TEXTURE_STAGES; i++) {
						DX8Wrapper::Set_Texture (i, NULL);
					}
					break;

				default:
					for (i = 0; i < MAX_TEXTURE_STAGES; i++) {
						DX8Wrapper::Set_Texture (i, Peek_Texture (i));
					}
					break;
			}
		}
		#endif

		/*
		** If the user is not installing LightEnvironmentClasses, we leave the lighting render
		** states untouched.  This way they can set a couple global lights that affect the entire scene.
		*/
		LightEnvironmentClass * lenv = mesh->Get_Lighting_Environment();
		if (lenv != NULL) {
			SNAPSHOT_SAY(("LightEnvironment, lights: %d\n",lenv->Get_Light_Count()));
			DX8Wrapper::Set_Light_Environment(lenv);
		}
		else {
			SNAPSHOT_SAY(("No light environment\n"));
		}

		/*
		** Support for ALIGNED and ORIENTED camera modes
		*/
		const Matrix3D* world_transform = &mesh->Get_Transform();
		bool identity=mesh->Is_Transform_Identity();
		Matrix3D tmp_world;

		if (mesh->Peek_Model()->Get_Flag(MeshModelClass::ALIGNED)) {
			SNAPSHOT_SAY(("Camera mode ALIGNED\n"));

			Vector3 mesh_position;
			Vector3 camera_z_vector;
			
			TheDX8MeshRenderer.Peek_Camera()->Get_Transform().Get_Z_Vector(&camera_z_vector);
			mesh->Get_Transform().Get_Translation(&mesh_position);

			tmp_world.Obj_Look_At(mesh_position,mesh_position + camera_z_vector,0.0f);
			world_transform = &tmp_world;

		} else if (mesh->Peek_Model()->Get_Flag(MeshModelClass::ORIENTED)) {
			SNAPSHOT_SAY(("Camera mode ORIENTED\n"));
		
			Vector3 mesh_position;
			Vector3 camera_position;

			TheDX8MeshRenderer.Peek_Camera()->Get_Transform().Get_Translation(&camera_position);
			mesh->Get_Transform().Get_Translation(&mesh_position);

			tmp_world.Obj_Look_At(mesh_position,camera_position,0.0f);
			world_transform = &tmp_world;
		
		} else if (mesh->Peek_Model()->Get_Flag(MeshModelClass::SKIN)) {
			SNAPSHOT_SAY(("Set world identity (for skin)\n"));
			
			tmp_world.Make_Identity();
			world_transform = &tmp_world;
			identity=true;
		}


		if (identity) {
			SNAPSHOT_SAY(("Set_World_Identity\n"));
			DX8Wrapper::Set_World_Identity();
		}
		else {
			SNAPSHOT_SAY(("Set_World_Transform\n"));
			DX8Wrapper::Set_Transform(D3DTS_WORLD,*world_transform);
		}

		// The mesh renderer debugger can disable mesh rendering
		if (!DX8RendererDebugger::Is_Enabled() || !mesh->Is_Disabled_By_Debugger()) {
			/*
			** Render mesh using either sorting or immediate pipeline
			*/
			if ((!!mesh->Peek_Model()->Get_Flag(MeshGeometryClass::SORT)) && WW3D::Is_Sorting_Enabled()) {
				renderer->Render_Sorted(mesh->Get_Base_Vertex_Offset(),mesh->Get_Bounding_Sphere());
			} else {
				renderer->Render(mesh->Get_Base_Vertex_Offset());
			}
		}

		/*
		** Move to the next render task.  Note that the delete should be fast because prt's are pooled
		*/
		PolyRenderTaskClass * next_prt = prt->Get_Next_Visible();
		delete prt;
		prt = next_prt;
	}

	Clear_Render_List();
}


DX8MeshRendererClass::DX8MeshRendererClass()
	:
	camera(NULL),
	enable_lighting(true),
	texture_category_container_list_skin(NULL),
	visible_decal_meshes(NULL)
{
}

DX8MeshRendererClass::~DX8MeshRendererClass()
{
	Invalidate();
	Clear_Pending_Delete_Lists();
	if (texture_category_container_list_skin != NULL) {
		delete texture_category_container_list_skin;
	}
}

void DX8MeshRendererClass::Init(void)
{
	texture_category_container_list_skin = new FVFCategoryList;
}

void DX8MeshRendererClass::Shutdown(void)
{
	Invalidate();
	Clear_Pending_Delete_Lists();
}

// ----------------------------------------------------------------------------

void DX8MeshRendererClass::Clear_Pending_Delete_Lists()
{
	while (DX8TextureCategoryClass* category=texture_category_delete_list.Remove_Head()) {
		delete category;
	}
	while (DX8FVFCategoryContainer* container=fvf_category_container_delete_list.Remove_Head()) {
		delete container;
	}
}

// ----------------------------------------------------------------------------

static void Add_Rigid_Mesh_To_Container(FVFCategoryList* container_list,unsigned fvf,MeshClass* mesh)
{
	WWASSERT(container_list);
	DX8FVFCategoryContainer * container = NULL;
	MeshModelClass * mmc = mesh->Peek_Model();
	bool sorting=((!!mmc->Get_Flag(MeshModelClass::SORT)) && WW3D::Is_Sorting_Enabled() && (mmc->Get_Sort_Level() == SORT_LEVEL_NONE));

	FVFCategoryListIterator it(container_list);
	while (!it.Is_Done()) {
		container = it.Peek_Obj();
		if (sorting==container->Is_Sorting() && container->Check_If_Mesh_Fits(mmc)) {
			container->Add_Mesh(mesh);
			return;
		}
		it.Next();
	}

	container=new DX8RigidFVFCategoryContainer(fvf,sorting);
	container_list->Add_Tail(container);
	container->Add_Mesh(mesh);
}

// ----------------------------------------------------------------------------

void DX8MeshRendererClass::Unregister_Mesh_Type(MeshClass* mesh)
{
	while (DX8PolygonRendererClass* n=mesh->PolygonRendererList.Remove_Head()) {
		delete n;
	}
	_RegisteredMeshTable.Remove(MeshRegKeyStruct(mesh->Peek_Model(),mesh->Get_User_Lighting_Array()),mesh);

	// Also remove the gap filler!
	MeshModelClass * mmc = mesh->Peek_Model();
	if (mmc->GapFiller) {
		GapFillerClass* gf=mmc->GapFiller;
		mmc->GapFiller=NULL;
		delete gf;
	}
}


void DX8MeshRendererClass::Register_Mesh_Type(MeshClass* mesh)
{
	WWMEMLOG(MEM_GEOMETRY);
	MeshModelClass * mmc = mesh->Peek_Model();
#ifdef ENABLE_CATEGORY_LOG
	WWDEBUG_SAY(("Registering mesh: %s (%d polys, %d verts + %d gap polygons)\n",mmc->Get_Name(),mmc->Get_Polygon_Count(),mmc->Get_Vertex_Count(),mmc->Get_Gap_Filler_Polygon_Count()));
#endif
	bool skin=(mmc->Get_Flag(MeshModelClass::SKIN) && mmc->VertexBoneLink);
	bool sorting=((!!mmc->Get_Flag(MeshModelClass::SORT)) && WW3D::Is_Sorting_Enabled() && (mmc->Get_Sort_Level() == SORT_LEVEL_NONE));

	if (skin) {

		/*
		** This mesh is a skin.  Add it to a DX8SkinFVFCategoryContainer.
		*/
		WWASSERT(texture_category_container_list_skin);

		FVFCategoryListIterator it(texture_category_container_list_skin);
		while (!it.Is_Done()) {
			DX8FVFCategoryContainer * container = it.Peek_Obj();
			if (sorting==container->Is_Sorting() && container->Check_If_Mesh_Fits(mmc)) {
				container->Add_Mesh(mesh);
				return;
			}
			it.Next();
		}

		DX8FVFCategoryContainer * new_container=new DX8SkinFVFCategoryContainer(sorting);
		texture_category_container_list_skin->Add_Tail(new_container);
		new_container->Add_Mesh(mesh);
	
	} else {

		unsigned int * user_lighting = mesh->Get_User_Lighting_Array();
		MeshClass * existing_mesh = _RegisteredMeshTable.Get(MeshRegKeyStruct(mmc,user_lighting));
		if (existing_mesh != NULL) {

			// We found another instance of this mesh model so we can simply clone the poly renderers
			DX8PolygonRendererListIterator it(&(existing_mesh->PolygonRendererList));
			while (!it.Is_Done()) {
				DX8PolygonRendererClass * src_renderer=it.Peek_Obj();
				DX8PolygonRendererClass * new_renderer = new DX8PolygonRendererClass(*src_renderer,mesh);
				src_renderer->Get_Texture_Category()->Add_Polygon_Renderer(new_renderer);
				it.Next();
			}

		} else {

			// This mesh model either is not registered yet or has been registered with a 
			// different user lighting array.  Simply add it into the system as if it is a 
			// completely unique mesh.
			unsigned fvf=DX8FVFCategoryContainer::Define_FVF(mmc,user_lighting,enable_lighting);

			/*
			** Search for an existing FVF Category Container that matches this mesh
			*/
			int i;
			for (i=0;i<texture_category_container_lists_rigid.Count();++i) {
				FVFCategoryList * list=texture_category_container_lists_rigid[i];
				WWASSERT(list);
				DX8FVFCategoryContainer * container=list->Peek_Head();
				if (container && container->Get_FVF()!=fvf) continue;

				Add_Rigid_Mesh_To_Container(list,fvf,mesh);
				break;
			}

			if (i==texture_category_container_lists_rigid.Count()) {

				/*
				** We couldn't find an existing FVF category container so we have to add one.  Future
				** meshes that use this FVF will also be able to use this container.
				*/
				FVFCategoryList * new_fvf_category = new FVFCategoryList();
				texture_category_container_lists_rigid.Add(new_fvf_category);
				Add_Rigid_Mesh_To_Container(new_fvf_category,fvf,mesh);
			}

			/*
			** Done processing the mesh, add its polygon renderers to the global registered mesh list
			*/
			if (mesh->PolygonRendererList.Is_Empty() == false) {
				_RegisteredMeshTable.Insert(MeshRegKeyStruct(mmc,user_lighting),mesh);
			}
			else {
				WWDEBUG_SAY(("Error: Register_Mesh_Type failed! file: %s line: %d\r\n",__FILE__,__LINE__));
			}
		}
	}
	return;
}

static unsigned statistics_requested=0;

void DX8MeshRendererClass::Request_Log_Statistics()
{
	statistics_requested=WW3D::Get_Frame_Count();
}


// ---------------------------------------------------------------------------
//
// Render all meshes that are added to visible lists
//
// ---------------------------------------------------------------------------

static void Render_FVF_Category_Container_List(FVFCategoryList& list)
{
	FVFCategoryListIterator it(&list);
	while (!it.Is_Done()) {
		it.Peek_Obj()->Render();
		it.Next();
	}
}

static void Render_FVF_Category_Container_List_Delayed_Passes(FVFCategoryList& list)
{
	FVFCategoryListIterator it(&list);
	while (!it.Is_Done()) {
		it.Peek_Obj()->Render_Delayed_Procedural_Material_Passes();
		it.Next();
	}
}

void DX8MeshRendererClass::Flush(void)
{
	int i;

	WWPROFILE("DX8MeshRenderer::Flush");
	if (!camera) return;
	Log_Statistics_String(true);	

	/*
	** Render the FVF categories.  Note that it is critical that skins be 
	** rendered *after* the rigid meshes.  This is caused by the fact that an object may
	** have its base passes disabled and a translucent procedural material pass rendered
	** instead.  In this case, technically we have to delay rendering of the material pass but
	** for skins we just render these passes as we go because we can assume that the
	** bulk of the meshes have already been drawn (there would be extra overhead involved
	** in solving this for skins)
	*/
	for (i=0;i<texture_category_container_lists_rigid.Count();++i) {
		Render_FVF_Category_Container_List(*texture_category_container_lists_rigid[i]);
	}

	Render_FVF_Category_Container_List(*texture_category_container_list_skin);

	Render_Decal_Meshes();

	/*
	** Render the translucent procedural material passes that were applied to meshes that
	** had their base passes disabled. 
	*/
	for (i=0;i<texture_category_container_lists_rigid.Count();++i) {
		Render_FVF_Category_Container_List_Delayed_Passes(*texture_category_container_lists_rigid[i]);
	}

	DX8Wrapper::Set_Vertex_Buffer(NULL);
	DX8Wrapper::Set_Index_Buffer(NULL,0);
}


void DX8MeshRendererClass::Add_To_Render_List(DecalMeshClass * decalmesh)
{
	WWASSERT(decalmesh != NULL);
	decalmesh->Set_Next_Visible(visible_decal_meshes);
	visible_decal_meshes = decalmesh;
}

static inline DWORD Float2Unsigned(float f) {
	return std::bit_cast<DWORD>(f);

}
void DX8MeshRendererClass::Render_Decal_Meshes()
{
	const float slope_scale = 0.0f;    
	const float const_bias = -0.001f; 

	DX8Wrapper::Set_DX8_Render_State(D3DRS_SLOPESCALEDEPTHBIAS, Float2Unsigned(slope_scale));
	DX8Wrapper::Set_DX8_Render_State(D3DRS_DEPTHBIAS, Float2Unsigned(const_bias));

	for (DecalMeshClass* d = visible_decal_meshes; d; d = d->Peek_Next_Visible()) {
		d->Render();
	}
	visible_decal_meshes = nullptr;

	DX8Wrapper::Set_DX8_Render_State(D3DRS_SLOPESCALEDEPTHBIAS, 0);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_DEPTHBIAS, 0);
}

// ----------------------------------------------------------------------------

static void Log_Container_List(FVFCategoryList& container_list,bool only_visible)
{
	FVFCategoryListIterator it(&container_list);
	while (!it.Is_Done()) {
		it.Peek_Obj()->Log(only_visible);
		it.Next();
	}
}

void DX8MeshRendererClass::Log_Statistics_String(bool only_visible)
{
	if (statistics_requested!=WW3D::Get_Frame_Count()) return;

	for (int i=0;i<texture_category_container_lists_rigid.Count();++i) {
		Log_Container_List(*texture_category_container_lists_rigid[i],only_visible);
	}
	Log_Container_List(*texture_category_container_list_skin,only_visible);

}

static void Invalidate_FVF_Category_Container_List(FVFCategoryList& list)
{
	while (DX8FVFCategoryContainer* fvf_category=list.Remove_Head()) {
		delete fvf_category;
	}
}

void DX8MeshRendererClass::Invalidate()
{
	WWMEMLOG(MEM_RENDERER);
	_RegisteredMeshTable.Remove_All();

	for (int i=0;i<texture_category_container_lists_rigid.Count();++i) {
		Invalidate_FVF_Category_Container_List(*texture_category_container_lists_rigid[i]);
		delete texture_category_container_lists_rigid[i];
	}
	if (texture_category_container_list_skin) {
		Invalidate_FVF_Category_Container_List(*texture_category_container_list_skin);
		delete texture_category_container_list_skin;
		texture_category_container_list_skin=NULL;
	}
	texture_category_container_list_skin = new FVFCategoryList;

	texture_category_container_lists_rigid.Delete_All();
}







