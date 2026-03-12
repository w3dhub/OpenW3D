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
 *                     $Archive:: /Commando/Code/Combat/muzzlerecoil.cpp                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/22/00 8:31a                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "muzzlerecoil.h"
#include "timemgr.h"
#include "rendobj.h"
#include "wwdebug.h"


MuzzleRecoilClass::MuzzleRecoilClass(void)
{
	Init(0);
}

void MuzzleRecoilClass::Init(int bone_index)
{
	BoneIndex = bone_index;
	RecoilScale = 1.0f;
	RecoilTimer = 0.0f;
	OORecoilTime = 0.0f;
}

void MuzzleRecoilClass::Start_Recoil(float recoil_scale,float recoil_time)
{
	RecoilScale = recoil_scale;
	RecoilTimer = recoil_time;
	if (RecoilTimer > 0.0f) {
		OORecoilTime = 1.0f / RecoilTimer;
	}
}

void MuzzleRecoilClass::Update(RenderObjClass * model)
{
	WWASSERT(model != NULL);
	if ((RecoilTimer <= 0.0f) || (BoneIndex <= 0)) {
		return;
	}

	// Since our timer is active, go ahead and capture the bone
	model->Capture_Bone(BoneIndex);

	// Apply the recoil effect.
	float recoil_scale = RecoilScale * RecoilTimer * OORecoilTime;
	Matrix3D recoil_tm(1);
	recoil_tm.Translate_X(-recoil_scale);
	model->Control_Bone(BoneIndex,recoil_tm);

	// Decrement the recoil timer and release the bone if it expires
	RecoilTimer -= TimeManager::Get_Frame_Seconds();
	if (RecoilTimer <= 0.0f) {
		RecoilTimer = 0.0f;
		model->Release_Bone(BoneIndex);
	}
}

