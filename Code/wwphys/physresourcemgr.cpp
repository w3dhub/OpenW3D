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
 *                     $Archive:: /Commando/Code/wwphys/physresourcemgr.cpp                   $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/20/01 3:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "physresourcemgr.h"
#include "vertmaterial.h"
#include "texture.h"
#include "matpass.h"
#include "assetmgr.h"


/**
** Resources that the physics resource manager can allocate-on-demand
*/
static TextureClass *			_ShadowBlobTexture = NULL;
static MaterialPassClass *		_HighlightMaterialPass = NULL;
static TextureClass *			_StealthTexture = NULL;
static TextureClass *			_GridTexture = NULL;



void PhysResourceMgrClass::Init(void)
{
}

void PhysResourceMgrClass::Shutdown(void)
{
	REF_PTR_RELEASE(_ShadowBlobTexture);
	REF_PTR_RELEASE(_HighlightMaterialPass);
	REF_PTR_RELEASE(_StealthTexture);
}

bool PhysResourceMgrClass::Set_Shadow_Blob_Texture(const char * texname)
{
	if (texname == NULL) return false;

	TextureClass * tex = WW3DAssetManager::Get_Instance()->Get_Texture(texname);
	if (tex == NULL) return false;

	REF_PTR_SET(_ShadowBlobTexture,tex);
	tex->Release_Ref();

	_ShadowBlobTexture->Set_U_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);
	_ShadowBlobTexture->Set_V_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);
	return true;
}

TextureClass *	PhysResourceMgrClass::Get_Shadow_Blob_Texture(void)
{
	if (_ShadowBlobTexture == NULL) {
		_ShadowBlobTexture = WW3DAssetManager::Get_Instance()->Get_Texture("shadowblob.tga");
		_ShadowBlobTexture->Set_U_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);
		_ShadowBlobTexture->Set_V_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);
	}
	WWASSERT(_ShadowBlobTexture != NULL);
	_ShadowBlobTexture->Add_Ref();
	return _ShadowBlobTexture;
}

MaterialPassClass * PhysResourceMgrClass::Get_Highlight_Material_Pass(void)
{
	// If we haven't initialized the highlight material, do it now.
	if (_HighlightMaterialPass == NULL) {

		// otherwise, create and initialize it
		_HighlightMaterialPass = NEW_REF(MaterialPassClass,());

		VertexMaterialClass * vmtl = NEW_REF(VertexMaterialClass,());
		vmtl->Set_Ambient(0,0,0);
		vmtl->Set_Diffuse(0,0,0);
		vmtl->Set_Specular(0,0,0);
		vmtl->Set_Emissive(0.2f,1.0f,0.2f);
		vmtl->Set_Opacity(1.0f);
		vmtl->Set_Shininess(0.0f);
		vmtl->Set_Lighting(true);

		_HighlightMaterialPass->Set_Material(vmtl);

		REF_PTR_RELEASE(vmtl);

	}

	ShaderClass shader = ShaderClass::_PresetOpaqueShader;
	shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
	shader.Set_Depth_Compare(ShaderClass::PASS_ALWAYS);
	_HighlightMaterialPass->Set_Shader(shader);

	_HighlightMaterialPass->Add_Ref();
	return _HighlightMaterialPass;
}

TextureClass * PhysResourceMgrClass::Get_Stealth_Texture(void)
{
	TextureClass * tex = Peek_Stealth_Texture();
	if (tex) {
		tex->Add_Ref();
	}
	return tex;
}

TextureClass * PhysResourceMgrClass::Peek_Stealth_Texture(void)
{
	if (_StealthTexture == NULL) {
		_StealthTexture = WW3DAssetManager::Get_Instance()->Get_Texture("stealth_effect.tga");
	}
	return _StealthTexture;
}


VertexMaterialClass * PhysResourceMgrClass::Create_Emissive_Material(void)
{
	VertexMaterialClass * vmtl = NEW_REF(VertexMaterialClass,());
	vmtl->Set_Ambient(0,0,0);
	vmtl->Set_Diffuse(0,0,0);
	vmtl->Set_Specular(0,0,0);
	vmtl->Set_Emissive(1.0f,1.0f,1.0f);
	vmtl->Set_Opacity(1.0f);
	vmtl->Set_Shininess(0.0f);
	vmtl->Set_Lighting(true);
	return vmtl;
}

TextureClass * PhysResourceMgrClass::Get_Grid_Texture(void)
{
	TextureClass * tex = Peek_Grid_Texture();
	if (tex != NULL) {
		tex->Add_Ref();
	}
	return tex;
}

TextureClass * PhysResourceMgrClass::Peek_Grid_Texture(void)
{
	if (_GridTexture == NULL) {
		_GridTexture = WW3DAssetManager::Get_Instance()->Get_Texture("grid_effect.tga");
	}
	return _GridTexture;
}

