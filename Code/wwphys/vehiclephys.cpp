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
 *                     $Archive:: /Commando/Code/wwphys/vehiclephys.cpp                       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/12/01 10:37a                                             $*
 *                                                                                             *
 *                    $Revision:: 26                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "vehiclephys.h"
#include "chunkio.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "wheel.h"
#include "physcontrol.h"
#include "octbox.h"
#include "pscene.h"
#include "wwprofile.h"
#include "vehicledazzle.h"

// Vehicles will sit rolled over for this long before exploding!
const float		EXPIRE_SECONDS								= 4.0f;

// HACK! when the engine is off, decimate the momentum each timestep by this fraction...
const float		PARKING_BRAKE_DAMPING					= 0.5f;

// max number of auxiliary graphical bones to support (e.g. engine flames)
const int		MAX_CAPTURED_BONE_COUNT					= 4;
const char *	ENGINE_FLAME_BONE_NAME					= "ENGINEFLAME";

// Wheel parsing constants...
const char *	WHEELP_BONE_NAME							= "WheelP";		// Position bone (contact patch)
const char *	WHEELC_BONE_NAME							= "WheelC";		// Center bone (rotational center of the wheel)
const char *	WHEELF_BONE_NAME							= "WheelF";		// Fork constraint bone (e.g. back wheel of a motorcycle)
const char *	WHEELT_BONE_NAME							= "WheelT";		// Translation constraint bone (e.g. front wheel of a motorcycle)
const int		WHEEL_INDEX_CHAR0							= 6;
const int		WHEEL_INDEX_CHAR1							= 7;
const int		WHEEL_FLAG_CHAR0							= 8;

// Wheel flags (begin appearing at character WHEEL_FLAG_CHAR0)
const char		WHEEL_FLAG_STEERING						= 'S';			// wheel turns with steering 
const char		WHEEL_FLAG_INV_STEERING					= 'I';			// wheel turns opposite steering
const char		WHEEL_FLAG_TILT_STEERING				= 'T';			// wheel turns when vehicle (bike) tilts
const char		WHEEL_FLAG_ENGINE							= 'E';			// wheel is attached to engine
const char		WHEEL_FLAG_LEFT_TRACK					= 'L';			// wheel is part of the left track
const char		WHEEL_FLAG_RIGHT_TRACK					= 'R';			// wheel is part of the right track
const char		WHEEL_FLAG_FAKE							= 'F';			// wheel is fake!

																
																						
																						
																						
/***********************************************************************************************
**
** VehiclePhysClass Implementation
**
***********************************************************************************************/


bool VehiclePhysClass::_DisableVehicleSimulation = false;
bool VehiclePhysClass::_DisableVehicleRendering = false;


/*
** Chunk Ids used by VehiclePhysClass
*/
enum
{
	VEHICLEPHYS_CHUNK_RIGIDBODY				= 405001445,		// parent class
	VEHICLEPHYS_CHUNK_VARIABLES,

	VEHICLEPHYS_VARIABLE_ISENGINEON			= 0,
};


VehiclePhysClass::VehiclePhysClass(void) :
	IsEngineOn(false),
	RealWheelCount(0),
	DriveWheelCount(0),
	EngineFlameBones(MAX_CAPTURED_BONE_COUNT),
	LastGoodPosition(1),
	ExpireTimer(EXPIRE_SECONDS)
{
	for (int i=0; i<MAX_CAPTURED_BONE_COUNT; i++) {
		EngineFlameBones[i] = -1;
	}
	Enable_Shadow_Generation(true);
}

void VehiclePhysClass::Init(const VehiclePhysDefClass & def)
{
	RigidBodyClass::Init(def);
	for (int i=0; i<MAX_CAPTURED_BONE_COUNT; i++) {
		EngineFlameBones[i] = -1;
	}
	Enable_Shadow_Generation(true);
}

void VehiclePhysClass::Definition_Changed(void)
{
	RigidBodyClass::Definition_Changed();

	for (int i=0; i<Wheels.Length(); i++) {
		const VehiclePhysDefClass * def = Get_VehiclePhysDef();
		Wheels[i]->Set_Spring_Constant(def->Get_Spring_Constant());
		Wheels[i]->Set_Damping_Coefficient(def->Get_Damping_Constant());
		Wheels[i]->Set_Spring_Length(def->Get_Spring_Length());
	}
}

VehiclePhysClass::~VehiclePhysClass(void)
{
	Release_Wheels();
	Release_Auxiliary_Bones();
	Release_Dazzles();
}


void VehiclePhysClass::Update_Wheels (void)
{
	/*
	** update the wheels graphically
	*/
	Set_Stationary_Collision_Region();
	Inc_Ignore_Counter();

	for (int i=0; i<Wheels.Length(); i++) {
		Wheels[i]->Update_Model();
	}
	
	PhysicsSceneClass::Get_Instance()->Release_Collision_Region();
	Dec_Ignore_Counter();

	return ;
}

void VehiclePhysClass::Non_Physical_Wheel_Update(float suspension_fraction,float rotation)
{
	for (int i=0; i<Wheels.Length(); i++) {
		Wheels[i]->Non_Physical_Update(suspension_fraction,rotation);
	}
}


void VehiclePhysClass::Render(RenderInfoClass & rinfo)
{
	const VehiclePhysDefClass * def = Get_VehiclePhysDef();

	/*
	** update the wheels graphically
	*/
	Update_Wheels();

	/*
	** update the engine flames
	*/
	Matrix3D flame_tm(1);
	flame_tm.Translate_Z(-Get_Normalized_Engine_Flame() * def->EngineFlameLength);
	for (int ibone=0; ibone<MAX_CAPTURED_BONE_COUNT; ibone++) {
		if (EngineFlameBones[ibone] != -1) {
			Model->Control_Bone(EngineFlameBones[ibone],flame_tm);
		}
	}

	/*
	** Update the dazzles
	*/
	for (int idazzle=0; idazzle<Dazzles.Length(); idazzle++) {
		Dazzles[idazzle]->Pre_Render_Update(this);
	}

	/*
	** Pass on up!
	*/
	RigidBodyClass::Render(rinfo);
}

void VehiclePhysClass::Set_Model(RenderObjClass * model)
{
	Release_Wheels();
	Release_Auxiliary_Bones();
	RigidBodyClass::Set_Model(model);
	Update_Cached_Model_Parameters();
}

float VehiclePhysClass::Compute_Approximate_Ride_Height(void)
{
	float val = 0.0f;

	if (ContactBox != NULL) {

		OBBoxClass box;
		ContactBox->Get_Inner_Box(&box,Quaternion(1),Vector3(0,0,0));
		val += (box.Extent.Z - box.Center.Z);
		
		if (Wheels.Length() > 0) {
			val += 0.66f * Wheels[0]->Get_Spring_Length();
		} else { 
			val += 0.6f;
		}

	} else { 
		val = 1.0f;
	}

	return val;
}

void VehiclePhysClass::Timestep(float dt)
{
	WWPROFILE("VehiclePhysClass::Timestep");
	int i;

	/*
	** Take a copy of our state for later "roll testing"
	*/
	RigidBodyStateStruct initial_state = State;

	/*
	** If the vehicle is not in use and it is on the ground, dampen the hell out of it
	*/
#if 0
	int contact_count=0;
	for (i=0; i<Wheels.Length(); i++) {
		if ((Wheels[i] != NULL) && (Wheels[i]->Get_Flag(SuspensionElementClass::INCONTACT) == true)) {
			contact_count++;
		}
	}
	if ((contact_count >= 3) && (Is_Engine_Enabled() == false)) {
		State.LMomentum *= PARKING_BRAKE_DAMPING;
		State.AMomentum *= PARKING_BRAKE_DAMPING;
	}
#endif

	/*
	** If there are wheels contacting the ground, turn off friction for the body of the vehicle
	*/
	bool wheels_touching_ground = false;
	for (i=0; i<Wheels.Length(); i++) {
		if ((Wheels[i] != NULL) && (Wheels[i]->Get_Flag(SuspensionElementClass::INCONTACT) == true)) {
			wheels_touching_ground = true;
		}
	}
	Set_Flag(FRICTION_DISABLED,wheels_touching_ground);

	/*
	** Call to parent class
	*/
	if (Get_VehiclePhysDef()->IsFake == false) {
		RigidBodyClass::Timestep(dt);
	}

	/*
	** See if we should be destroyed due to coming to rest upside down
	*/
	float up_cos = Get_Transform().Get_Z_Vector().Z;
	const float MIN_Z_COSINE = 0.25f;
	if (up_cos < MIN_Z_COSINE) {
		ExpireTimer -= dt;
		if (ExpireTimer < 0.0f) {
			ExpireTimer = EXPIRE_SECONDS; // if expiration is denied, try again later.
			Expire();
		}
	} else {
		ExpireTimer = EXPIRE_SECONDS;
	}
}

SuspensionElementClass * VehiclePhysClass::Peek_Wheel(int wheel_index)
{
	return Wheels[wheel_index];
}


void VehiclePhysClass::Compute_Force_And_Torque(Vector3 * force,Vector3 * torque)
{
	{
		WWPROFILE("VehiclePhysClass::Compute_Force_And_Torque");
		
		/*
		** Compute forces and torques for each wheel.
		*/
		int goodwheels = 0;
		for (int iwheel = 0; iwheel<Wheels.Length(); iwheel++) {
			Wheels[iwheel]->Compute_Force_And_Torque(force,torque);
			if (Wheels[iwheel]->Get_Flag(WheelClass::INCONTACT)) {
				goodwheels++;
			}
		}
		
		/*
		** If all of our wheels are contacting and we are mostly upright,
		** remember this transform as a "good" position.
		*/
		if ((goodwheels == Wheels.Length()) && (Rotation.Get_Z_Vector().Z > 0.7f)) {
			LastGoodPosition.Set(Rotation,State.Position);
		}
	}

	/*
	** Let base class add in its forces
	*/
	RigidBodyClass::Compute_Force_And_Torque(force,torque);
}

bool VehiclePhysClass::Can_Go_To_Sleep(float dt)
{
	/*
	** Vehicles go to sleep if at least three wheels are in contact and their 
	** velocities are below some thresh-hold and their controller isn't doing anything.
	*/
	if ((Controller != NULL) && (Controller->Is_Inactive() != true)) {
		GoToSleepTimer = RBODY_SLEEP_DELAY;
		return false;
	}
	
	/*
	** Check our velocities
	*/
	const float VEL_THRESHHOLD = 0.10f;
	const float AVEL_THRESHHOLD = 0.10f;

	float max_lmomentum2 = Mass * Mass * VEL_THRESHHOLD * VEL_THRESHHOLD;
	float max_amomentum2 = IBody[1][1] * IBody[1][1] * AVEL_THRESHHOLD * AVEL_THRESHHOLD;
	bool tried_to_sleep = false;

	if ((State.LMomentum.Length2() < max_lmomentum2) && 
		(State.AMomentum.Length2() < max_amomentum2) ) 
	{

		/*
		** Count the contacts
		*/
		int contact_count = 0;

		for (int iwheel = 0; iwheel<Wheels.Length(); iwheel++) {
			if (Wheels[iwheel]->Get_Flag(SuspensionElementClass::INCONTACT)) {
				contact_count++;
			}
		}	

		if ((Wheels.Length() <= 2) || (contact_count >= 3) || (ContactBox->ContactCount >= 3)) {
			tried_to_sleep = true;
			if (GoToSleepTimer < 0.0f) {
				return true;
			}
		}
	}

	if (tried_to_sleep) {
		GoToSleepTimer -= dt;
	} else {
		GoToSleepTimer = RBODY_SLEEP_DELAY;
	}

	return false;
}


void VehiclePhysClass::Update_Cached_Model_Parameters(void)
{
	if (Model == NULL) return;

	// capture the wheels
	Release_Wheels();
	Create_Wheels();

	// capture auxiliary bones
	Release_Auxiliary_Bones();
	Capture_Auxiliary_Bones();

	// count the drive wheels and real wheels
	DriveWheelCount = 0;
	RealWheelCount = 0;
	for (int i=0; i<Wheels.Length(); i++) {
		if (	Wheels[i]->Get_Flag(SuspensionElementClass::ENGINE) ||
				Wheels[i]->Get_Flag(SuspensionElementClass::LEFT_TRACK) ||
				Wheels[i]->Get_Flag(SuspensionElementClass::RIGHT_TRACK) ) 
		{
			DriveWheelCount++;
		}
		
		// "Real" wheels are all of them except the fake ones.
		if (Wheels[i]->Get_Flag(SuspensionElementClass::FAKE) == false) {
			RealWheelCount++;
		}
	}

	// cache pointers to the dazzles
	Release_Dazzles();
	Capture_Dazzles();

	// Force the shadow manager to use a scaled version of our "blob box"
	ShadowManager.Enable_Force_Use_Blob_Box(true);
	ShadowManager.Set_Blob_Box_Projection_Scale(Vector3(1.5f,1.5f,1.5f));
}

void VehiclePhysClass::Release_Wheels(void)
{
	// release the bones we "captured" and destroy the wheels
	for (int i=0; i<Wheels.Length(); i++) {
		delete Wheels[i];
		Wheels[i] = NULL;
	}
	Wheels.Resize(0);
	DriveWheelCount = 0;
	RealWheelCount = 0;
}

void VehiclePhysClass::Create_Wheels(void)
{
	RenderObjClass * model = Peek_Model();
	if (model == NULL) return;

	const VehiclePhysDefClass * def = Get_VehiclePhysDef();
	WWASSERT(def != NULL);
	
	int ibone;
	int wheelcount = 0;

	// Count the wheels.
	for (ibone=0; ibone < model->Get_Num_Bones(); ibone++) {

		// search for bones named WheelP
		const char * bonename = model->Get_Bone_Name(ibone);
        if (strnicmp(bonename,WHEELP_BONE_NAME,strlen(WHEELP_BONE_NAME)) == 0) {
			wheelcount++;
		}
	}

	if (wheelcount == 0) {
		return;
	}

	// Allocate the array of pointers for the wheels
	Wheels.Resize(wheelcount);
	for (int i=0; i<Wheels.Length(); i++) {
		Wheels[i] = NULL;
	}

	int curwheel = 0;
	for (ibone=0; (ibone < model->Get_Num_Bones()) && (curwheel < Wheels.Length()); ibone++) {

		// for each bone named WheelP
		const char * wpname = model->Get_Bone_Name(ibone);
        if (strnicmp(wpname,WHEELP_BONE_NAME,strlen(WHEELP_BONE_NAME)) == 0) {

			int position_bone = ibone;
			int rotation_bone = Find_Rotation_Bone(model,wpname);
			int fork_bone = Find_Fork_Bone(model,wpname);
			int trans_bone = Find_Translation_Bone(model,wpname);
		
			if (position_bone != -1) {

				// initialize a wheel structure:
				SuspensionElementClass * new_wheel = Alloc_Suspension_Element();
				new_wheel->Init(this,position_bone,rotation_bone,fork_bone,trans_bone);
				new_wheel->Set_Spring_Constant(def->SpringConstant);
				new_wheel->Set_Damping_Coefficient(def->DampingConstant);
				new_wheel->Set_Spring_Length(def->SpringLength);

				// parse any flag characters
				unsigned int index = WHEEL_FLAG_CHAR0;
				while (index < strlen(wpname)) {
					switch (wpname[index]) {
					case WHEEL_FLAG_STEERING:
						new_wheel->Set_Flag(SuspensionElementClass::STEERING,true);
						break;

					case WHEEL_FLAG_INV_STEERING:
						new_wheel->Set_Flag(SuspensionElementClass::INV_STEERING,true);
						break;

					case WHEEL_FLAG_TILT_STEERING:
						new_wheel->Set_Flag(SuspensionElementClass::TILT_STEERING,true);
						break;

					case WHEEL_FLAG_ENGINE:
						new_wheel->Set_Flag(SuspensionElementClass::ENGINE,true);
						break;

					case WHEEL_FLAG_LEFT_TRACK:
						new_wheel->Set_Flag(SuspensionElementClass::LEFT_TRACK,true);
						break;

					case WHEEL_FLAG_RIGHT_TRACK:
						new_wheel->Set_Flag(SuspensionElementClass::RIGHT_TRACK,true);
						break;

					case WHEEL_FLAG_FAKE:
						new_wheel->Set_Flag(SuspensionElementClass::FAKE,true);
						break;

					}
					index++;
				}
				
				// install the wheel
				Wheels[curwheel++] = new_wheel;
			}
		}
	}
}

int VehiclePhysClass::Find_Fork_Bone(RenderObjClass * model,const char * wpname)
{
	for (int ibone=0; ibone < model->Get_Num_Bones(); ibone++) {
				
		const char * wfname = model->Get_Bone_Name(ibone);
        if (	(strnicmp(wfname,WHEELF_BONE_NAME,strlen(WHEELF_BONE_NAME)) == 0) &&
				(wfname[WHEEL_INDEX_CHAR0] == wpname[WHEEL_INDEX_CHAR0]) &&
				(wfname[WHEEL_INDEX_CHAR1] == wpname[WHEEL_INDEX_CHAR1]))
		{
			return ibone;
		}
	}
	return -1;
}

int VehiclePhysClass::Find_Rotation_Bone(RenderObjClass * model,const char * wpname)
{
	for (int ibone=0; ibone < model->Get_Num_Bones(); ibone++) {
				
		const char * wcname = model->Get_Bone_Name(ibone);
        if (	(strnicmp(wcname,WHEELC_BONE_NAME,strlen(WHEELC_BONE_NAME)) == 0) &&
				(wcname[WHEEL_INDEX_CHAR0] == wpname[WHEEL_INDEX_CHAR0]) &&
				(wcname[WHEEL_INDEX_CHAR1] == wpname[WHEEL_INDEX_CHAR1]))
		{
			return ibone;
		}
	}
	return -1;
}

int VehiclePhysClass::Find_Translation_Bone(RenderObjClass * model,const char * wpname)
{
	for (int ibone=0; ibone < model->Get_Num_Bones(); ibone++) {
				
		const char * wtname = model->Get_Bone_Name(ibone);
        if (	(strnicmp(wtname,WHEELT_BONE_NAME,strlen(WHEELT_BONE_NAME)) == 0) &&
				(wtname[WHEEL_INDEX_CHAR0] == wpname[WHEEL_INDEX_CHAR0]) &&
				(wtname[WHEEL_INDEX_CHAR1] == wpname[WHEEL_INDEX_CHAR1]))
		{
			return ibone;
		}
	}
	return -1;
}
 
void VehiclePhysClass::Release_Auxiliary_Bones(void)
{
	// release any bones that we currently have captured
	if (Model != NULL) {
		for (int i=0;i<MAX_CAPTURED_BONE_COUNT; i++) {
			if (EngineFlameBones[i] != -1) {
				Model->Release_Bone(EngineFlameBones[i]);
				EngineFlameBones[i] = -1;
			}
		}
	}
}

void VehiclePhysClass::Capture_Auxiliary_Bones(void)
{
	// search through the model for bones beginning with ENGINEFLAME
	int ibone = 0;
	int engine_bone_count = 0;

	for (ibone=0; (ibone < Model->Get_Num_Bones()) && (engine_bone_count < MAX_CAPTURED_BONE_COUNT); ibone++) {
		const char * bone_name = Model->Get_Bone_Name(ibone);
        if (strnicmp(bone_name,ENGINE_FLAME_BONE_NAME,strlen(ENGINE_FLAME_BONE_NAME)) == 0) {
			
			EngineFlameBones[engine_bone_count] = ibone;
			Model->Capture_Bone(ibone);
			engine_bone_count++;
		}
	}
}


void VehiclePhysClass::Release_Dazzles(void)
{
	// delete the dazzle controllers
	for (int i=0; i<Dazzles.Length(); i++) {
		delete Dazzles[i];
		Dazzles[i] = NULL;
	}
	Dazzles.Resize(0);
}

void VehiclePhysClass::Capture_Dazzles(void)
{
	RenderObjClass * model = Peek_Model();
	if (model == NULL) return;

	[[maybe_unused]] const VehiclePhysDefClass * def = Get_VehiclePhysDef();
	WWASSERT(def != NULL);
	
	int imodel;
	int dazzlecount = 0;

	// Count the dazzles.
	for (imodel=0; imodel < model->Get_Num_Sub_Objects(); imodel++) {

		// Search for dazzle render objects whose name starts with _HLight, _TLight, or _BLight
		RenderObjClass * subobj = model->Get_Sub_Object(imodel);
		if ((subobj != NULL) && (subobj->Class_ID() == RenderObjClass::CLASSID_DAZZLE)) {
			if (VehicleDazzleClass::Is_Vehicle_Dazzle(subobj)) {
				dazzlecount++;
			}
		}
		REF_PTR_RELEASE(subobj);
	}

	// Return if we didn't find any dazzles that want to be controlled by the vehicle logic
	if (dazzlecount == 0) {
		return;
	}

	// Allocate the array of pointers for the wheels
	Dazzles.Resize(dazzlecount);
	for (int i=0; i<Dazzles.Length(); i++) {
		Dazzles[i] = NULL;
	}

	// Create dazzle controllers
	int curdazzle = 0;
	for (imodel=0; (imodel < model->Get_Num_Sub_Objects()) && (curdazzle < Dazzles.Length()); imodel++) {

		RenderObjClass * subobj = model->Get_Sub_Object(imodel);
		if (VehicleDazzleClass::Is_Vehicle_Dazzle(subobj)) {
			Dazzles[curdazzle] = Create_Dazzle_Controller(subobj);
			curdazzle++;
		}
		REF_PTR_RELEASE(subobj);
	}
}


VehicleDazzleClass * VehiclePhysClass::Create_Dazzle_Controller(RenderObjClass * obj)
{
	WWASSERT(VehicleDazzleClass::Is_Vehicle_Dazzle(obj));
	VehicleDazzleClass * controller = new VehicleDazzleClass();
	controller->Set_Model((DazzleRenderObjClass *)obj);
	return controller;
}

void VehiclePhysClass::Teleport_To_Last_Good_Position(void)
{
	Set_Transform(LastGoodPosition);
}


bool	VehiclePhysClass::Is_In_Contact(void)
{
	bool retval = false;

	//
	//	Check to see if any of the wheels are on the ground
	//
	for (int index = 0; index < Wheels.Length(); index++) {
		
		//
		//	Is this wheel on the ground?
		//
		if (Wheels[index]->Get_Flag(WheelClass::INCONTACT)) {
			retval = true;
			break;
		}
	}

	return retval;
}


bool VehiclePhysClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(VEHICLEPHYS_CHUNK_RIGIDBODY);
	RigidBodyClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(VEHICLEPHYS_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,VEHICLEPHYS_VARIABLE_ISENGINEON,IsEngineOn);
	csave.End_Chunk();

	return true;
}

bool VehiclePhysClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case VEHICLEPHYS_CHUNK_RIGIDBODY:
				RigidBodyClass::Load(cload);
				break;
				
			case VEHICLEPHYS_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,VEHICLEPHYS_VARIABLE_ISENGINEON,IsEngineOn);
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
 
 
/***********************************************************************************************
**
** VehiclePhysDefClass Implementation
**
***********************************************************************************************/

/*
** Declare a PersistFactory for VehiclePhysDefClass
** Dont declare a Definition Factory because we don't want users to directly create this type.
*/
SimplePersistFactoryClass<VehiclePhysDefClass,PHYSICS_CHUNKID_VEHICLEPHYSDEF>	_VehiclePhysDefFactory;

/*
** Chunk ID's used by VehiclePhysDefClass
*/
enum 
{
	VEHICLEPHYSDEF_CHUNK_RIGIDBODYDEF					= 405001519,			// (parent class)
	VEHICLEPHYSDEF_CHUNK_VARIABLES,

	VEHICLEPHYSDEF_VARIABLE_SPRINGCONSTANT				= 0x00,	
	VEHICLEPHYSDEF_VARIABLE_DAMPINGCONSTANT,
	VEHICLEPHYSDEF_VARIABLE_SPRINGLENGTH,
	VEHICLEPHYSDEF_VARIABLE_TRACTIONMULTIPLIER,
	VEHICLEPHYSDEF_VARIABLE_LATERALMOMENTARM,
	VEHICLEPHYSDEF_VARIABLE_TRACTIVEMOMENTARM,
	VEHICLEPHYSDEF_VARIABLE_ENGINEFLAMELENGTH,
	VEHICLEPHYSDEF_VARIABLE_ISFAKE,
};

VehiclePhysDefClass::VehiclePhysDefClass(void) :
	SpringConstant(DEFAULT_SPRING_CONSTANT),
	DampingConstant(DEFAULT_DAMPING_COEFFICIENT),
	SpringLength(DEFAULT_SPRING_LENGTH),
	TractionMultiplier(2.0f),
	LateralMomentArm(0.0f),
	TractiveMomentArm(0.0f),
	EngineFlameLength(1.0f),
	IsFake(false)
{
	// make our parameters editable!
	EDITABLE_PARAM(VehiclePhysDefClass, ParameterClass::TYPE_BOOL, IsFake);	
	FLOAT_UNITS_PARAM(VehiclePhysDefClass, SpringConstant, 0.0f, 100000.0f,"N/m");
	FLOAT_UNITS_PARAM(VehiclePhysDefClass, DampingConstant, 0.0f, 100000.0f,"N/(m/s)");
	FLOAT_UNITS_PARAM(VehiclePhysDefClass, SpringLength, 0.0f, 100.0f,"m");
	FLOAT_EDITABLE_PARAM(VehiclePhysDefClass, TractionMultiplier, 0.5f, 5.0f);
	FLOAT_UNITS_PARAM(VehiclePhysDefClass, LateralMomentArm, 0.0f, 10.0f,"m");
	FLOAT_UNITS_PARAM(VehiclePhysDefClass, TractiveMomentArm, 0.0f, 10.0f,"m");
	FLOAT_UNITS_PARAM(VehiclePhysDefClass, EngineFlameLength, 0.0f, 100.0f,"m");
}

VehiclePhysDefClass::~VehiclePhysDefClass(void)
{
}

uint32 VehiclePhysDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_VEHICLEPHYSDEF; 
}

const PersistFactoryClass & VehiclePhysDefClass::Get_Factory (void) const
{
	return _VehiclePhysDefFactory;
}

bool VehiclePhysDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(VEHICLEPHYSDEF_CHUNK_RIGIDBODYDEF);
	RigidBodyDefClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(VEHICLEPHYSDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,VEHICLEPHYSDEF_VARIABLE_SPRINGCONSTANT,SpringConstant);
	WRITE_MICRO_CHUNK(csave,VEHICLEPHYSDEF_VARIABLE_DAMPINGCONSTANT,DampingConstant);
	WRITE_MICRO_CHUNK(csave,VEHICLEPHYSDEF_VARIABLE_SPRINGLENGTH,SpringLength);
	WRITE_MICRO_CHUNK(csave,VEHICLEPHYSDEF_VARIABLE_TRACTIONMULTIPLIER,TractionMultiplier);
	WRITE_MICRO_CHUNK(csave,VEHICLEPHYSDEF_VARIABLE_LATERALMOMENTARM,LateralMomentArm);
	WRITE_MICRO_CHUNK(csave,VEHICLEPHYSDEF_VARIABLE_TRACTIVEMOMENTARM,TractiveMomentArm);
	WRITE_MICRO_CHUNK(csave,VEHICLEPHYSDEF_VARIABLE_ENGINEFLAMELENGTH,EngineFlameLength);
	WRITE_MICRO_CHUNK(csave,VEHICLEPHYSDEF_VARIABLE_ISFAKE,IsFake);
	csave.End_Chunk();
	return true;
}

bool VehiclePhysDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case VEHICLEPHYSDEF_CHUNK_RIGIDBODYDEF:
				RigidBodyDefClass::Load(cload);
				break;

			case VEHICLEPHYSDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,VEHICLEPHYSDEF_VARIABLE_SPRINGCONSTANT,SpringConstant);
						READ_MICRO_CHUNK(cload,VEHICLEPHYSDEF_VARIABLE_DAMPINGCONSTANT,DampingConstant);
						READ_MICRO_CHUNK(cload,VEHICLEPHYSDEF_VARIABLE_SPRINGLENGTH,SpringLength);
						READ_MICRO_CHUNK(cload,VEHICLEPHYSDEF_VARIABLE_TRACTIONMULTIPLIER,TractionMultiplier);
						READ_MICRO_CHUNK(cload,VEHICLEPHYSDEF_VARIABLE_LATERALMOMENTARM,LateralMomentArm);
						READ_MICRO_CHUNK(cload,VEHICLEPHYSDEF_VARIABLE_TRACTIVEMOMENTARM,TractiveMomentArm);
						READ_MICRO_CHUNK(cload,VEHICLEPHYSDEF_VARIABLE_ENGINEFLAMELENGTH,EngineFlameLength);
						READ_MICRO_CHUNK(cload,VEHICLEPHYSDEF_VARIABLE_ISFAKE,IsFake);
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

bool VehiclePhysDefClass::Is_Type(const char * type_name)
{
	if (stricmp(type_name,VehiclePhysDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return RigidBodyDefClass::Is_Type(type_name);
	}
}

