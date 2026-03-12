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
 *                     $Archive:: /Commando/Code/wwphys/renderobjphys.cpp                     $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/27/00 1:54p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "renderobjphys.h"
#include "rendobj.h"
#include "persistfactory.h"
#include "wwphysids.h"


/*
** Declare a PersistFactory for RenderObjPhysClasses
*/
SimplePersistFactoryClass<RenderObjPhysClass,PHYSICS_CHUNKID_RENDEROBJPHYS>	_RenderObjPhysFactory;

/*
** Chunk ID's used by RenderObjPhysClass
*/
enum
{
	RENDEROBJPHYS_CHUNK_PHYS				= 0x00099900,	// obsolete, old parent class
	RENDEROBJPHYS_CHUNK_DYNAMICPHYS
};

/**
** RenderObjPhysClass
** NOTE: remember that this class is mainly intended just as a simple wrapper around render
** objects that are added directly to the physics scene.  Normally, only the physics scene
** itself will be creating and using these.  If you find yourself creating these objects,
** consider using DecorationPhysClass instead...
*/
RenderObjPhysClass::RenderObjPhysClass(void)
{
	Enable_Is_Pre_Lit(true);

	if (Model) {
		Model->Set_User_Data(this);
	}
}

RenderObjPhysClass::~RenderObjPhysClass(void)
{
	if (Model) {
		Model->Set_User_Data(0);
	}
}

void RenderObjPhysClass::Set_Model(RenderObjClass * model)
{
	if (Model) {
		Model->Set_User_Data(0);
	}

	// Let the base class have the model
	DynamicPhysClass::Set_Model(model);

	if (Model) {
		Model->Set_User_Data(this);

		if (Model->Class_ID() == RenderObjClass::CLASSID_HLOD) {
			Enable_Is_Pre_Lit(false);
		} else {
			Enable_Is_Pre_Lit(true);
		}

	}
}

const AABoxClass & RenderObjPhysClass::Get_Bounding_Box(void) const
{
	assert(Model);
	return Model->Get_Bounding_Box();
}

const Matrix3D & RenderObjPhysClass::Get_Transform(void) const
{
	assert(Model);
	return Model->Get_Transform();
}

void RenderObjPhysClass::Set_Transform(const Matrix3D & m)
{
	// Note: this kind of object never causes collisions so we
	// can just warp it to the users desired position.  However,
	// we do need to tell the scene that we moved so that
	// it can update us in the culling system
	assert(Model);
	Model->Set_Transform(m);
	Update_Cull_Box();
}

bool RenderObjPhysClass::Cast_Ray(PhysRayCollisionTestClass & raytest)
{
	assert(Model);
	if (Model->Cast_Ray(raytest)) {
		raytest.CollidedPhysObj = this;
		return true;
	}
	return false;
}

bool RenderObjPhysClass::Cast_AABox(PhysAABoxCollisionTestClass & boxtest)
{
	assert(Model);
	if (Model->Cast_AABox(boxtest)) {
		boxtest.CollidedPhysObj = this;
		return true;
	}
	return false;
}

bool RenderObjPhysClass::Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest)
{
	assert(Model);
	if (Model->Cast_OBBox(boxtest)) {
		boxtest.CollidedPhysObj = this;
		return true;
	}
	return false;
}

/*
** Save-Load System
*/
const PersistFactoryClass & RenderObjPhysClass::Get_Factory (void) const
{
	return _RenderObjPhysFactory;
}

bool RenderObjPhysClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(RENDEROBJPHYS_CHUNK_DYNAMICPHYS);
	DynamicPhysClass::Save(csave);
	csave.End_Chunk();

	return true;
}

bool RenderObjPhysClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID())
		{
			case RENDEROBJPHYS_CHUNK_PHYS:
				PhysClass::Load(cload);		// note, legacy loading here... file must be old.
				break;

			case RENDEROBJPHYS_CHUNK_DYNAMICPHYS:
				DynamicPhysClass::Load(cload);
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}

		cload.Close_Chunk();
	}
	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}

void RenderObjPhysClass::On_Post_Load (void)
{
	DynamicPhysClass::On_Post_Load();
	if (Model) {
		Model->Set_User_Data((void*)this);
	}
}


