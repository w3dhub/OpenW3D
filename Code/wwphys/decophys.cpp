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
 *                     $Archive:: /Commando/Code/wwphys/decophys.cpp                          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/05/02 5:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "decophys.h"
#include "rendobj.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"
#include "wwphysids.h"
#include "wwhack.h"
#include "part_emt.h"
#include "physinttest.h"


DECLARE_FORCE_LINK(decophys);

/****************************************************************************************************
**
** DecorationPhysClass Implementation
**
****************************************************************************************************/

/*
** Persist factory for DecorationPhysClass
*/
SimplePersistFactoryClass<DecorationPhysClass,PHYSICS_CHUNKID_DECORATIONPHYS>	_DecoPhysFactory;


/*
** Chunk-ID's used by DecoPhys
*/
enum 
{
	DECOPHYS_CHUNK_PHYS				=	0x005060000,			// old parent class data
	DECOPHYS_CHUNK_DYNAMICPHYS,									// current parent class data
};

DecorationPhysClass::DecorationPhysClass(void)
{ 
	ObjSpaceWorldBox.Center.Set(0,0,0);
	ObjSpaceWorldBox.Extent.Set(1,1,1);
}

void DecorationPhysClass::Init(const DecorationPhysDefClass & def)
{
	DynamicPhysClass::Init(def);
}

void DecorationPhysClass::Set_Model(RenderObjClass * model)
{
	DynamicPhysClass::Set_Model(model);

	// Initialize our copy of the world box
	if (Model != NULL) {
	
		RenderObjClass * box = Model->Get_Sub_Object_By_Name("WORLDBOX");
		if (box) {
	
			// Get the box when the model has an identity transform
			Matrix3D old_transform = Model->Get_Transform();
			Model->Set_Transform(Matrix3D(1));
			ObjSpaceWorldBox = box->Get_Bounding_Box();
			Model->Set_Transform(old_transform);

			box->Release_Ref();

		} else {
			Model->Get_Obj_Space_Bounding_Box(ObjSpaceWorldBox);
		} 
	}
}

const AABoxClass & DecorationPhysClass::Get_Bounding_Box(void) const
{
	assert(Model);
	return Model->Get_Bounding_Box();
}

const Matrix3D & DecorationPhysClass::Get_Transform(void) const
{
	assert(Model);
	return Model->Get_Transform();
}

void DecorationPhysClass::Set_Transform(const Matrix3D & m)
{
	// Note: this kind of object never causes collisions so we
	// can just warp it to the users desired position.  However,
	// we do need to tell the scene that we moved so that
	// it can update us in the culling system
	WWASSERT(Model);
	Model->Set_Transform(m);
	Update_Cull_Box();
	Update_Visibility_Status();
}

void DecorationPhysClass::Get_Shadow_Blob_Box(AABoxClass * set_obj_space_box)
{
	if (set_obj_space_box != NULL) {
		*set_obj_space_box = ObjSpaceWorldBox;
	}
}

bool DecorationPhysClass::Intersection_Test(PhysAABoxIntersectionTestClass & test)		
{ 
	WWASSERT(Model);
	if (Model->Intersect_AABox(test)) {
		test.Add_Intersected_Object(this);
		return true;
	}
	return false; 
}

bool DecorationPhysClass::Intersection_Test(PhysOBBoxIntersectionTestClass & test)		
{ 
	WWASSERT(Model);
	if (Model->Intersect_OBBox(test)) {
		test.Add_Intersected_Object(this);
		return true;
	}
	return false; 
}

bool DecorationPhysClass::Cast_Ray(PhysRayCollisionTestClass & raytest)
{
	WWASSERT(Model);
	if (Model->Cast_Ray(raytest)) {
		raytest.CollidedPhysObj = this;
		return true;
	}
	return false;
}

bool DecorationPhysClass::Cast_AABox(PhysAABoxCollisionTestClass & boxtest)
{
	WWASSERT(Model);
	if (Model->Cast_AABox(boxtest)) {
		boxtest.CollidedPhysObj = this;
		return true;
	}
	return false;
}

bool DecorationPhysClass::Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest)
{
	WWASSERT(Model);
	if (Model->Cast_OBBox(boxtest)) {
		boxtest.CollidedPhysObj = this;
		return true;
	}
	return false;
}


const PersistFactoryClass & DecorationPhysClass::Get_Factory (void) const
{
	return _DecoPhysFactory;
}

bool DecorationPhysClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(DECOPHYS_CHUNK_DYNAMICPHYS);
	DynamicPhysClass::Save(csave);
	csave.End_Chunk();

	return true;
}

bool DecorationPhysClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case DECOPHYS_CHUNK_PHYS:
				PhysClass::Load(cload);
				break;

			case DECOPHYS_CHUNK_DYNAMICPHYS:
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

void DecorationPhysClass::On_Post_Load(void)
{
	DynamicPhysClass::On_Post_Load();
	WWASSERT(Model);

	if (Model) {
		Set_Cull_Box(Model->Get_Bounding_Box());

		if (Model->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {
			Model->Set_User_Data (this);
			((ParticleEmitterClass *)Model)->Start ();
		}
	}
}

/****************************************************************************************************
**
** DecorationPhysDefClass Implementation
**
****************************************************************************************************/

/*
** Persist factory for DecorationPhysDefClass's
*/
SimplePersistFactoryClass<DecorationPhysDefClass,PHYSICS_CHUNKID_DECOPHYSDEF>	_DecorationPhysDefFactory;

/*
** Definition factory for DecorationPhysDefClass.  This makes it show up in the editor
*/
DECLARE_DEFINITION_FACTORY(DecorationPhysDefClass, CLASSID_DECOPHYSDEF, "DecorationPhys") _DecorationPhysDefDefFactory;

/*
** Chunk ID's used by DecorationPhysDefClass
*/
enum 
{
	DECORATIONPHYSDEF_CHUNK_PHYSDEF						= 0x01070003,			// old parent class
	DECORATIONPHYSDEF_CHUNK_DYNAMICPHYSDEF,										// current parent class
};


DecorationPhysDefClass::DecorationPhysDefClass(void)
{
}

uint32 DecorationPhysDefClass::Get_Class_ID (void) const
{
	return CLASSID_DECOPHYSDEF; 
}

PersistClass * DecorationPhysDefClass::Create(void) const
{
	DecorationPhysClass * new_obj = NEW_REF(DecorationPhysClass,());
	new_obj->Init(*this);
	return new_obj;
}

const char * DecorationPhysDefClass::Get_Type_Name(void)
{ 
	return "DecorationPhysDef"; 
}

bool DecorationPhysDefClass::Is_Type(const char * type_name)
{
	if (openw3d::stricmp(type_name,DecorationPhysDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return DynamicPhysDefClass::Is_Type(type_name);
	}
}

const PersistFactoryClass & DecorationPhysDefClass::Get_Factory (void) const
{
	return _DecorationPhysDefFactory;
}

bool DecorationPhysDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(DECORATIONPHYSDEF_CHUNK_DYNAMICPHYSDEF);
	DynamicPhysDefClass::Save(csave);
	csave.End_Chunk();
	
	return true;
}

bool DecorationPhysDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case DECORATIONPHYSDEF_CHUNK_PHYSDEF:
				PhysDefClass::Load(cload);
				break;

			case DECORATIONPHYSDEF_CHUNK_DYNAMICPHYSDEF:
				DynamicPhysDefClass::Load(cload);
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}

		cload.Close_Chunk();
	}
	return true;
}

