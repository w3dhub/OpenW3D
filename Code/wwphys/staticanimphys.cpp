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
 *                     $Archive:: /Commando/Code/wwphys/staticanimphys.cpp                    $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/18/01 6:17p                                               $*
 *                                                                                             *
 *                    $Revision:: 43                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "staticanimphys.h"
#include "colmathaabox.h"
#include "physinttest.h"
#include "bitstream.h"
#include "wwphysids.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"
#include "movephys.h"
#include "hanim.h"
#include "mesh.h"
#include "boxrobj.h"
#include "wwhack.h"
#include "wwprofile.h"
#include "assetmgr.h"

#include "vertmaterial.h"
#include "dx8wrapper.h"

DECLARE_FORCE_LINK(staticanimphys);




/********************************************************************************************
**
** StaticAnimPhysClass Implementation
**
********************************************************************************************/

/*
** Declare a PersistFactory for StaticAnimPhysClass.  This enables the class to save and load.
*/
SimplePersistFactoryClass<StaticAnimPhysClass,PHYSICS_CHUNKID_STATICANIMPHYS>	_StaticAnimPhysFactory;

/*
** Chunk ID's used by StaticAnimPhysClass
*/
enum
{
	STATICANIMPHYS_CHUNK_STATICPHYS							= 0x01170010,
	STATICANIMPHYS_CHUNK_VARIABLES,
	STATICANIMPHYS_CHUNK_ANIMMANAGER,

	OBSOLETE_STATICANIMPHYS_VARIABLE_DEFID					= 0x00,
	STATICANIMPHYS_VARIABLE_COLLISIONMODE,
};										


StaticAnimPhysClass::StaticAnimPhysClass(void) :
	AnimManager(*this),
	ShadowProjector(NULL)
{
	Set_Collision_Group(0xF);	//Hardcoding this???
}

StaticAnimPhysClass::~StaticAnimPhysClass(void)
{
	if (ShadowProjector != NULL) {
		PhysicsSceneClass::Get_Instance()->Remove_Static_Texture_Projector(ShadowProjector);
		ShadowProjector->Release_Ref();
		ShadowProjector = NULL;
	}
}

void StaticAnimPhysClass::Init(const StaticAnimPhysDefClass & def)
{
	StaticPhysClass::Init(def);
	ProjectorManager.Init(def.ProjectorManagerDef,Model);
	AnimManager.Init(def.AnimManagerDef);
}

void StaticAnimPhysClass::Set_Model(RenderObjClass * model)
{
	// call parent class
	StaticPhysClass::Set_Model(model);
	Update_Cached_Model_Parameters();
}

void StaticAnimPhysClass::Vis_Render(SpecialRenderInfoClass & rinfo)
{
	if (Model != NULL) {
		// static anim objects need to render their bounding box so temporarily make it visible:
		int was_hidden = 0;
		RenderObjClass * bbox = Model->Get_Sub_Object_By_Name("BoundingBox");
		if (bbox != NULL) {
			was_hidden = bbox->Is_Hidden();
			bbox->Set_Hidden(false);
		}

		StaticPhysClass::Vis_Render(rinfo);

		if (bbox != NULL) {
			bbox->Set_Hidden(was_hidden);
			bbox->Release_Ref();
		}
	}
}

void StaticAnimPhysClass::Update_Cached_Model_Parameters(void)
{
	// check if there is a collideable mesh in the model and cache a pointer to it.
	AnimManager.Update_Cached_Model_Parameters();	
}

void StaticAnimPhysClass::Update_Sun_Status(void)
{
	float MAX_XYSIZE2 = 15.0f * 15.0f;

	// weird...  well, I cant have all static objects check for the sun but
	// "small" "animated" ones should (like doors and elevators).  So, leapfrog
	// past our parent class back to the default behavior.  Ug...
	AABoxClass box;
	Model->Get_Obj_Space_Bounding_Box(box);
	float xysize2 = box.Extent.X * box.Extent.X + box.Extent.Y * box.Extent.Y;
	if (xysize2 < MAX_XYSIZE2) {
		PhysClass::Update_Sun_Status();	
	} else {
		Set_Flag(IS_IN_THE_SUN,true);
	}
}

void StaticAnimPhysClass::Set_Shadow(TexProjectClass * shadow)
{
	if (ShadowProjector != NULL) {
		PhysicsSceneClass::Get_Instance()->Remove_Static_Texture_Projector(ShadowProjector);
		ShadowProjector->Release_Ref();
		ShadowProjector = NULL;
	}

	ShadowProjector = shadow;

	if (ShadowProjector != NULL) {
		ShadowProjector->Add_Ref();
		ShadowProjector->Enable_Affect_Static_Objects(false);
		PhysicsSceneClass::Get_Instance()->Add_Static_Texture_Projector(ShadowProjector);
	}
}


bool StaticAnimPhysClass::Needs_Timestep(void)
{ 
	return true; 
}

void StaticAnimPhysClass::Timestep(float dt)
{
	WWPROFILE("StaticAnim::Timestep");

	/*
	** Let the animation manager handle progressing the animation, checking
	** for collisions, and managing any "riders"
	*/
	AnimManager.Timestep(dt);

	/*
	** update projectors
	*/
	ProjectorManager.Update_From_Model(Model);
}


bool StaticAnimPhysClass::Internal_Link_Rider(PhysClass * rider)	
{
	AnimManager.Link_Rider(rider); 
	return true; 
}


bool StaticAnimPhysClass::Internal_Unlink_Rider(PhysClass * rider)
{ 
	AnimManager.Unlink_Rider(rider); 
	return true; 
}

void StaticAnimPhysClass::Save_State(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(STATICANIMPHYS_CHUNK_ANIMMANAGER);
	AnimManager.Save(csave);
	csave.End_Chunk();
}

void StaticAnimPhysClass::Load_State(ChunkLoadClass & cload)
{
	while (cload.Open_Chunk()) {
			
		switch(cload.Cur_Chunk_ID()) 
		{
			case STATICANIMPHYS_CHUNK_ANIMMANAGER:
				AnimManager.Load(cload);
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		cload.Close_Chunk();
	}
}

const PersistFactoryClass & StaticAnimPhysClass::Get_Factory(void) const
{
	return _StaticAnimPhysFactory;
}

bool StaticAnimPhysClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(STATICANIMPHYS_CHUNK_STATICPHYS);
	StaticPhysClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(STATICANIMPHYS_CHUNK_ANIMMANAGER);
	AnimManager.Save(csave);
	csave.End_Chunk();
	
	return true;
}


bool StaticAnimPhysClass::Load(ChunkLoadClass &cload)
{
	int legacy_collision_mode = -1;

	/*
	** Read in the chunks from the file
	*/
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case STATICANIMPHYS_CHUNK_STATICPHYS:
				StaticPhysClass::Load(cload);
				break;

			case STATICANIMPHYS_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,STATICANIMPHYS_VARIABLE_COLLISIONMODE,legacy_collision_mode);
					}
					cload.Close_Micro_Chunk();	
				}
				break;
				
			case STATICANIMPHYS_CHUNK_ANIMMANAGER:
				AnimManager.Load(cload);
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		
		cload.Close_Chunk();
	}

	if (legacy_collision_mode != -1) {
		AnimManager.Set_Collision_Mode((AnimCollisionManagerClass::CollisionModeType)legacy_collision_mode);
	}

	SaveLoadSystemClass::Register_Post_Load_Callback(this);
	return true;
}

void StaticAnimPhysClass::On_Post_Load(void)
{
	StaticPhysClass::On_Post_Load();

	/*
	** Always re-initialize any variables which depend on the model
	*/
	Update_Cached_Model_Parameters();

	/*
	** Re-Initialize our projector.
	*/
	StaticAnimPhysDefClass * definition = Get_StaticAnimPhysDef();
	if (definition != NULL) {
		ProjectorManager.Init(definition->ProjectorManagerDef,Model);
	}

}




/********************************************************************************************
**
** StaticAnimPhysDefClass Implementation
**
** Virtual constructor mechanism for StaticAnimPhysClass's and also the hook into the editor
**
********************************************************************************************/

/*
** Persist factory for StaticAnimPhysDefClass's
*/
SimplePersistFactoryClass<StaticAnimPhysDefClass,PHYSICS_CHUNKID_STATICANIMPHYSDEF>	_StaticAnimPhysDefFactory;

/*
** Definition factory for StaticAnimPhysDefClass.  This makes it show up in the editor
*/
DECLARE_DEFINITION_FACTORY(StaticAnimPhysDefClass, CLASSID_STATICANIMPHYSDEF, "StaticAnimPhys") _StaticAnimPhysDefDefFactory;

/*
** Chunk ID's used by StaticAnimPhysDefClass
*/
enum 
{
	STATICANIMPHYSDEF_CHUNK_STATICPHYSDEF				= 0x055110100,			// (parent class)
	STATICANIMPHYSDEF_CHUNK_PROJECTORMANAGERDEF,
	STATICANIMPHYSDEF_CHUNK_VARIABLES,
	STATICANIMPHYSDEF_CHUNK_ANIMMANAGERDEF,

	STATICANIMPHYSDEF_VARIABLE_COLLISIONMODE			= 0x00,
	STATICANIMPHYSDEF_VARIABLE_SHADOWDYNAMICOBJS,
	STATICANIMPHYSDEF_VARIABLE_SHADOWISADDITIVE,
	STATICANIMPHYSDEF_VARIABLE_SHADOWIGNORESZROTATION,
	STATICANIMPHYSDEF_VARIABLE_SHADOWFARZ,
	STATICANIMPHYSDEF_VARIABLE_SHADOWINTENSITY,
	STATICANIMPHYSDEF_VARIABLE_SHADOWNEARZ,
	STATICANIMPHYSDEF_VARIABLE_ANIMATIONNAME,
	STATICANIMPHYSDEF_VARIABLE_COLLIDEINPATHFIND,
	STATICANIMPHYSDEF_VARIABLE_ISCOSMETIC,
};


StaticAnimPhysDefClass::StaticAnimPhysDefClass(void) :
	ShadowDynamicObjs(false),
	ShadowIsAdditive(false),
	ShadowIgnoresZRotation(true),
	ShadowNearZ(0.5f),
	ShadowFarZ(5.0f),
	ShadowIntensity(0.5f),
	DoesCollideInPathfind(false),
	IsCosmetic(false)
{
	// Add the misc flags to the editable interface
	EDITABLE_PARAM(StaticAnimPhysDefClass, ParameterClass::TYPE_BOOL, IsCosmetic);
	EDITABLE_PARAM(StaticAnimPhysDefClass, ParameterClass::TYPE_BOOL, DoesCollideInPathfind);

	// Make the animation manager variables editable
	ANIMCOLLISIONMANAGERDEF_EDITABLE_PARAMS( StaticAnimPhysDefClass , AnimManagerDef );

	// Make the projector manager variables editable
	PROJECTORMANAGERDEF_EDITABLE_PARAMS( StaticAnimPhysDefClass , ProjectorManagerDef );

	// make the shadow parameters editable
  	PARAM_SEPARATOR(StaticAnimPhysDefClass, "Shadow Settings");
	EDITABLE_PARAM(StaticAnimPhysDefClass, ParameterClass::TYPE_BOOL, ShadowDynamicObjs);
	EDITABLE_PARAM(StaticAnimPhysDefClass, ParameterClass::TYPE_BOOL, ShadowIsAdditive);
	EDITABLE_PARAM(StaticAnimPhysDefClass, ParameterClass::TYPE_BOOL, ShadowIgnoresZRotation);
	FLOAT_EDITABLE_PARAM(StaticAnimPhysDefClass, ShadowNearZ, 1.0f, 100.0f);
	FLOAT_EDITABLE_PARAM(StaticAnimPhysDefClass, ShadowFarZ, 1.0f, 100.0f);
	FLOAT_EDITABLE_PARAM(StaticAnimPhysDefClass, ShadowIntensity, 0.0f, 1.0f);

}

uint32 StaticAnimPhysDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_STATICANIMPHYSDEF; 
}

PersistClass * StaticAnimPhysDefClass::Create(void) const
{
	StaticAnimPhysClass * obj = NEW_REF(StaticAnimPhysClass,());
	obj->Init(*this);
	return obj;
}

const PersistFactoryClass & StaticAnimPhysDefClass::Get_Factory (void) const
{
	return _StaticAnimPhysDefFactory;
}

bool StaticAnimPhysDefClass::Is_Type(const char * type_name)
{
	if (stricmp(type_name,StaticAnimPhysDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return StaticPhysDefClass::Is_Type(type_name);
	}
}


bool StaticAnimPhysDefClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(STATICANIMPHYSDEF_CHUNK_STATICPHYSDEF);
	StaticPhysDefClass::Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(STATICANIMPHYSDEF_CHUNK_PROJECTORMANAGERDEF);
	ProjectorManagerDef.Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(STATICANIMPHYSDEF_CHUNK_ANIMMANAGERDEF);
	AnimManagerDef.Save(csave);
	csave.End_Chunk();

	csave.Begin_Chunk(STATICANIMPHYSDEF_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,STATICANIMPHYSDEF_VARIABLE_SHADOWDYNAMICOBJS,ShadowDynamicObjs);
	WRITE_MICRO_CHUNK(csave,STATICANIMPHYSDEF_VARIABLE_SHADOWISADDITIVE,ShadowIsAdditive);
	WRITE_MICRO_CHUNK(csave,STATICANIMPHYSDEF_VARIABLE_SHADOWIGNORESZROTATION,ShadowIgnoresZRotation);
	WRITE_MICRO_CHUNK(csave,STATICANIMPHYSDEF_VARIABLE_SHADOWFARZ,ShadowFarZ);
	WRITE_MICRO_CHUNK(csave,STATICANIMPHYSDEF_VARIABLE_SHADOWINTENSITY,ShadowIntensity);
	WRITE_MICRO_CHUNK(csave,STATICANIMPHYSDEF_VARIABLE_SHADOWNEARZ,ShadowNearZ);
	WRITE_MICRO_CHUNK(csave,STATICANIMPHYSDEF_VARIABLE_COLLIDEINPATHFIND,DoesCollideInPathfind);
	WRITE_MICRO_CHUNK(csave,STATICANIMPHYSDEF_VARIABLE_ISCOSMETIC,IsCosmetic);
	csave.End_Chunk();

	return true;
}

bool StaticAnimPhysDefClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {

		switch(cload.Cur_Chunk_ID()) {

			case STATICANIMPHYSDEF_CHUNK_STATICPHYSDEF:
				StaticPhysDefClass::Load(cload);
				break;

			case STATICANIMPHYSDEF_CHUNK_PROJECTORMANAGERDEF:
				ProjectorManagerDef.Load(cload);
				break;

			case STATICANIMPHYSDEF_CHUNK_ANIMMANAGERDEF:
				AnimManagerDef.Load(cload);
				break;

			case STATICANIMPHYSDEF_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						// NOTE: these two variables/microchunks have been moved into the animation manager.
						READ_MICRO_CHUNK(cload,STATICANIMPHYSDEF_VARIABLE_COLLISIONMODE,AnimManagerDef.CollisionMode);	
						READ_MICRO_CHUNK_WWSTRING(cload,STATICANIMPHYSDEF_VARIABLE_ANIMATIONNAME,AnimManagerDef.AnimationName);	

						READ_MICRO_CHUNK(cload,STATICANIMPHYSDEF_VARIABLE_SHADOWDYNAMICOBJS,ShadowDynamicObjs);
						READ_MICRO_CHUNK(cload,STATICANIMPHYSDEF_VARIABLE_SHADOWISADDITIVE,ShadowIsAdditive);
						READ_MICRO_CHUNK(cload,STATICANIMPHYSDEF_VARIABLE_SHADOWIGNORESZROTATION,ShadowIgnoresZRotation);
						READ_MICRO_CHUNK(cload,STATICANIMPHYSDEF_VARIABLE_SHADOWFARZ,ShadowFarZ);
						READ_MICRO_CHUNK(cload,STATICANIMPHYSDEF_VARIABLE_SHADOWINTENSITY,ShadowIntensity);
						READ_MICRO_CHUNK(cload,STATICANIMPHYSDEF_VARIABLE_SHADOWNEARZ,ShadowNearZ);
						READ_MICRO_CHUNK(cload,STATICANIMPHYSDEF_VARIABLE_COLLIDEINPATHFIND,DoesCollideInPathfind);
						READ_MICRO_CHUNK(cload,STATICANIMPHYSDEF_VARIABLE_ISCOSMETIC,IsCosmetic);
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

