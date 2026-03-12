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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/projectormanager.cpp                  $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/14/01 7:58p                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "projectormanager.h"
#include "colmathaabox.h"
#include "pscene.h"
#include "assetmgr.h"
#include "chunkio.h"
#include "phystexproject.h"
#include "phys.h"
#include "texture.h"


/*
** load up a texture.  also strips off the path since the game has everything
** on one place
*/
TextureClass * create_projector_texture_from_filename( const char * filename )
{
	StringClass	tex_name = filename;
	if (::strchr(tex_name, '\\') != 0) {
		tex_name = ::strrchr(filename, '\\') + 1;
	}

	TextureClass * texture = WW3DAssetManager::Get_Instance()->Get_Texture(tex_name,TextureClass::MIP_LEVELS_1);
	if (texture == NULL) {
		WWDEBUG_SAY(("Failed to create %s from %s\n", (const char *)tex_name, filename));
	}
	return texture;
}


/********************************************************************************************
**
** ProjectorManagerClass Implementation
**
********************************************************************************************/

ProjectorManagerClass::ProjectorManagerClass(void) :
	Projector(NULL),
	ProjectorBoneIndex(0)
{
}

ProjectorManagerClass::~ProjectorManagerClass(void)
{
	Free();
}

void ProjectorManagerClass::Free(void)
{
	if (Projector) {
		PhysicsSceneClass::Get_Instance()->Remove_Texture_Projector(Projector);
		Projector->Release_Ref();
		Projector = NULL;
	}
	ProjectorBoneIndex = 0;
}

void ProjectorManagerClass::Init(const ProjectorManagerDefClass & def,RenderObjClass * model)
{
	Free();
	WWASSERT(Projector == NULL);

	if (model == NULL) {
		return;
	}

	if (def.IsEnabled) {

		/*
		** Create the projector
		*/
		Projector = NEW_REF(PhysTexProjectClass, ());
		Projector->Peek_Material_Pass()->Enable_On_Translucent_Meshes(false);

		if (Projector != NULL) {
			/*
			** Set up projection parameters
			*/
			if (def.IsPerspective) {
				Projector->Set_Perspective_Projection(	def.HorizontalFOV,
																	def.VerticalFOV,
																	def.NearZ,
																	def.FarZ);
			} else {
				Projector->Set_Ortho_Projection(			-def.OrthoWidth * 0.5f,def.OrthoWidth * 0.5f,
																	-def.OrthoHeight * 0.5f,def.OrthoHeight * 0.5f,
																	def.NearZ,def.FarZ);
			}

			/*
			** Set up the blending
			*/
			if (def.IsAdditive) {
				Projector->Init_Additive();
			} else {
				Projector->Init_Multiplicative();
			}
			Projector->Set_Intensity(def.Intensity);

			/*
			** Install the texture
			*/
			TextureClass * tex = create_projector_texture_from_filename(def.TextureName);

			if (tex != NULL) {
				tex->Set_U_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);
				tex->Set_V_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);
				Projector->Set_Texture(tex);
				tex->Release_Ref();
			} else {
				Projector->Release_Ref();
				Projector = NULL;
			}

		}

		/*
		** If the projector is animated, resolve the bone name into a bone index and install it into the scene
		** If it is not animated, add it as a static projector (for more efficient culling)
		*/
		if (Projector != NULL) {

			/*
			** Find the bone
			*/
			ProjectorBoneIndex = model->Get_Bone_Index(def.BoneName);
			if (ProjectorBoneIndex == 0xFFFF) {
				ProjectorBoneIndex = 0;
			}
			Projector->Set_Transform(model->Get_Bone_Transform(ProjectorBoneIndex));

			Set_Flag(IS_ANIMATED,(def.IsAnimated) && (ProjectorBoneIndex != 0));

			if (Get_Flag(IS_ANIMATED)) {
				PhysicsSceneClass::Get_Instance()->Add_Dynamic_Texture_Projector(Projector);
			} else {
				PhysicsSceneClass::Get_Instance()->Add_Static_Texture_Projector(Projector);
			}
		}
	}
}

void ProjectorManagerClass::Update_From_Model(RenderObjClass * model)
{
	if ((Projector != NULL) && (ProjectorBoneIndex != -1)) {
		const Matrix3D & tm = model->Get_Bone_Transform(ProjectorBoneIndex);
		Projector->Set_Transform(tm);
	}
}


/********************************************************************************************
**
** ProjectorManagerDefClass Implementation
** Note, ProjectorManagerDef is not a full-fleged definition class.  It is meant to be
** embedded inside another real definition.
**
********************************************************************************************/

enum
{
	PROJECTORMANAGERDEF_CHUNK_VARIABLES							= 0x01110004,

	PROJECTORMANAGERDEF_VARIABLE_ISENABLED						= 0x00,
	PROJECTORMANAGERDEF_VARIABLE_ISPERSPECTIVE,
	PROJECTORMANAGERDEF_VARIABLE_ISADDITIVE,
	PROJECTORMANAGERDEF_VARIABLE_ISANIMATED,
	PROJECTORMANAGERDEF_VARIABLE_ORTHOWIDTH,
	PROJECTORMANAGERDEF_VARIABLE_ORTHOHEIGHT,
	PROJECTORMANAGERDEF_VARIABLE_HORIZONTALFOV,
	PROJECTORMANAGERDEF_VARIABLE_VERTICALFOV,
	PROJECTORMANAGERDEF_VARIABLE_NEARZ,
	PROJECTORMANAGERDEF_VARIABLE_FARZ,
	PROJECTORMANAGERDEF_VARIABLE_TEXTURENAME,
	PROJECTORMANAGERDEF_VARIABLE_BONENAME,

	PROJECTORMANAGERDEF_VARIABLE_INTENSITY,
};


ProjectorManagerDefClass::ProjectorManagerDefClass(void) :
	IsEnabled(false),
	IsPerspective(false),
	IsAdditive(false),
	IsAnimated(false),
	OrthoWidth(10.0f),
	OrthoHeight(10.0f),
	HorizontalFOV(DEG_TO_RADF(10.0f)),
	VerticalFOV(DEG_TO_RADF(10.0f)),
	NearZ(5.0f),
	FarZ(20.0f),
	Intensity(1.0f)
{
}

ProjectorManagerDefClass::~ProjectorManagerDefClass(void)
{
}

void ProjectorManagerDefClass::Validate_Parameters(void)
{
	if (HorizontalFOV <= 0.0f) { HorizontalFOV = DEG_TO_RADF(10.0f); }
	if (VerticalFOV <= 0.0f) { VerticalFOV = DEG_TO_RADF(10.0f); }
	if (OrthoWidth <= 0.0f) { OrthoWidth = 10.0f; }
	if (OrthoHeight <= 0.0f) { OrthoHeight = 10.0f; }
	if (NearZ < 0.0f) { NearZ = 0.0f; }
	if (FarZ < NearZ) { FarZ = NearZ + 10.0f; }
}

bool ProjectorManagerDefClass::Save(ChunkSaveClass &csave)
{
	Validate_Parameters();

	csave.Begin_Chunk(PROJECTORMANAGERDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,PROJECTORMANAGERDEF_VARIABLE_ISENABLED,IsEnabled);
	WRITE_MICRO_CHUNK(csave,PROJECTORMANAGERDEF_VARIABLE_ISPERSPECTIVE,IsPerspective);
	WRITE_MICRO_CHUNK(csave,PROJECTORMANAGERDEF_VARIABLE_ISADDITIVE,IsAdditive);
	WRITE_MICRO_CHUNK(csave,PROJECTORMANAGERDEF_VARIABLE_ISANIMATED,IsAnimated);
	WRITE_MICRO_CHUNK(csave,PROJECTORMANAGERDEF_VARIABLE_ORTHOWIDTH,OrthoWidth);
	WRITE_MICRO_CHUNK(csave,PROJECTORMANAGERDEF_VARIABLE_ORTHOHEIGHT,OrthoHeight);
	WRITE_MICRO_CHUNK(csave,PROJECTORMANAGERDEF_VARIABLE_HORIZONTALFOV,HorizontalFOV);
	WRITE_MICRO_CHUNK(csave,PROJECTORMANAGERDEF_VARIABLE_VERTICALFOV,VerticalFOV);
	WRITE_MICRO_CHUNK(csave,PROJECTORMANAGERDEF_VARIABLE_NEARZ,NearZ);
	WRITE_MICRO_CHUNK(csave,PROJECTORMANAGERDEF_VARIABLE_FARZ,FarZ);
	WRITE_MICRO_CHUNK_WWSTRING(csave,PROJECTORMANAGERDEF_VARIABLE_TEXTURENAME,TextureName);
	WRITE_MICRO_CHUNK_WWSTRING(csave,PROJECTORMANAGERDEF_VARIABLE_BONENAME,BoneName);
	WRITE_MICRO_CHUNK(csave,PROJECTORMANAGERDEF_VARIABLE_INTENSITY,Intensity);
	csave.End_Chunk();

	return true;
}

bool ProjectorManagerDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case PROJECTORMANAGERDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,PROJECTORMANAGERDEF_VARIABLE_ISENABLED,IsEnabled);
						READ_MICRO_CHUNK(cload,PROJECTORMANAGERDEF_VARIABLE_ISPERSPECTIVE,IsPerspective);
						READ_MICRO_CHUNK(cload,PROJECTORMANAGERDEF_VARIABLE_ISADDITIVE,IsAdditive);
						READ_MICRO_CHUNK(cload,PROJECTORMANAGERDEF_VARIABLE_ISANIMATED,IsAnimated);
						READ_MICRO_CHUNK(cload,PROJECTORMANAGERDEF_VARIABLE_ORTHOWIDTH,OrthoWidth);
						READ_MICRO_CHUNK(cload,PROJECTORMANAGERDEF_VARIABLE_ORTHOHEIGHT,OrthoHeight);
						READ_MICRO_CHUNK(cload,PROJECTORMANAGERDEF_VARIABLE_HORIZONTALFOV,HorizontalFOV);
						READ_MICRO_CHUNK(cload,PROJECTORMANAGERDEF_VARIABLE_VERTICALFOV,VerticalFOV);
						READ_MICRO_CHUNK(cload,PROJECTORMANAGERDEF_VARIABLE_NEARZ,NearZ);
						READ_MICRO_CHUNK(cload,PROJECTORMANAGERDEF_VARIABLE_FARZ,FarZ);
						READ_MICRO_CHUNK_WWSTRING(cload,PROJECTORMANAGERDEF_VARIABLE_TEXTURENAME,TextureName);
						READ_MICRO_CHUNK_WWSTRING(cload,PROJECTORMANAGERDEF_VARIABLE_BONENAME,BoneName);
						READ_MICRO_CHUNK(cload,PROJECTORMANAGERDEF_VARIABLE_INTENSITY,Intensity);
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}

		cload.Close_Chunk();
	}
	return true;
}
