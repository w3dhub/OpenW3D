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
 *                     $Archive:: /Commando/Code/wwphys/transitioneffect.cpp                  $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/07/01 3:39p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "transitioneffect.h"
#include "matpass.h"
#include "vertmaterial.h"
#include "physresourcemgr.h"
#include "phys.h"
#include "rinfo.h"
#include "camera.h"
#include "chunkio.h"
#include "texture.h"


/*************************************************************************************************
**
** TransitionEffectClass Implementation
**
*************************************************************************************************/

static float STEALTH_FRACTION_RATE			= 0.5f;		// rate at which the fraction approaches its target 
static float STEALTH_ZNEAR_FADE_DISTANCE	= 15.0f;		// distance at which the intensity starts to fade due to camera proximity
static float STEALTH_ZNEAR_AMOUNT			= 0.25f;		// amount the intensity can change as it approaches the camera
static float STEALTH_FRIENDLY_FRACTION		= 0.75f;		// fraction at which friendly stealthed objects appear
static float STEALTH_BROKEN_FRACTION		= 0.25f;		// fraction at which broken stealthed objects appear
static float STEALTH_DAMAGED_FRACTION		= 0.6f;		// fraction that is jumped to when an object gets damaged

static Vector2 MAX_STEALTH_UV_RATE(2.75f,-3.0f);
static Vector2 MIN_STEALTH_UV_RATE(0.5f,-0.5f);


TransitionEffectClass::TransitionEffectClass(void) :
	CurrentParameter(0.0f),
	TargetParameter(1.0f),
	ParameterVelocity(1.0f),
	MinUVRate(0.5f,-0.5f),
	MaxUVRate(2.75f,-3.0f),
	UVRate(0.0f,0.0f),
	StartDelay(0.0f),
	RemoveOnComplete(false),
	RenderBaseMaterial(false),
	RenderTransitionMaterial(true),
	IntensityScale(1.0f),
	UVOffset(0.0f,0.0f),
	MaxIntensity(1.0f),
	Mapper(NULL),
	MaterialPass(NULL)
{
	MaterialPass = NEW_REF(MaterialPassClass,());
	MaterialPass->Enable_On_Translucent_Meshes(false);
	
	VertexMaterialClass * vmtl = NEW_REF(VertexMaterialClass,()); //PhysResourceMgrClass::Get_Stealth_Material();
	vmtl->Set_Ambient(0,0,0);
	vmtl->Set_Diffuse(0,0,0);
	vmtl->Set_Specular(0,0,0);
	vmtl->Set_Emissive(1.0f,1.0f,1.0f);
	vmtl->Set_Opacity(1.0f);
	vmtl->Set_Shininess(0.0f);
	vmtl->Set_Lighting(true);

	Mapper = NEW_REF(MatrixMapperClass,(0));
	Mapper->Set_Type(MatrixMapperClass::ORTHO_PROJECTION);
	Mapper->Set_Texture_Transform(Matrix4(1),64.0f);
	vmtl->Set_Mapper(Mapper,0);

	MaterialPass->Set_Material(vmtl);
	REF_PTR_RELEASE(vmtl);
	
	ShaderClass shader = ShaderClass::_PresetAdditiveShader;
	shader.Set_Primary_Gradient(ShaderClass::GRADIENT_ADD);
	MaterialPass->Set_Shader(ShaderClass::_PresetAdditiveShader);
}

TransitionEffectClass::~TransitionEffectClass(void)
{
	REF_PTR_RELEASE(MaterialPass);
	REF_PTR_RELEASE(Mapper);
}


void TransitionEffectClass::Timestep(float dt)
{
	/*
	** Update the parameter
	*/
	if (StartDelay > 0.0f) {
		StartDelay -= dt;
	} else {
		float step = ParameterVelocity * dt;
		if (step > WWMath::Fabs(TargetParameter - CurrentParameter)) {
			CurrentParameter = TargetParameter;
			if (RemoveOnComplete) {
				Enable_Auto_Remove(true);
			}
		} else {
			if (TargetParameter < CurrentParameter) {
				CurrentParameter -= step;
			} else {
				CurrentParameter += step;
			}
		}
	}

	/*
	** The Intensity is computed as a function of the stealth fraction
	*/
	IntensityScale = 1.0f - 2.0f * WWMath::Fabs(CurrentParameter - 0.5f);
	IntensityScale *= MaxIntensity;

	/*
	** Update the uv cycling animation.  I'm deriving the UV animation rate
	** from the intensity
	*/
	Vector2 uv_rate = MinUVRate + IntensityScale * IntensityScale * (MaxUVRate - MinUVRate);
	UVOffset += uv_rate * dt;
	UVOffset.X = fmod(UVOffset.X , 2.0f);
	UVOffset.Y = fmod(UVOffset.Y , 2.0f);

	/*
	** The base pass gets rendered when the stealh fraction is < 0.5
	** The material pass gets rendered if the intensity is greater than 0.0
	*/
	RenderBaseMaterial = (CurrentParameter < 0.5f);
	RenderTransitionMaterial = (IntensityScale > 0.0f);
}

void TransitionEffectClass::Render_Push(RenderInfoClass & rinfo,PhysClass * /* obj */)
{
	if (RenderTransitionMaterial) {
		
		/*
		** Update the material settings and texture transform
		*/
		MaterialPass->Peek_Material()->Set_Emissive(Vector3(1,1,1) * IntensityScale);
		
		Matrix4 tm(1);
		tm[0][3] = UVOffset.X;
		tm[1][3] = UVOffset.Y;
		Mapper->Set_Texture_Transform(tm,64.0f);

		/*
		** Add the material pass!
		*/
		rinfo.Push_Material_Pass(MaterialPass);
	}
	
	if (RenderBaseMaterial == false) {
		rinfo.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);
	}
}

void TransitionEffectClass::Render_Pop(RenderInfoClass & rinfo)
{
	if (RenderBaseMaterial == false) {
		rinfo.Pop_Override_Flags();
	}
	if (RenderTransitionMaterial) {
		rinfo.Pop_Material_Pass();
	}
}


void TransitionEffectClass::Set_Texture(TextureClass * texture)
{
	if (MaterialPass != NULL) {
		MaterialPass->Set_Texture(texture,0);
	}
}
	

/************************************************************************************
**
** Save-Load support
**
************************************************************************************/

enum 
{
	TRANSITIONEFFECT_CHUNK_VARIABLES	= 702011215,			// member variables.

	TRANSITIONEFFECT_VARIABLE_CURRENTPARAMETER = 0,
	TRANSITIONEFFECT_VARIABLE_TARGETPARAMETER,
	TRANSITIONEFFECT_VARIABLE_PARAMETERVELOCITY,
	TRANSITIONEFFECT_VARIABLE_MINUVRATE,
	TRANSITIONEFFECT_VARIABLE_MAXUVRATE,
	TRANSITIONEFFECT_VARIABLE_UVRATE,
	TRANSITIONEFFECT_VARIABLE_RENDERBASEMATERIAL,
	TRANSITIONEFFECT_VARIABLE_RENDERTRANSITIONMATERIAL,
	TRANSITIONEFFECT_VARIABLE_INTENSITYSCALE,
	TRANSITIONEFFECT_VARIABLE_UVOFFSET,
	TRANSITIONEFFECT_VARIABLE_TEXTURENAME,
};


bool TransitionEffectClass::Save(ChunkSaveClass & csave)
{
	const char * texname = NULL;
	TextureClass * tex = MaterialPass->Peek_Texture(0);
	if (tex != NULL) {
		texname = tex->Get_Texture_Name();
	}

	csave.Begin_Chunk(TRANSITIONEFFECT_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,TRANSITIONEFFECT_VARIABLE_CURRENTPARAMETER,CurrentParameter);
	WRITE_MICRO_CHUNK(csave,TRANSITIONEFFECT_VARIABLE_TARGETPARAMETER,TargetParameter);
	WRITE_MICRO_CHUNK(csave,TRANSITIONEFFECT_VARIABLE_PARAMETERVELOCITY,ParameterVelocity);
	WRITE_MICRO_CHUNK(csave,TRANSITIONEFFECT_VARIABLE_MINUVRATE,MinUVRate);
	WRITE_MICRO_CHUNK(csave,TRANSITIONEFFECT_VARIABLE_MAXUVRATE,MaxUVRate);
	WRITE_MICRO_CHUNK(csave,TRANSITIONEFFECT_VARIABLE_UVRATE,UVRate);
	WRITE_MICRO_CHUNK(csave,TRANSITIONEFFECT_VARIABLE_RENDERBASEMATERIAL,RenderBaseMaterial);
	WRITE_MICRO_CHUNK(csave,TRANSITIONEFFECT_VARIABLE_RENDERTRANSITIONMATERIAL,RenderBaseMaterial);
	WRITE_MICRO_CHUNK(csave,TRANSITIONEFFECT_VARIABLE_INTENSITYSCALE,IntensityScale);
	WRITE_MICRO_CHUNK(csave,TRANSITIONEFFECT_VARIABLE_UVOFFSET,UVOffset);
	
	if (texname != NULL) {
		WRITE_MICRO_CHUNK_STRING(csave,TRANSITIONEFFECT_VARIABLE_TEXTURENAME,texname);
	}

	csave.End_Chunk();

	return true;
}

bool TransitionEffectClass::Load(ChunkLoadClass & cload)
{
	StringClass texname;

	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {			

			case TRANSITIONEFFECT_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,TRANSITIONEFFECT_VARIABLE_CURRENTPARAMETER,CurrentParameter);
						READ_MICRO_CHUNK(cload,TRANSITIONEFFECT_VARIABLE_TARGETPARAMETER,TargetParameter);
						READ_MICRO_CHUNK(cload,TRANSITIONEFFECT_VARIABLE_PARAMETERVELOCITY,ParameterVelocity);
						READ_MICRO_CHUNK(cload,TRANSITIONEFFECT_VARIABLE_MINUVRATE,MinUVRate);
						READ_MICRO_CHUNK(cload,TRANSITIONEFFECT_VARIABLE_MAXUVRATE,MaxUVRate);
						READ_MICRO_CHUNK(cload,TRANSITIONEFFECT_VARIABLE_UVRATE,UVRate);
						READ_MICRO_CHUNK(cload,TRANSITIONEFFECT_VARIABLE_RENDERBASEMATERIAL,RenderBaseMaterial);
						READ_MICRO_CHUNK(cload,TRANSITIONEFFECT_VARIABLE_RENDERTRANSITIONMATERIAL,RenderBaseMaterial);
						READ_MICRO_CHUNK(cload,TRANSITIONEFFECT_VARIABLE_INTENSITYSCALE,IntensityScale);
						READ_MICRO_CHUNK(cload,TRANSITIONEFFECT_VARIABLE_UVOFFSET,UVOffset);
						READ_MICRO_CHUNK_WWSTRING(cload,TRANSITIONEFFECT_VARIABLE_TEXTURENAME,texname);
					}
					cload.Close_Micro_Chunk();
				}
				break;
		}

		cload.Close_Chunk();
	}
	return true;
}

