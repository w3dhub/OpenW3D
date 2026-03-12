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
 *                     $Archive:: /Commando/Code/wwphys/lightphys.cpp                         $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/01/02 3:32p                                               $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "lightphys.h"
#include "w3d_file.h"
#include "chunkio.h"
#include "light.h"
#include "matrix3.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "lightexclude.h"
#include "wwhack.h"
#include "vistable.h"


DECLARE_FORCE_LINK(lightphys);


/*
** Declare a PersistFactory for LightPhysClasses
*/
SimplePersistFactoryClass<LightPhysClass,PHYSICS_CHUNKID_LIGHTPHYS>	_LightPhysFactory;


/*
** Chunk ID's used by LightPhysClass
*/
enum
{
	LIGHTPHYS_CHUNK_DECOPHYS			= 0x00770010,
	LIGHTPHYS_CHUNK_VARIABLES,

	LIGHTPHYS_VARIABLE_VISSECTORID	= 0x00,
	LIGHTPHYS_VARIABLE_GROUPID,
};


LightPhysClass::LightPhysClass(bool auto_allocate_light) :
	VisSectorID(0xFFFFFFFF),
	GroupID(0)
{
	if (auto_allocate_light) {
		LightClass * new_light = NEW_REF(LightClass,());
		Set_Model(new_light);
		new_light->Release_Ref();
	}
}

void LightPhysClass::Set_Model(RenderObjClass * model)
{
	DecorationPhysClass::Set_Model(model);
}

int LightPhysClass::Is_Vis_Object_Visible(int vis_object_id)
{
	if (VisSectorID == 0xFFFFFFFF) {
		return 1;
	}
	VisTableClass * pvs = PhysicsSceneClass::Get_Instance()->Get_Vis_Table(VisSectorID);
	if (pvs != NULL) {
		int vis_bit = pvs->Get_Bit(vis_object_id);
		REF_PTR_RELEASE(pvs);
		return vis_bit;
	}

	return 1;
}


const PersistFactoryClass & LightPhysClass::Get_Factory (void) const
{
	return _LightPhysFactory;
}

bool LightPhysClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(LIGHTPHYS_CHUNK_DECOPHYS);
	DecorationPhysClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(LIGHTPHYS_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,LIGHTPHYS_VARIABLE_VISSECTORID,VisSectorID);
	WRITE_MICRO_CHUNK(csave,LIGHTPHYS_VARIABLE_GROUPID,GroupID);
	csave.End_Chunk();

	return true;
}

bool LightPhysClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case LIGHTPHYS_CHUNK_DECOPHYS:
				DecorationPhysClass::Load(cload);
				break;

			case LIGHTPHYS_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,LIGHTPHYS_VARIABLE_VISSECTORID,VisSectorID);
						READ_MICRO_CHUNK(cload,LIGHTPHYS_VARIABLE_GROUPID,GroupID);
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

	//(gth) turn off the disabled flag, let the building re-set this when it updates its state.
	Set_Disabled(false);

	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}

void LightPhysClass::On_Post_Load(void)
{
	// Set our cull box but don't let the culling system re-insert us.
	Set_Cull_Box(Model->Get_Bounding_Box(),true);
}
