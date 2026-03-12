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
 *                 Project Name : ENBAssetMgr                                                  *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/ww3d2/hueshift/enbassetmgr.cpp                    $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 10:35p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "enbassetmgr.h"
#include "proto.h"
#include "rendobj.h"
#include "vector3.h"
#include "mesh.h"
#include "hlod.h"
#include "matinfo.h"
#include "meshmdl.h"
#include "part_emt.h"
#include "vertmaterial.h"
#include "dx8wrapper.h"
#include "texture.h"
#include "surfaceclass.h"
#include "textureloader.h"
#include "ww3dformat.h"
#include "colorspace.h"
#include <stdio.h>

//---------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------

const float ident_scale(1.0f);
const Vector3 ident_HSV(0,0,0);

//---------------------------------------------------------------------
// ENBPrototype
//---------------------------------------------------------------------

class ENBPrototypeClass : public PrototypeClass
{
public:
	ENBPrototypeClass(RenderObjClass * proto,const char* name);
	virtual ~ENBPrototypeClass(void);

	virtual const char *			Get_Name(void) const;
	virtual int						Get_Class_ID(void) const;
	virtual RenderObjClass *	Create(void);
	RenderObjClass *				Proto;
	char *							Name;
};

ENBPrototypeClass::ENBPrototypeClass(RenderObjClass * proto, const char *name)
{
	Name=strdup(name);
	Proto = proto;
	assert(Proto);
	Proto->Add_Ref();
}

ENBPrototypeClass::~ENBPrototypeClass(void)
{
	delete [] Name;
	if (Proto) {
		Proto->Release_Ref();
	}
}

const char * ENBPrototypeClass::Get_Name(void) const
{
	return Name;
}

int ENBPrototypeClass::Get_Class_ID(void) const
{
	return Proto->Class_ID();
}

RenderObjClass * ENBPrototypeClass::Create(void)
{
	return (RenderObjClass *)( SET_REF_OWNER( Proto->Clone() ) );
}

//---------------------------------------------------------------------
// ENBAssetManager
//---------------------------------------------------------------------

ENBAssetManager::ENBAssetManager(void)
{
}

ENBAssetManager::~ENBAssetManager(void)
{
}

RenderObjClass * ENBAssetManager::Create_Render_Obj(const char * name,float Scale,Vector3 &HSV_shift)
{
	char newname[512];
	bool scale,hsv_shift;
	scale=(Scale!=ident_scale);
	hsv_shift=(HSV_shift!=ident_HSV);

	// base case, no scale or hue shifting
	if (!scale && !hsv_shift) return WW3DAssetManager::Create_Render_Obj(name);

	sprintf(newname,"#%s!%gH%gS%gV%g",name,Scale,HSV_shift.X,HSV_shift.Y,HSV_shift.Z);

	// see if we got a cached version
	RenderObjClass *rendobj=NULL;
	rendobj=WW3DAssetManager::Create_Render_Obj(newname);
	if (rendobj) return rendobj;

	// create a new one based on
	// exisiting prototype
	rendobj=WW3DAssetManager::Create_Render_Obj(name);
	if (!rendobj) return NULL;

	Make_Unique(rendobj,scale,hsv_shift);
	if (scale) rendobj->Scale(Scale);
	if (hsv_shift) Recolor_Asset(rendobj,HSV_shift);

	ENBPrototypeClass *proto=new ENBPrototypeClass(rendobj,newname);
	Add_Prototype(proto);

	return rendobj;
}

//---------------------------------------------------------------------
// Uniqing
//---------------------------------------------------------------------

void ENBAssetManager::Make_Mesh_Unique(RenderObjClass *robj,bool geometry, bool colors)
{
	int i;
	MeshClass *mesh=(MeshClass*) robj;
	mesh->Make_Unique();
	MeshModelClass * model = mesh->Get_Model();

	if (colors)	{
		// make all vertex materials unique
		MaterialInfoClass	*material = mesh->Get_Material_Info();
		for (i=0; i<material->Vertex_Material_Count(); i++)
			material->Peek_Vertex_Material(i)->Make_Unique();
		REF_PTR_RELEASE(material);
		// make all color arrays unique
		model->Make_Color_Array_Unique(0);
		model->Make_Color_Array_Unique(1);
		// do not do textures yet
		// because we want to do the color conversion
		// for the top mip level and then
		// mip filter instead of converting all mip levels
	}

	if (geometry)	{
		// make geometry unique
		model->Make_Geometry_Unique();
	}

	REF_PTR_RELEASE(model);
}

void ENBAssetManager::Make_HLOD_Unique(RenderObjClass *robj,bool geometry, bool colors)
{
	int num_sub = robj->Get_Num_Sub_Objects();
	for(int i = 0; i < num_sub; i++) {
		RenderObjClass *sub_obj = robj->Get_Sub_Object(i);
		Make_Unique(sub_obj,geometry,colors);
		REF_PTR_RELEASE(sub_obj);
	}
}

void ENBAssetManager::Make_Unique(RenderObjClass *robj,bool geometry, bool colors)
{
	switch (robj->Class_ID())	{
	case RenderObjClass::CLASSID_MESH:
		Make_Mesh_Unique(robj,geometry,colors);
		break;
	case RenderObjClass::CLASSID_HLOD:
		Make_HLOD_Unique(robj,geometry,colors);
		break;
	}
}

//---------------------------------------------------------------------
// Coloring
//---------------------------------------------------------------------

void ENBAssetManager::Recolor_Vertex_Material(VertexMaterialClass *vmat,Vector3 &hsv_shift)
{
	Vector3 rgb;

	vmat->Get_Ambient(&rgb);
	Recolor(rgb,hsv_shift);
	vmat->Set_Ambient(rgb);

	vmat->Get_Diffuse(&rgb);
	Recolor(rgb,hsv_shift);
	vmat->Set_Diffuse(rgb);

	vmat->Get_Emissive(&rgb);
	Recolor(rgb,hsv_shift);
	vmat->Set_Emissive(rgb);

	vmat->Get_Specular(&rgb);
	Recolor(rgb,hsv_shift);
	vmat->Set_Specular(rgb);
}

void ENBAssetManager::Recolor_Vertices(unsigned int *color, int count, Vector3 &hsv_shift)
{
	int i;
	Vector4 rgba;

	for (i=0; i<count; i++)
	{
		rgba=DX8Wrapper::Convert_Color(color[i]);
		Recolor(reinterpret_cast<Vector3&>(rgba),hsv_shift);
		color[i]=DX8Wrapper::Convert_Color_Clamp(rgba);
	}
}

TextureClass * ENBAssetManager::Recolor_Texture(TextureClass *texture, Vector3 &hsv_shift)
{
	const char *name=texture->Get_Name();

	// if texture is procedural return NULL
	if (name && name[0]=='!') return NULL;

	// make sure texture is loaded
	if (!texture->Is_Initialized())
		TextureLoader::Request_High_Priority_Loading(texture,0,(TextureClass::MipCountType) texture->Get_Mip_Level_Count());

	SurfaceClass::SurfaceDescription desc;
	SurfaceClass *newsurf, *oldsurf, *smallsurf;
	texture->Get_Level_Description(desc);

	// if texture is monochrome and no value shifting
	// return NULL
	smallsurf=texture->Get_Surface_Level((TextureClass::MipCountType)texture->Get_Mip_Level_Count()-1);
	if (hsv_shift.Z==0.0f && smallsurf->Is_Monochrome())
	{
		REF_PTR_RELEASE(smallsurf);
		return NULL;
	}
	REF_PTR_RELEASE(smallsurf);

	oldsurf=texture->Get_Surface_Level();

	// see if we have a cached copy
	char newname[512];
	StringClass lower_case_name(texture->Get_Name(),true);
	_strlwr(lower_case_name.Peek_Buffer());
	sprintf(newname,"#%s!H%gS%gV%g",lower_case_name,hsv_shift.X,hsv_shift.Y,hsv_shift.Z);
	TextureClass* newtex = TextureHash.Get(newname);
	if (newtex)
	{
		newtex->Add_Ref();
		REF_PTR_RELEASE(oldsurf);
		return newtex;
	}

	// no cached copy, make new recolorized texture
	newsurf=NEW_REF(SurfaceClass,(desc.Width,desc.Height,desc.Format));
	newsurf->Copy(0,0,0,0,desc.Width,desc.Height,oldsurf);
	newsurf->Hue_Shift(hsv_shift);
	newtex=NEW_REF(TextureClass,(newsurf,(TextureClass::MipCountType)texture->Get_Mip_Level_Count()));
	newtex->Set_Mag_Filter(texture->Get_Mag_Filter());
	newtex->Set_Min_Filter(texture->Get_Min_Filter());
	newtex->Set_Mip_Mapping(texture->Get_Mip_Mapping());
	newtex->Set_Name(newname);
	newtex->Set_U_Addr_Mode(texture->Get_U_Addr_Mode());
	newtex->Set_V_Addr_Mode(texture->Get_V_Addr_Mode());

	TextureHash.Insert(newname,newtex);
	newtex->Add_Ref();

	REF_PTR_RELEASE(oldsurf);
	REF_PTR_RELEASE(newsurf);

	return newtex;
}

void ENBAssetManager::Recolor_Mesh(RenderObjClass *robj,Vector3 &hsv_shift)
{
	int i;

	MeshClass *mesh=(MeshClass*) robj;
	MeshModelClass * model = mesh->Get_Model();
	MaterialInfoClass	*material = mesh->Get_Material_Info();

	// recolor vertex material
	for (i=0; i<material->Vertex_Material_Count(); i++)
			Recolor_Vertex_Material(material->Peek_Vertex_Material(i),hsv_shift);

	// recolor color arrays
	unsigned int * color;
	color=model->Get_Color_Array(0,false);
	if (color) Recolor_Vertices(color,model->Get_Vertex_Count(),hsv_shift);
	color=model->Get_Color_Array(1,false);
	if (color) Recolor_Vertices(color,model->Get_Vertex_Count(),hsv_shift);

	// recolor textures

	TextureClass *newtex,*oldtex;
	for (i=0; i<material->Texture_Count(); i++)
	{
		oldtex=material->Peek_Texture(i);
		newtex=Recolor_Texture(oldtex,hsv_shift);
		if (newtex)
		{
			model->Replace_Texture(oldtex,newtex);
			material->Replace_Texture(i,newtex);
			REF_PTR_RELEASE(newtex);
		}
	}

	REF_PTR_RELEASE(material);
	REF_PTR_RELEASE(model);
}

void ENBAssetManager::Recolor_HLOD(RenderObjClass *robj,Vector3 &hsv_shift)
{
	int num_sub = robj->Get_Num_Sub_Objects();
	for(int i = 0; i < num_sub; i++) {
		RenderObjClass *sub_obj = robj->Get_Sub_Object(i);
		Recolor_Asset(sub_obj,hsv_shift);
		REF_PTR_RELEASE(sub_obj);
	}
}

void ENBAssetManager::Recolor_ParticleEmitter(RenderObjClass *robj,Vector3 &hsv_shift)
{
	unsigned int i;

	ParticleEmitterClass* emit=(ParticleEmitterClass*) robj;
	ParticlePropertyStruct<Vector3> colors;

	emit->Get_Color_Key_Frames(colors);
	Recolor(colors.Start,hsv_shift);
	Recolor(colors.Rand,hsv_shift);
	for (i=0; i<colors.NumKeyFrames; i++)
		Recolor(colors.Values[i],hsv_shift);
	emit->Reset_Colors(colors);

	delete colors.Values;
	delete colors.KeyTimes;

	TextureClass *tex=emit->Get_Texture();
	TextureClass *newtex=Recolor_Texture(tex,hsv_shift);
	if (newtex)
	{
		emit->Set_Texture(newtex);
		REF_PTR_RELEASE(newtex);
	}
	REF_PTR_RELEASE(tex);
}

void ENBAssetManager::Recolor_Asset(RenderObjClass *robj,Vector3 &hsv_shift)
{
	switch (robj->Class_ID())	{
	case RenderObjClass::CLASSID_MESH:
		Recolor_Mesh(robj,hsv_shift);
		break;
	case RenderObjClass::CLASSID_HLOD:
		Recolor_HLOD(robj,hsv_shift);
		break;
	case RenderObjClass::CLASSID_PARTICLEEMITTER:
		Recolor_ParticleEmitter(robj,hsv_shift);
		break;
	}
}
