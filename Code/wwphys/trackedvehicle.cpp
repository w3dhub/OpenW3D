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
 *                     $Archive:: /Commando/Code/wwphys/trackedvehicle.cpp                    $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/15/02 1:44p                                               $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "trackedvehicle.h"
#include "rendobj.h"
#include "lineseg.h"
#include "lookuptable.h"
#include "physcoltest.h"
#include "pscene.h"
#include "physcontrol.h"
#include "wwphysids.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"
#include "wwhack.h"
#include "wwprofile.h"
#include "wheel.h"
#include "octbox.h"
#include "mesh.h"
#include "meshmdl.h"
#include "matinfo.h"
#include "vertmaterial.h"
#include "mapper.h"
#include <string.h>


DECLARE_FORCE_LINK(trackedvehicle);



static bool _is_left_track_name(const char * name) 
{
	const char * LEFT_TRACK_NAME0 = "V_TRACK-L";
	const char * LEFT_TRACK_NAME1 = "V_TREAD-L";
	const char * LEFT_TRACK_NAME2 = "V_TRACK_L";
	const char * LEFT_TRACK_NAME3 = "V_TREAD_L";
	
	/*
	** Track skins can only be sub-objects of a hierarchy so check the name
	** after the hierarchy name.
	*/
	const char * sub_name = strchr(name,'.');
	if (sub_name != NULL) {
		sub_name++;
		if (	(strnicmp(sub_name,LEFT_TRACK_NAME0,strlen(LEFT_TRACK_NAME0)) == 0) ||
				(strnicmp(sub_name,LEFT_TRACK_NAME1,strlen(LEFT_TRACK_NAME1)) == 0) || 
				(strnicmp(sub_name,LEFT_TRACK_NAME2,strlen(LEFT_TRACK_NAME2)) == 0) || 
				(strnicmp(sub_name,LEFT_TRACK_NAME3,strlen(LEFT_TRACK_NAME3)) == 0)) 
		{
			return true;
		}
	}
	return false;
}

static bool _is_right_track_name(const char * name)
{
	const char * RIGHT_TRACK_NAME0 = "V_TRACK-R";
	const char * RIGHT_TRACK_NAME1 = "V_TREAD-R";
	const char * RIGHT_TRACK_NAME2 = "V_TRACK_R";
	const char * RIGHT_TRACK_NAME3 = "V_TREAD_R";

	/*
	** Track skins can only be sub-objects of a hierarchy so check the name
	** after the hierarchy name.
	*/
	const char * sub_name = strchr(name,'.');
	if (sub_name != NULL) {
		sub_name++;
		if (	(strnicmp(sub_name,RIGHT_TRACK_NAME0,strlen(RIGHT_TRACK_NAME0)) == 0) ||
				(strnicmp(sub_name,RIGHT_TRACK_NAME1,strlen(RIGHT_TRACK_NAME1)) == 0) || 
				(strnicmp(sub_name,RIGHT_TRACK_NAME2,strlen(RIGHT_TRACK_NAME2)) == 0) || 
				(strnicmp(sub_name,RIGHT_TRACK_NAME3,strlen(RIGHT_TRACK_NAME3)) == 0)) 
		{
			return true;
		}
	}
	return false;
}


/**************************************************************************************
**
** TrackedVehicleClass Implementation
**
**************************************************************************************/
												
/*
** Declare a PersistFactory for TrackedVehicleClasses
*/
SimplePersistFactoryClass<TrackedVehicleClass,PHYSICS_CHUNKID_TRACKEDVEHICLE>	_TrackedVehicleFactory;

/*
** Chunk-ID's used by tracked vehicle class
*/
enum 
{
	TRACKEDVEHICLE_CHUNK_VEHICLEPHYS			= 0x00119801,
	TRACKEDVEHICLE_CHUNK_VARIABLES,
};


// Debugging colors
const Vector3	SPRING_COLOR(1.0f,0.5f,0.0f);
const Vector3	SUSPENSION_FORCE_COLOR(1.0f,0.0f,0.0f);

TrackedVehicleClass::TrackedVehicleClass(void) :
	LeftTrackMovement(0),
	RightTrackMovement(0),
	LeftTrackLastPosition(0,0,0),
	RightTrackLastPosition(0,0,0)
{
}
 
void TrackedVehicleClass::Init(const TrackedVehicleDefClass & def) 
{
	VehiclePhysClass::Init(def);	
}

TrackedVehicleClass::~TrackedVehicleClass(void)
{
}

void TrackedVehicleClass::Render(RenderInfoClass & rinfo)
{
	/*
	** Compute the track movement
	*/
	const Matrix3D & tm = Get_Transform();
	float extent_y = ContactBox->InnerBox.Extent.Y;
	float extent_z = ContactBox->InnerBox.Extent.Z;
	Vector3 left_track_position = tm * Vector3(0,extent_y,-extent_z);
	Vector3 right_track_position = tm * Vector3(0,-extent_y,-extent_z);

	Vector3 forward;
	tm.Get_X_Vector(&forward);

	Vector3 move;
	Vector3::Subtract(left_track_position,LeftTrackLastPosition,&move);										
	LeftTrackMovement = Vector3::Dot_Product(move,forward);

	Vector3::Subtract(right_track_position,RightTrackLastPosition,&move);
	RightTrackMovement = Vector3::Dot_Product(move,forward);
	LeftTrackLastPosition = left_track_position;
	RightTrackLastPosition = right_track_position;
	
	/*
	** Update the mappers
	*/
	const TrackedVehicleDefClass * def = Get_TrackedVehicleDef();
	for (int i=0; i<TrackMappers.Count(); i++) {
		float movement = 0.0f;
		if (TrackMappers[i].TrackType == LEFT_TRACK) {
			movement = LeftTrackMovement;
		} else {
			movement = RightTrackMovement;
		}

		TrackMappers[i].Mapper->Set_UV_Offset_Delta(
			Vector2(def->TrackUScaleFactor * movement,def->TrackVScaleFactor * movement) );
	}

	/*
	** Parent class render
	*/
	VehiclePhysClass::Render(rinfo);
}

void TrackedVehicleClass::Set_Model(RenderObjClass * model)
{
	VehiclePhysClass::Set_Model(model);
	Update_Cached_Model_Parameters();
}

void TrackedVehicleClass::Update_Cached_Model_Parameters(void)
{
	/*
	** Reset our array of mapper pointers
	*/
	TrackMappers.Delete_All(false);
	
	/*
	** Make sure that this model has unique meshes for its tracks
	** and grab pointers to the mappers.
	*/
	if (Model != NULL) {
		Grab_Track_Mappers(Model);
	}
}

void TrackedVehicleClass::Grab_Track_Mappers(RenderObjClass * model)
{
	for (int i=0; i<model->Get_Num_Sub_Objects(); i++) {
		RenderObjClass * sub_obj = model->Get_Sub_Object(i);
		if (sub_obj && (sub_obj->Class_ID() == RenderObjClass::CLASSID_MESH)) {
			const char * name = sub_obj->Get_Name();
			if (_is_left_track_name(name)) {
				Add_Track_Mappers((MeshClass*)sub_obj,LEFT_TRACK);
			}
			if (_is_right_track_name(name)) {
				Add_Track_Mappers((MeshClass*)sub_obj,RIGHT_TRACK);
			}
		}
		
		Grab_Track_Mappers(sub_obj);
		REF_PTR_RELEASE(sub_obj);
	}
}

void TrackedVehicleClass::Add_Track_Mappers(MeshClass * mesh,int track_type)
{
	/*
	** First check if this model has a linear offset mapper in any of its vertex materials
	*/
	bool has_mapper = false;
	MaterialInfoClass * matinfo = mesh->Get_Material_Info();

	if (matinfo != NULL) {
		for (int i=0; i<matinfo->Vertex_Material_Count(); i++) {
			VertexMaterialClass * vmtl = matinfo->Peek_Vertex_Material(i);
			if (vmtl != NULL) {
				TextureMapperClass * mapper = vmtl->Get_Mapper(0);
				if ((mapper != NULL) && (mapper->Mapper_ID() == TextureMapperClass::MAPPER_ID_LINEAR_OFFSET)) {
					has_mapper = true;
				}
				REF_PTR_RELEASE(mapper);
			}
		}			
		REF_PTR_RELEASE(matinfo);
	}

	/*
	** If it does, then make the mesh unique, re-dig out the mapper, and hang onto its pointer
	*/
	if (has_mapper) {
		mesh->Make_Unique();
		
		MaterialInfoClass * matinfo = mesh->Get_Material_Info();

		if (matinfo != NULL) {
			for (int i=0; i<matinfo->Vertex_Material_Count(); i++) {
				VertexMaterialClass * vmtl = matinfo->Peek_Vertex_Material(i);
				if (vmtl != NULL) {
					TextureMapperClass * mapper = vmtl->Get_Mapper(0);
					if ((mapper != NULL) && (mapper->Mapper_ID() == TextureMapperClass::MAPPER_ID_LINEAR_OFFSET)) {
						TrackMapperStruct ts;
						ts.Mapper = (LinearOffsetTextureMapperClass*)mapper;
						ts.TrackType = track_type;
						TrackMappers.Add(ts);

#if 0 //Paranoid debug logging
						WWDEBUG_SAY(("Grabbing Track Mesh! \r\n"));
						WWDEBUG_SAY(("  container: 0x%X mesh: 0x%X vmtl: 0x%X mapper 0x%X\r\n",
							(unsigned int)mesh->Get_Container(),
							(unsigned int)mesh,
							(unsigned int)vmtl,
							(unsigned int)mapper));
						WWDEBUG_SAY(("  tracked vehicle: 0x%X\r\n",(unsigned int)this));
						WWDEBUG_SAY(("  Name = %s\r\n",mesh->Get_Name()));
#endif
					}
					REF_PTR_RELEASE(mapper);
				}
			}			
			REF_PTR_RELEASE(matinfo);
		}
	}
}

void TrackedVehicleClass::Compute_Force_And_Torque(Vector3 * force,Vector3 * torque)
{
	{
		WWPROFILE("TrackedVehicleClass::Compute_Force_And_Torque");

		const TrackedVehicleDefClass * def = Get_TrackedVehicleDef();
		WWASSERT(def != NULL);

		/*
		** Compute the left and right track torque
		** Accept either strafe or turn as a turn command
		*/
		LeftTrackTorque = RightTrackTorque = 0.0f;
		if (Controller) {
			float forward = Controller->Get_Move_Forward();
			float left = 0.0f;
			if (WWMath::Fabs(Controller->Get_Turn_Left()) > WWMath::Fabs(Controller->Get_Move_Left())) {
				left = Controller->Get_Turn_Left();
			} else {
				left = Controller->Get_Move_Left();
			}

			float turn_scale = 1.0f - WWMath::Fabs(left) * (1.0f - def->TurnTorqueScaleFactor);
#if 0
			if (forward != 0.0f) {
				turn_scale = 1.0f;
			}

			LeftTrackTorque = WWMath::Clamp((2.12f * forward - left),-1.0f,1.0f);
			RightTrackTorque = WWMath::Clamp((2.12f * forward + left),-1.0f,1.0f);

			LeftTrackTorque *= def->MaxEngineTorque * turn_scale;
			RightTrackTorque *= def->MaxEngineTorque * turn_scale;
#else
			LeftTrackTorque = (forward - left) * def->MaxEngineTorque * turn_scale;
			RightTrackTorque = (forward + left) * def->MaxEngineTorque * turn_scale;
#endif

		}
		
		/*
		** Update each wheel's torque.
		*/
		for (int iwheel = 0; iwheel<Wheels.Length(); iwheel++) {

			if (Wheels[iwheel]->Get_Flag(WheelClass::LEFT_TRACK)) {
				((TrackWheelClass *)Wheels[iwheel])->Set_Axle_Torque(LeftTrackTorque);
			}
			if (Wheels[iwheel]->Get_Flag(WheelClass::RIGHT_TRACK)) {
				((TrackWheelClass *)Wheels[iwheel])->Set_Axle_Torque(RightTrackTorque);
			}
		}
	}

	/*
	** Pass on to the base class
	*/
	VehiclePhysClass::Compute_Force_And_Torque(force,torque);
}

SuspensionElementClass * TrackedVehicleClass::Alloc_Suspension_Element(void)
{
	return new TrackWheelClass;
}

/*
** Save-Load System
*/
const PersistFactoryClass & TrackedVehicleClass::Get_Factory (void) const
{
	return _TrackedVehicleFactory;
}

bool TrackedVehicleClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(TRACKEDVEHICLE_CHUNK_VEHICLEPHYS);
	VehiclePhysClass::Save(csave);
	csave.End_Chunk();

	return true;
}

bool TrackedVehicleClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case TRACKEDVEHICLE_CHUNK_VEHICLEPHYS:
				VehiclePhysClass::Load(cload);
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

void TrackedVehicleClass::On_Post_Load (void)
{
	VehiclePhysClass::On_Post_Load();
}


/***********************************************************************************************
**
** TrackedVehicleDefClass Implementation
**
***********************************************************************************************/

SimplePersistFactoryClass<TrackedVehicleDefClass,PHYSICS_CHUNKID_TRACKEDVEHICLEDEF>	_TrackedVehicleDefFactory;
DECLARE_DEFINITION_FACTORY(TrackedVehicleDefClass, CLASSID_TRACKEDVEHICLEDEF, "TrackedVehicle") _TrackedVehicleDefDefFactory;

/*
** Chunk ID's used by TrackedVehicleDefClass
*/
enum 
{
	TRACKEDVEHICLEDEF_CHUNK_VEHICLEPHYSDEF			= 0406001454,			// (parent class)
	TRACKEDVEHICLEDEF_CHUNK_VARIABLES,

	TRACKEDVEHICLEDEF_VARIABLE_MAXENGINETORQUE	= 0x00,
	TRACKEDVEHICLEDEF_VARIABLE_TRACKUSCALEFACTOR,
	TRACKEDVEHICLEDEF_VARIABLE_TRACKVSCALEFACTOR,
	TRACKEDVEHICLEDEF_VARIABLE_TURNTORQUESCALEFACTOR,
	
};


TrackedVehicleDefClass::TrackedVehicleDefClass(void) :
	MaxEngineTorque(0.0f),
	TrackUScaleFactor(25.0f),
	TrackVScaleFactor(0.0f),
	TurnTorqueScaleFactor(1.0f)
{
	// make our parameters editable
	FLOAT_EDITABLE_PARAM(TrackedVehicleDefClass, MaxEngineTorque, 0.0f, 100000.0f);
	FLOAT_EDITABLE_PARAM(TrackedVehicleDefClass, TrackUScaleFactor, -1000.0f, 1000.0f);
	FLOAT_EDITABLE_PARAM(TrackedVehicleDefClass, TrackVScaleFactor, -1000.0f, 1000.0f);
	FLOAT_EDITABLE_PARAM(TrackedVehicleDefClass, TurnTorqueScaleFactor, 0.0f, 1.0f);
}

uint32 TrackedVehicleDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_TRACKEDVEHICLEDEF; 
}

const PersistFactoryClass & TrackedVehicleDefClass::Get_Factory (void) const
{
	return _TrackedVehicleDefFactory;
}

PersistClass * TrackedVehicleDefClass::Create(void) const
{
	TrackedVehicleClass * obj = NEW_REF(TrackedVehicleClass,());
	obj->Init(*this);
	return obj;
}

bool TrackedVehicleDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(TRACKEDVEHICLEDEF_CHUNK_VEHICLEPHYSDEF);
	VehiclePhysDefClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(TRACKEDVEHICLEDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,TRACKEDVEHICLEDEF_VARIABLE_MAXENGINETORQUE,MaxEngineTorque);
	WRITE_MICRO_CHUNK(csave,TRACKEDVEHICLEDEF_VARIABLE_TRACKUSCALEFACTOR,TrackUScaleFactor);
	WRITE_MICRO_CHUNK(csave,TRACKEDVEHICLEDEF_VARIABLE_TRACKVSCALEFACTOR,TrackVScaleFactor);
	WRITE_MICRO_CHUNK(csave,TRACKEDVEHICLEDEF_VARIABLE_TURNTORQUESCALEFACTOR,TurnTorqueScaleFactor);
	csave.End_Chunk();
	return true;
}


bool TrackedVehicleDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case TRACKEDVEHICLEDEF_CHUNK_VEHICLEPHYSDEF:
				VehiclePhysDefClass::Load(cload);
				break;

			case TRACKEDVEHICLEDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,TRACKEDVEHICLEDEF_VARIABLE_MAXENGINETORQUE,MaxEngineTorque);
						READ_MICRO_CHUNK(cload,TRACKEDVEHICLEDEF_VARIABLE_TRACKUSCALEFACTOR,TrackUScaleFactor);
						READ_MICRO_CHUNK(cload,TRACKEDVEHICLEDEF_VARIABLE_TRACKVSCALEFACTOR,TrackVScaleFactor);
						READ_MICRO_CHUNK(cload,TRACKEDVEHICLEDEF_VARIABLE_TURNTORQUESCALEFACTOR,TurnTorqueScaleFactor);
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


bool TrackedVehicleDefClass::Is_Type(const char * type_name)
{
	if (openw3d::string_compare(type_name,TrackedVehicleDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return VehiclePhysDefClass::Is_Type(type_name);
	}
}


