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
 *                 Project Name : Combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/humanrecoil.cpp                       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/12/00 5:24p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "humanrecoil.h"
#include "vector3.h"
#include "wwdebug.h"
#include "rendobj.h"
#include "htree.h"
#include <string.h>


/*
** Static members of HumanRecoilClass
*/
bool HumanRecoilClass::IsInitted = false;


/*
** Recoil Data
*/
struct RecoilDataStruct
{
	RecoilDataStruct(const char * name,const Vector3 & translation) :
		BoneName(name),
		BoneIndex(0),
		Translation(translation)
	{}

	const char *	BoneName;
	int				BoneIndex;
	Vector3			Translation;
};

static RecoilDataStruct _RecoilData[] = 
{
	RecoilDataStruct("C L CLAVICLE",Vector3(-0.025f,0.0f,0.0f)),
	RecoilDataStruct("C L UPPERARM",Vector3(-0.025f,0.0f,0.0f)),
	RecoilDataStruct("C L FOREARM",Vector3(-0.05f,0.0f,0.0f)),
	RecoilDataStruct("C L HAND",Vector3(-0.05f,0.0f,0.0f)),
	RecoilDataStruct("C R CLAVICLE",Vector3(-0.05f,0.0f,0.0f)),
	RecoilDataStruct("C R UPPERARM",Vector3(-0.05f,0.0f,0.0f)),
	RecoilDataStruct("C R FOREARM",Vector3(-0.1f,0.0f,0.0f)),
	RecoilDataStruct("C R HAND",Vector3(-0.1f,0.0f,0.0f)),
	RecoilDataStruct("BACKGUNBONE",Vector3(-0.075f,0.0f,0.0f)),
};

const int _RECOIL_BONE_COUNT = sizeof(_RecoilData) / sizeof(RecoilDataStruct);


/***********************************************************************************************
**
** HumanRecoilClass Implementation
**
***********************************************************************************************/
HumanRecoilClass::HumanRecoilClass(void)
{
}

void HumanRecoilClass::Capture_Bones(RenderObjClass * model)
{
	WWASSERT(model != NULL);
	if (IsInitted == false) {
		Initialize(model);
	}
	for (int recoil_bone = 0; recoil_bone < _RECOIL_BONE_COUNT; recoil_bone++) {
		model->Capture_Bone(_RecoilData[recoil_bone].BoneIndex);
	}
}

void HumanRecoilClass::Apply_Recoil(const Matrix3D & recoil_tm,RenderObjClass * model,float scale)
{
	WWASSERT(model != NULL);
	if (IsInitted == false) {
		Initialize(model);
	}

	Vector3 recoil_offset;
	for (int recoil_bone = 0; recoil_bone < _RECOIL_BONE_COUNT; recoil_bone++) {
	
		// for each bone to recoil, compute the translation and plug it in
		Matrix3D::Transform_Vector(recoil_tm,scale * _RecoilData[recoil_bone].Translation,&recoil_offset);
		Matrix3D tm(recoil_offset);
		model->Control_Bone(_RecoilData[recoil_bone].BoneIndex,tm,true);
	}
}

void HumanRecoilClass::Release_Bones(RenderObjClass * model)
{
	WWASSERT(model != NULL);
	if (IsInitted == false) {
		Initialize(model);
	}
	for (int recoil_bone = 0; recoil_bone < _RECOIL_BONE_COUNT; recoil_bone++) {
		model->Release_Bone(_RecoilData[recoil_bone].BoneIndex);
	}
}

void HumanRecoilClass::Initialize(RenderObjClass * model)
{
	const HTreeClass * tree = model->Get_HTree();
	for (int recoil_bone = 0; recoil_bone < _RECOIL_BONE_COUNT; recoil_bone++) {
		
		for (int model_bone = 0; model_bone < tree->Num_Pivots(); model_bone++) {
			
			if (stricmp(_RecoilData[recoil_bone].BoneName, tree->Get_Bone_Name(model_bone)) == 0) {
				_RecoilData[recoil_bone].BoneIndex = model_bone;
				model_bone = tree->Num_Pivots();
			}
		}
	}
	IsInitted = true;
}


