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
 *                     $Archive:: /Commando/Code/wwphys/stealtheffect.cpp                     $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/20/01 6:32p                                              $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "stealtheffect.h"
#include "matpass.h"
#include "vertmaterial.h"
#include "physresourcemgr.h"
#include "phys.h"
#include "rinfo.h"
#include "camera.h"
#include "chunkio.h"


/*************************************************************************************************
**
** StealthEffectClass Implementation
**
*************************************************************************************************/

static float STEALTH_FRACTION_RATE			= 0.5f;		// rate at which the fraction approaches its target
static float STEALTH_DEFAULT_FADE_DISTANCE= 25.0f;		// distance at which the intensity starts to fade due to camera proximity
static float STEALTH_ZNEAR_AMOUNT			= 0.40f;		// amount the intensity can change as it approaches the camera
static float STEALTH_FRIENDLY_FRACTION		= 0.75f;		// fraction at which friendly stealthed objects appear
static float STEALTH_BROKEN_FRACTION		= 0.25f;		// fraction at which broken stealthed objects appear
static float STEALTH_DAMAGED_FRACTION		= 0.6f;		// fraction that is jumped to when an object gets damaged

static Vector2 MAX_STEALTH_UV_RATE(2.75f,-3.0f);
static Vector2 MIN_STEALTH_UV_RATE(0.5f,-0.5f);


StealthEffectClass::StealthEffectClass(void) :
	IsStealthEnabled(false),
	IsFriendly(false),
	IsBroken(false),
	FadeDistance(STEALTH_DEFAULT_FADE_DISTANCE),
	CurrentFraction(0.0f),
	TargetFraction(1.0f),
	UVRate(0.5f,0.5f),
	RenderBaseMaterial(true),
	RenderStealthMaterial(true),
	IntensityScale(0.0f),
	UVOffset(0.0f,0.0f),
	Mapper(NULL),
	MaterialPass(NULL)
{
	MaterialPass = NEW_REF(MaterialPassClass,());

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

	MaterialPass->Set_Texture(PhysResourceMgrClass::Peek_Stealth_Texture());
	MaterialPass->Enable_On_Translucent_Meshes( false );
}

StealthEffectClass::~StealthEffectClass(void)
{
	REF_PTR_RELEASE(MaterialPass);
	REF_PTR_RELEASE(Mapper);
}

void StealthEffectClass::Enable_Stealth(bool onoff)
{
	IsStealthEnabled = onoff;
	Update_Target_Fraction();
}

void StealthEffectClass::Set_Friendly(bool onoff)
{
	IsFriendly = onoff;
	Update_Target_Fraction();
}

void StealthEffectClass::Set_Broken(bool onoff)
{
	IsBroken = onoff;
	Update_Target_Fraction();
}

void StealthEffectClass::Damage_Occured(void)
{
	if (IsStealthEnabled && (CurrentFraction > STEALTH_DAMAGED_FRACTION)) {
		CurrentFraction = STEALTH_DAMAGED_FRACTION;
	}
}

void StealthEffectClass::Update_Target_Fraction(void)
{
	if (IsStealthEnabled) {

		if (IsBroken) {
			TargetFraction = STEALTH_BROKEN_FRACTION;
		} else if (IsFriendly) {
			TargetFraction = STEALTH_FRIENDLY_FRACTION;
		} else {
			TargetFraction = 1.0f;
		}

	} else {
		TargetFraction = 0.0f;
	}
}

void StealthEffectClass::Timestep(float dt)
{
	/*
	** Update the stealth fraction
	*/
	float step = STEALTH_FRACTION_RATE * dt;
	if (step > WWMath::Fabs(TargetFraction - CurrentFraction)) {
		CurrentFraction = TargetFraction;
	} else {
		if (TargetFraction < CurrentFraction) {
			CurrentFraction -= step;
		} else {
			CurrentFraction += step;
		}
	}

	/*
	** The Intensity is computed as a function of the stealth fraction
	*/
	IntensityScale = 1.0f - 2.0f * WWMath::Fabs(CurrentFraction - 0.5f);

	/*
	** Update the uv cycling animation.  I'm deriving the UV animation rate
	** from the intensity
	*/
	Vector2 uv_rate = MIN_STEALTH_UV_RATE + IntensityScale * IntensityScale * (MAX_STEALTH_UV_RATE - MIN_STEALTH_UV_RATE);
		UVOffset += uv_rate * dt;
	UVOffset.X = fmod(UVOffset.X , 2.0f);
	UVOffset.Y = fmod(UVOffset.Y , 2.0f);

	/*
	** The base pass gets rendered when the stealh fraction is < 0.5
	** shadows are also suppressed.
	*/
	RenderBaseMaterial = (CurrentFraction < 0.5f);
	Enable_Suppress_Shadows(CurrentFraction > 0.5f);
}

void StealthEffectClass::Render_Push(RenderInfoClass & rinfo,PhysClass * obj)
{
	/*
	** If we are in the (0.5,1.0) range of the fraction (object is hidden) then
	** the distance from the camera to the object can affect the intensity.
	*/
	Vector3 obj_pos;
	obj->Get_Position(&obj_pos);
	float xydist = (rinfo.Camera.Get_Position() - obj_pos).Quick_Length();
	if (xydist <= FadeDistance) {
		float intensity_delta = STEALTH_ZNEAR_AMOUNT * (FadeDistance - xydist) / FadeDistance;
		float intensity = WWMath::Min(IntensityScale + intensity_delta, 1.0f);
		IntensityScale = intensity;
	}

	/*
	** The stealth material gets rendered if the intensity is greater than 0.0
	*/
	RenderStealthMaterial = (IntensityScale > 0.0f);

	if (RenderStealthMaterial) {

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

void StealthEffectClass::Render_Pop(RenderInfoClass & rinfo)
{
	if (RenderBaseMaterial == false) {
		rinfo.Pop_Override_Flags();
	}
	if (RenderStealthMaterial) {
		rinfo.Pop_Material_Pass();
	}
}

/************************************************************************************
**
** Save-Load support
**
************************************************************************************/

enum
{
	STEALTHEFFECT_CHUNK_VARIABLES	= 0x055ffe07,			// member variables.

	STEALTHEFFECT_VARIABLE_ISSTEALTHENABLED		= 0x00,
	STEALTHEFFECT_VARIABLE_ISFRIENDLY,
	STEALTHEFFECT_VARIABLE_ISBROKEN,
	STEALTHEFFECT_VARIABLE_CURRENTFRACTION,
	STEALTHEFFECT_VARIABLE_TARGETFRACTION,
	STEALTHEFFECT_VARIABLE_UVRATE,
	STEALTHEFFECT_VARIABLE_FADEDISTANCE,
};


bool StealthEffectClass::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(STEALTHEFFECT_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,STEALTHEFFECT_VARIABLE_ISSTEALTHENABLED,IsStealthEnabled);
	WRITE_MICRO_CHUNK(csave,STEALTHEFFECT_VARIABLE_ISFRIENDLY,IsFriendly);
	WRITE_MICRO_CHUNK(csave,STEALTHEFFECT_VARIABLE_ISBROKEN,IsBroken);
	WRITE_MICRO_CHUNK(csave,STEALTHEFFECT_VARIABLE_CURRENTFRACTION,CurrentFraction);
	WRITE_MICRO_CHUNK(csave,STEALTHEFFECT_VARIABLE_TARGETFRACTION,TargetFraction);
	WRITE_MICRO_CHUNK(csave,STEALTHEFFECT_VARIABLE_UVRATE,UVRate);
	WRITE_MICRO_CHUNK(csave,STEALTHEFFECT_VARIABLE_FADEDISTANCE,FadeDistance);
	csave.End_Chunk();
	return true;
}

bool StealthEffectClass::Load(ChunkLoadClass & cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case STEALTHEFFECT_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,STEALTHEFFECT_VARIABLE_ISSTEALTHENABLED,IsStealthEnabled);
						READ_MICRO_CHUNK(cload,STEALTHEFFECT_VARIABLE_ISFRIENDLY,IsFriendly);
						READ_MICRO_CHUNK(cload,STEALTHEFFECT_VARIABLE_ISBROKEN,IsBroken);
						READ_MICRO_CHUNK(cload,STEALTHEFFECT_VARIABLE_CURRENTFRACTION,CurrentFraction);
						READ_MICRO_CHUNK(cload,STEALTHEFFECT_VARIABLE_TARGETFRACTION,TargetFraction);
						READ_MICRO_CHUNK(cload,STEALTHEFFECT_VARIABLE_UVRATE,UVRate);
						READ_MICRO_CHUNK(cload,STEALTHEFFECT_VARIABLE_FADEDISTANCE,FadeDistance);
					}
					cload.Close_Micro_Chunk();
				}
				break;
		}

		cload.Close_Chunk();
	}
	return true;
}
