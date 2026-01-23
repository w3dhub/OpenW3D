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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/scriptcommands.cpp                    $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/16/02 8:37p                                               $*
 *                                                                                             *
 *                    $Revision:: 285                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "scriptcommands.h"
#include "debug.h"
#include "combat.h"
#include "smartgameobj.h"
#include "damage.h"
#include "gameobjmanager.h"
#include "crandom.h"
#include "scriptman.h"
#include "objlibrary.h"
#include "phys.h"
#include "ccamera.h"
#include "Sound3D.h"
#include "combat.h"
#include "radar.h"
#include "explosion.h"
#include "powerup.h"
#include "soldier.h"
#include "weapons.h"
#include "pscene.h"
#include "staticanimphys.h"
#include "hanim.h"
#include "objectives.h"
#include "spawn.h"
#include "assets.h"
#include "widestring.h"
#include "translatedb.h"
#include "vehicle.h"
#include "combatchunkid.h"
#include "LogicalSound.h"
#include "soldierobserver.h"
#include "cinematicgameobj.h"
#include "dynamicanimphys.h"
#include <stdio.h>
#include "conversationmgr.h"
#include "activeconversation.h"
#include "orator.h"
#include "gameobjobserver.h"
#include "animcontrol.h"
#include "playerdata.h"
#include "building.h"
#include "scriptzone.h"
#include "hud.h"
#include "backgroundmgr.h"
#include "WeatherMgr.h"
#include "mapmgr.h"
#include "heightdb.h"
#include "weaponbag.h"
#include "ffactory.h"
#include "playerterminal.h"
#include "hudinfo.h"
#include "globalsettings.h"
#include "screenfademanager.h"


#define	SCRIPT_TRACE(x)	if (ScriptTrace) {Debug_Say(x);}
bool		ScriptTrace	= false;

#define	SCRIPT_PTR_CHECK( x )				if ( x == NULL ) { Debug_Say(( "NULL Script Ptr at %s line %d\n", __FILE__, __LINE__ )); return;	}
#define	SCRIPT_PTR_CHECK_RET( x, ret )	if ( x == NULL ) { Debug_Say(( "NULL Script Ptr at %s line %d\n", __FILE__, __LINE__ )); return ret;	}

char	DamageModelName[ 30 ];	// HACK
bool	DamageModelDirection;		// HACK

#define	SCRIPT_PRIORITY	2

/*
** Debug messages
*/
void	Debug_Message( [[maybe_unused]] const char *format, ... )
{
#ifdef WWDEBUG
	SCRIPT_PTR_CHECK( format );
	va_list arg_list;
	va_start (arg_list, format);
	StringClass string(true);
	string.Format_Args( format, arg_list );
	va_end (arg_list);
	Debug_Script(( (const char *)string ));
#endif
}

void Modify_Action( GameObject * obj, int action_id, const ActionParamsStruct & params, bool modify_move, bool modify_attack )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Modify_Action( %d, %d )\n", obj->Get_ID(), action_id ));

	SmartGameObj * smart = obj->As_SmartGameObj();
	if (	smart != NULL && smart->Get_Action() != NULL &&
			smart->Get_Action()->Get_Parameters().ActionID == action_id )
	{
		smart->Get_Action()->Modify( params, modify_move, modify_attack );
	}

	return ;
}


int	Get_Action_ID( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, 0 );
	SCRIPT_TRACE((	"ST>Get_Action_ID( %d )\n", obj->Get_ID() ));

	int retval = 0;

	//
	//	Simply return the ID of the current action
	//
	SmartGameObj * smart = obj->As_SmartGameObj();
	if (	smart != NULL && smart->Get_Action() != NULL ) {
		retval = smart->Get_Action()->Get_Parameters().ActionID;
	}

	return retval;
}

bool	Get_Action_Params( GameObject * obj, ActionParamsStruct & params )
{
	SCRIPT_PTR_CHECK_RET( obj, false );
	SCRIPT_TRACE((	"ST>Get_Action_Params( %d )\n", obj->Get_ID() ));

	bool retval = false;

	//
	//	Simply copy the parameters to the provided structure
	//
	SmartGameObj * smart = obj->As_SmartGameObj();
	if (	smart != NULL && smart->Get_Action() != NULL ) {
		params					= smart->Get_Action()->Get_Parameters();
		params.MoveObject		= smart->Get_Action()->Get_Parameters().MoveObjectRef;
		params.LookObject		= smart->Get_Action()->Get_Parameters().LookObjectRef;
		params.AttackObject	= smart->Get_Action()->Get_Parameters().AttackObjectRef;
		retval = true;
	}

	return retval;
}

bool	Is_Performing_Pathfind_Action( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, false );
	SCRIPT_TRACE((	"ST>Is_Performing_Pathfind_Action( %d )\n", obj->Get_ID() ));

	bool retval = false;

	//
	//	Simply ask the action if its "busy", this means its doing something that
	// can't be interrupted
	//
	SmartGameObj * smart = obj->As_SmartGameObj();
	if (	smart != NULL && smart->Get_Action() != NULL ) {
		smart->Get_Action()->Is_Busy();
	}

	return retval;
}


void Action_Reset( GameObject * obj, float priority )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Action_Reset( %d, %f )\n", obj->Get_ID(), priority ));

	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart ) {
		smart->Get_Action()->Reset( priority );
	}
}

void Action_Goto( GameObject * obj, const ActionParamsStruct & params )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Action_Goto( %d )\n", obj->Get_ID() ));

	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart ) {
		smart->Get_Action()->Goto( params );
	}
}

void Action_Attack( GameObject * obj, const ActionParamsStruct & params )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Action_Attack( %d )\n", obj->Get_ID() ));

	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart ) {
		smart->Get_Action()->Attack( params );
	}
}

void Action_Play_Animation( GameObject * obj, const ActionParamsStruct & params )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Action_Play_Animation( %d )\n", obj->Get_ID() ));

	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart ) {
		smart->Get_Action()->Play_Animation( params );
	}
}

void Action_Enter_Exit( GameObject * obj, const ActionParamsStruct & params )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Action_Enter_Exit( %d )\n", obj->Get_ID() ));

	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart ) {
		smart->Get_Action()->Enter_Exit( params );
	}
}


void Action_Face_Location( GameObject * obj, const ActionParamsStruct & params )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Action_Face_Location( %d )\n", obj->Get_ID() ));

	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart ) {
		smart->Get_Action()->Face_Location( params );
	}
}


void Action_Dock( GameObject * obj, const ActionParamsStruct & params )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Action_Dock( %d )\n", obj->Get_ID() ));

	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart ) {
		smart->Get_Action()->Dock_Vehicle( params );
	}
}


void Action_Follow_Input( GameObject * obj, const ActionParamsStruct & params )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Action_Follow_Input( %d )\n", obj->Get_ID() ));

	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart ) {
		smart->Get_Action()->Follow_Input( params );
	}
}



/*
** Physical control
*/
void	Set_Position( GameObject * obj, const Vector3 & position )
{
	SCRIPT_PTR_CHECK( obj );

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}

	SCRIPT_TRACE((	"ST>Set_Position( %d, (%f ,%f,%f) )\n", pgobj->Get_ID(), position[0], position[1], position[2] ));
	pgobj->Peek_Physical_Object()->Set_Position( position );
}

Vector3 Get_Position( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, Vector3(0,0,0) );

	Vector3 pos;
	obj->Get_Position(&pos);
	return pos;
}

Vector3 Get_Bone_Position( GameObject * obj, const char * bone_name )
{
	SCRIPT_PTR_CHECK_RET( obj, Vector3(0,0,0) );

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return Vector3(0,0,0);
	}

	SCRIPT_PTR_CHECK_RET( pgobj->Peek_Model(), Vector3(0,0,0) );
	Matrix3D tm = pgobj->Peek_Model()->Get_Bone_Transform( bone_name );
	return tm.Get_Translation();
}

float Get_Facing( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, 0 );

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return 0;
	}

	return RAD_TO_DEG( pgobj->Get_Facing() );
}

void	Set_Facing( GameObject * obj, float degrees )
{
	SCRIPT_PTR_CHECK( obj );

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}


	SCRIPT_TRACE((	"ST>Set_Facing( %d, %f )\n", pgobj->Get_ID(), degrees ));
	Vector3 pos;
	pgobj->Get_Position(&pos);

	Matrix3D tm(1);
	tm.Translate(pos);
	tm.Rotate_Z( DEG_TO_RAD( degrees ) );

	pgobj->Set_Transform(tm);
}


/*
** Collision Control
*/
void	Disable_All_Collisions( GameObject * obj )
{
	SCRIPT_PTR_CHECK( obj );

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}

	SCRIPT_TRACE((	"ST>Disable_All_Collisions( %d )\n", pgobj->Get_ID() ));
	pgobj->Set_Collision_Group( UNCOLLIDEABLE_GROUP );
}

void	Disable_Physical_Collisions( GameObject * obj )
{
	SCRIPT_PTR_CHECK( obj );

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}

	SCRIPT_TRACE((	"ST>Disable_Physical_Collisions( %d )\n", pgobj->Get_ID() ));
	pgobj->Set_Collision_Group( BULLET_ONLY_COLLISION_GROUP );
}

void	Enable_Collisions( GameObject * obj )
{
	SCRIPT_PTR_CHECK( obj );

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}

	SCRIPT_TRACE((	"ST>Enable_Collisions( %d )\n", pgobj->Get_ID() ));
	if ( pgobj->As_SoldierGameObj () != NULL ) {
		pgobj->Set_Collision_Group( SOLDIER_COLLISION_GROUP );
	} else {
		pgobj->Set_Collision_Group( DEFAULT_COLLISION_GROUP );
	}
}

/*
** Object Management
*/
void	Destroy_Object( GameObject * obj )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Destroy_Object( %d )\n", obj->Get_ID() ));
	obj->Set_Delete_Pending();
}


GameObject * Find_Object( int obj_id )
{
	return GameObjManager::Find_ScriptableGameObj( obj_id );
}

int	Get_ID( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, 0 );
	return obj->Get_ID();
}

int	Get_Preset_ID( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, 0 );
	return obj->Get_Definition ().Get_ID ();
}

const char * Get_Preset_Name( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, "" );
	return obj->Get_Definition ().Get_Name ();
}




/******************************************************************************
*
* NAME
*     Create_Object
*
* DESCRIPTION
*     Create an object with the specified transform.
*
* INPUTS
*     const char* name
*     const Matrix3D& tm
*
* RESULTS
*     GameObject*
*
******************************************************************************/

GameObject* Create_Object(const char* name, const Matrix3D& tm)
{
	assert(name != NULL);
	GameObject* object = ObjectLibraryManager::Create_Object(name);

	if (object != NULL) {
		WWASSERT( object->As_PhysicalGameObj() );
		object->As_PhysicalGameObj()->Set_Transform(tm);
		object->Start_Observers();
	}

	return object;
}


GameObject* Create_Object(const char* type_name, const Vector3 & position)
{
	SCRIPT_PTR_CHECK_RET(type_name, NULL);
	SCRIPT_TRACE((	"ST>Create_Object( %s (%f,%f,%f) )\n",
		type_name, position[0], position[1], position[2] ));

#if(0) // Denzil
	GameObject* obj = ObjectLibraryManager::Create_Object(type_name);

	if (obj != NULL) {
		Matrix3D tm(1);
		tm.Set_Translation(position);
		obj->Set_Transform(tm);
	}

	return obj;
#else
	Matrix3D tm(true);
	tm.Set_Translation(position);
	return Create_Object(type_name, tm);
#endif
}


GameObject* Create_Object_At_Bone(GameObject* host_obj, const char* new_obj_type_name, const char* bone_name)
{
	SCRIPT_PTR_CHECK_RET( host_obj, NULL );
	SCRIPT_PTR_CHECK_RET( new_obj_type_name, NULL );
	SCRIPT_PTR_CHECK_RET( bone_name, NULL );

	PhysicalGameObj * pgobj = host_obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return NULL;
	}

	SCRIPT_TRACE((	"ST>Create_Object_At_Bone( %d %s %s )\n", host_obj->Get_ID(), new_obj_type_name, bone_name ));

#if(0) // Denzil
	GameObject* obj = ObjectLibraryManager::Create_Object(new_obj_type_name);

	if (obj != NULL) {
		obj->Set_Transform(host_obj->Peek_Model()->Get_Bone_Transform(bone_name));
	}

	return obj;
#else

	Matrix3D tm = pgobj->Peek_Model()->Get_Bone_Transform(bone_name);
	return Create_Object(new_obj_type_name, tm);
#endif
}


// Attach a script to and object.
void Attach_Script(GameObject* object, const char* scriptName, const char* scriptParams)
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_PTR_CHECK( scriptName );
	// Create a script instance
	assert(scriptName != NULL);
	ScriptClass* script = ScriptManager::Create_Script(scriptName);

	if (script != NULL) {

		// Set the scripts parameters
		assert(scriptParams != NULL);
		script->Set_Parameters_String(scriptParams);

		// Add the script to the object
		assert(object != NULL);
		object->Add_Observer(script);

	} else {

		WWDEBUG_SAY(("Unable to create script: %s\r\n",scriptName));
	}
}

void Add_To_Dirty_Cull_List(GameObject* obj)
{
	SCRIPT_PTR_CHECK( obj );
	PhysicalGameObj *physobj = obj->As_PhysicalGameObj();
	if ( physobj != NULL ) {
		COMBAT_SCENE->Add_To_Dirty_Cull_List( physobj->Peek_Physical_Object() );
	}
}


/*
** Timers
*/
void	Start_Timer( GameObject * obj, ScriptClass * script, float duration, int timer_id )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_PTR_CHECK( script );
	SCRIPT_TRACE((	"ST>Start_Timer( %d, %d, %f, %d )\n", obj->Get_ID(), script->Get_ID(), duration, timer_id ));
	obj->Start_Observer_Timer( script->Get_ID(), duration, timer_id );
}



/*
** Weapons
*/
void	Trigger_Weapon( GameObject * obj, bool trigger, const Vector3 & target, bool primary_fire )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Trigger_Weapon( %d, %d, (%f %f %f ) %d\n", obj->Get_ID(), trigger, target.X, target.Y, target.Z, primary_fire ));
	PhysicalGameObj *phyobj = obj->As_PhysicalGameObj();
	if ( phyobj != NULL ) {
		ArmedGameObj *armed = phyobj->As_ArmedGameObj();
		if (armed) {
			WeaponClass * weapon = armed->Get_Weapon();

			if ( weapon != NULL ) {
				armed->Set_Targeting( target );
				if ( primary_fire ) {
					weapon->Set_Primary_Triggered( trigger );
				} else {
					weapon->Set_Secondary_Triggered( trigger );
				}
			} else {
				Debug_Say(( "This object does not have this weapon\n" ));
			}
		} else {
			Debug_Say(( "This object cannot fire weapons\n" ));
		}
	} else {
		Debug_Say(( "This object cannot fire weapons\n" ));
	}
}

void	Select_Weapon( GameObject * obj, const char * weapon_name )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Select_Weapon( %d, %s\n", obj->Get_ID(), weapon_name ));
	PhysicalGameObj *phyobj = obj->As_PhysicalGameObj();
	if ( phyobj != NULL ) {
		ArmedGameObj *armed = phyobj->As_ArmedGameObj();
		if (armed) {
			armed->Get_Weapon_Bag()->Select_Weapon_Name( weapon_name );
		} else {
			Debug_Say(( "This object cannot fire weapons\n" ));
		}
	} else {
		Debug_Say(( "This object cannot fire weapons\n" ));
	}
}


/*
**
*/
void	Send_Custom_Event( GameObject * from, GameObject * to, int type, intptr_t param, float delay )
{
#if(0) // Denzil 4/4/00 - From not required for most messages
	SCRIPT_PTR_CHECK( from );
	SCRIPT_PTR_CHECK( to );  // ?? This may be okay to be null
	SCRIPT_TRACE((	"ST>Send_Custom_Event( %d, %d %d, %d )\n", from->Get_ID(), to->Get_ID(), type, param ));
#else
	SCRIPT_PTR_CHECK( to );
	SCRIPT_TRACE(("ST>Send_Custom_Event( %d, %d %d, %d )\n",
		((from != NULL) ? from->Get_ID() : 0), to->Get_ID(), type, param));
#endif

	WWASSERT( type < CUSTOM_EVENT_SYSTEM_FIRST );

	if ( to ) {
		if ( delay <= 0 ) {
			const GameObjObserverList & observer_list = to->Get_Observers();
			for( int index = 0; index < observer_list.Count(); index++ ) {
				observer_list[ index ]->Custom( to, type, param, from );
			}
		} else {
			to->Start_Custom_Timer( from, delay, type, param );
		}
	}
}

void	Send_Damaged_Event( GameObject * object, GameObject * damager )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE(("ST>Send_Damaged_Event( %d, %d )\n",
		object->Get_ID(), ((damager != NULL) ? damager->Get_ID() : 0) ));

	if ( object ) {
		const GameObjObserverList & observer_list = object->Get_Observers();
		for( int index = 0; index < observer_list.Count(); index++ ) {
			observer_list[ index ]->Damaged( object, damager, 0 );
		}
	}
}


/*
**
*/
float	Get_Random( float min, float max )
{
	return FreeRandom.Get_Float( min, max );
}


int	Get_Random_Int( int min, int max )
{
	return FreeRandom.Get_Int( min, max );
}


/*
**	Find_Random_Simple_Object
*/
GameObject *Find_Random_Simple_Object ( const char *preset_name )
{
	SCRIPT_TRACE((	"ST>Find_Random_Simple_Object( %s )\n", preset_name ));

	GameObject *retval = NULL;
	DynamicVectorClass<SimpleGameObj *> obj_list;

	//
	//	Build a list of all the simple game objects that match the criteria
	//
	SLNode<BaseGameObj> *node = NULL;
	for (node = GameObjManager::Get_Game_Obj_List()->Head(); node; node = node->Next()) {
		WWASSERT(node->Data() != NULL);

		//
		//	Is this a simple game obj?
		//
		PhysicalGameObj *physical_obj = node->Data()->As_PhysicalGameObj();
		if ( physical_obj != NULL ) {
			SimpleGameObj *simple_obj = physical_obj->As_SimpleGameObj();
			if ( simple_obj != NULL ) {

				//
				//	Is this one of the objects we can choose from?
				//
				if ( ::stricmp ( simple_obj->Get_Definition ().Get_Name (), preset_name ) == 0 ) {
					obj_list.Add (simple_obj);
				}
			}
		}
	}

	//
	//	Index randomly into the object list
	//
	int count = obj_list.Count ();
	if ( count > 0 ) {
		int random_index = FreeRandom.Get_Int( count );
		retval = obj_list[random_index];
	}

	return retval;
}


/*
** Object Display
*/
void	Set_Model( GameObject * obj, const char * model_name )
{
	SCRIPT_PTR_CHECK( obj );

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}

	if ( obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_SimpleGameObj() ) {
		if ( obj->As_PhysicalGameObj()->As_SimpleGameObj()->Get_Definition().Get_Is_Editor_Object() ) {
			Debug_Say(( "Can't Set_Model on a IsEditorObject %s %s\n", obj->As_PhysicalGameObj()->As_SimpleGameObj()->Get_Definition().Get_Name(), model_name ));
//			WWASSERT(0);
		}
	}

//	Matrix3D tm = pgobj->Get_Transform();
	SCRIPT_TRACE((	"ST>Set_Model( %d, %s, %d )\n", obj->Get_ID(), model_name == NULL ? "NULL" : model_name ));
	if (pgobj->As_SoldierGameObj() ) {
		// For soldiers, call Set_Model, so the anim control will be updated
		pgobj->As_SoldierGameObj()->Set_Model( model_name );
	} else {
		pgobj->Peek_Physical_Object()->Set_Model_By_Name( model_name );
		if ( pgobj->Get_Anim_Control() != NULL ) {
			pgobj->Get_Anim_Control()->Set_Model( pgobj->Peek_Model() );
		}
	}

	pgobj->Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true);

	pgobj->Hide_Muzzle_Flashes();				// Cinematics need this
	return ;
}

void	Set_Animation( GameObject * obj, const char * anim_name, bool looping, const char * sub_obj_name, float start_frame, float end_frame, bool is_blended )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Set_Animation( %d, %s, %d )\n", obj->Get_ID(), anim_name == NULL ? "NULL" : anim_name, looping ));

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}

	// check that this object is not in Action_Play_Animation
	if ( pgobj->As_SmartGameObj() != NULL ) {
		if ( pgobj->As_SmartGameObj()->Get_Action()->Is_Animating() ) {
			Debug_Say(( "Can't Set_Animation when Action_Play_Animationing\n" ));
			WWASSERT( 0 );
		}
	}

	if ( ( sub_obj_name != NULL ) && ( *sub_obj_name != 0 ) ) {

		RenderObjClass * model = pgobj->Peek_Model();
		RenderObjClass * sub = model->Get_Sub_Object_By_Name( sub_obj_name );

		if ( sub != NULL ) {
			HAnimClass * anim = WW3DAssetManager::Get_Instance()->Get_HAnim( anim_name );
			if ( anim != NULL ) {
				sub->Set_Animation( anim, start_frame, looping ? RenderObjClass::ANIM_MODE_LOOP : RenderObjClass::ANIM_MODE_ONCE );
				anim->Release_Ref();
			}
			sub->Release_Ref();
		}

	} else {
		CinematicGameObj * cinobj = pgobj->As_CinematicGameObj();
		if (cinobj != NULL) {

			// CinematicGameObj's require you to use the physics animation interface
			DynamicAnimPhysClass * dynanim = cinobj->Peek_Physical_Object()->As_DynamicAnimPhysClass();
			if (dynanim != NULL) {

				AnimCollisionManagerClass & anim_mgr = dynanim->Get_Animation_Manager();

				if (anim_name != NULL) {
					anim_mgr.Set_Animation(anim_name);
				}
				anim_mgr.Set_Current_Frame( start_frame );

				if (looping) {
					anim_mgr.Set_Animation_Mode(AnimCollisionManagerClass::ANIMATE_LOOP);
				} else {
					anim_mgr.Set_Animation_Mode(AnimCollisionManagerClass::ANIMATE_TARGET);
					anim_mgr.Set_Target_Frame_End();
				}

				//
				//	Specify the target frame (if necessary)
				//
				if ( end_frame >= 0 ) {
					anim_mgr.Set_Target_Frame( end_frame );
				}

				//
				//	"Dirty" the object for networking
				//
				cinobj->Set_Object_Dirty_Bit( NetworkObjectClass::BIT_RARE, true );

			} else {
				WWDEBUG_SAY(("Error! cinematic game obj %s not using DynamicAnimPhys.\r\n"));
			}

		} else {

			//
			//	Soldier's can blend their animation, so if the game object is
			// a soldier and the blend flag is set, then blend the animation.
			//
			SoldierGameObj *soldier = pgobj->As_SoldierGameObj();
			if ( soldier != NULL && is_blended ) {
				soldier->Set_Blended_Animation( anim_name, looping, start_frame );
			} else {
				pgobj->Set_Animation( anim_name, looping, start_frame );
			}

			//
			//	If the anim_name is the same as the current anim, then the frame_offset isn't set, so
			// force it.
			//
			AnimControlClass *anim_control = pgobj->Get_Anim_Control();
			if (anim_control != NULL) {
				anim_control->Set_Mode( looping ? ANIM_MODE_LOOP : ANIM_MODE_ONCE, start_frame );

				//
				//	Specify the target frame (if necessary)
				//
				if ( end_frame >= 0 ) {
					anim_control->Set_Mode( ANIM_MODE_TARGET, start_frame );
					anim_control->Set_Target_Frame( end_frame );
				}
			}
		}
	}
}


void	Set_Animation_Frame( GameObject * obj, const char * anim_name, int frame )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Set_Animation_Frame( %d, %s, %d )\n", obj->Get_ID(), anim_name == NULL ? "NULL" : anim_name, frame ));

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}

	// check that this object is not in Action_Play_Animation
	if ( pgobj->As_SmartGameObj() != NULL ) {
		if ( pgobj->As_SmartGameObj()->Get_Action()->Is_Animating() ) {
			Debug_Say(( "Can't Set_Animation when Action_Play_Animationing\n" ));
			WWASSERT( 0 );
		}
	}

	pgobj->Set_Animation_Frame ( anim_name, frame );
}


/*
**
*/
int Create_Sound(const char* sound_name, const Vector3& position, GameObject* creator)
{
	SCRIPT_TRACE(("ST>Create_Sound( %s, (%f,%f,%f) )\n", sound_name, position[0], position[1], position[2]));

	Matrix3D matrix(true);
	matrix.Set_Translation(position);

	WWAudioClass* audio = WWAudioClass::Get_Instance();
	assert(audio != NULL);

	RefCountedGameObjReference *owner_ref = new RefCountedGameObjReference;
	owner_ref->Set_Ptr( creator );
	int sound_id = audio->Create_Instant_Sound(sound_name, matrix, owner_ref);
	REF_PTR_RELEASE( owner_ref );

	return sound_id;
}

int Create_2D_Sound( const char * sound_preset_name )
{
	WWAudioClass* audio = WWAudioClass::Get_Instance();
	assert(audio != NULL);

	//
	//	Create the sound object from its preset
	//
	int sound_id = 0;
	AudibleSoundClass *sound = audio->Create_Sound( sound_preset_name );
	if ( sound != NULL ) {

		sound_id = sound->Get_ID ();
		sound->Play();
		sound->Release_Ref();
	}

	return sound_id;
}

int Create_2D_WAV_Sound( const char * wav_filename )
{
	WWAudioClass* audio = WWAudioClass::Get_Instance();
	assert(audio != NULL);

	AudibleSoundClass * sound = audio->Create_Sound_Effect(wav_filename);

	int sound_id = 0;
	if ( sound != NULL ) {
		WWASSERT( sound->Get_Loop_Count() != 0 );
		sound_id = sound->Get_ID ();
		sound->Play();
		sound->Release_Ref();
	}

	return sound_id;
}


int Create_3D_Sound_At_Bone( const char * sound_preset_name, GameObject * obj, const char * bone_name )
{
	WWAudioClass* audio = WWAudioClass::Get_Instance();
	assert(audio != NULL);

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return 0;
	}

	//
	//	Create the sound object from its preset
	//
	int sound_id = 0;
	AudibleSoundClass *sound = audio->Create_Sound( sound_preset_name );
	if ( sound != NULL ) {

		//
		//	Attach the sound to the bone
		//
		RenderObjClass * model = pgobj->Peek_Model();
		int bone_index = model->Get_Bone_Index( bone_name );
		sound->Attach_To_Object( model, bone_index );

		//
		//	Add the sound to the world and release our hold on it
		//
		sound_id = sound->Get_ID ();
		sound->Add_To_Scene();
		REF_PTR_RELEASE( sound );
	}

	return sound_id;
}


int Create_3D_WAV_Sound_At_Bone( const char * wav_filename, GameObject * obj, const char * bone_name )
{
	WWAudioClass* audio = WWAudioClass::Get_Instance();
	assert(audio != NULL);

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return 0;
	}

	int sound_id = 0;
	Sound3DClass *	sound = audio->Create_3D_Sound( wav_filename );
	if ( sound ) {

		sound_id = sound->Get_ID ();
		sound->Set_Type ( AudibleSoundClass::TYPE_SOUND_EFFECT );
		sound->Set_Priority ( 1 );
		sound->Set_Volume ( 1 );
		sound->Set_Loop_Count ( 1 );
		sound->Set_DropOff_Radius ( 30 );
		sound->Set_Definition ( NULL );
		sound->Set_Max_Vol_Radius ( 30 );
		RenderObjClass * model = pgobj->Peek_Model();
		int bone_index = model->Get_Bone_Index( bone_name );
		sound->Attach_To_Object( model, bone_index );

		sound->Add_To_Scene();
		sound->Release_Ref();
	}

	return sound_id;
}

int Create_Logical_Sound( GameObject * creator, int type, const Vector3 & position, float radius )
{
	SCRIPT_TRACE(("ST>Create_Sound( %d, (%f,%f,%f) )\n", type, position[0], position[1], position[2]));

	int sound_id = 0;
	WWAudioClass* audio = WWAudioClass::Get_Instance();
	if ( audio != NULL ) {
		LogicalSoundClass *	sound = audio->Create_Logical_Sound();
		if ( sound != NULL ) {

			RefCountedGameObjReference *owner_ref = new RefCountedGameObjReference;
			owner_ref->Set_Ptr( creator );
			sound->Set_User_Data( owner_ref, 0 );
			REF_PTR_RELEASE( owner_ref );

			sound_id = sound->Get_ID ();
			sound->Set_Type_Mask( type );
			sound->Set_Notify_Delay( 0 );
			sound->Set_Single_Shot( true );
			sound->Set_DropOff_Radius( radius );
			sound->Set_Position( position );
			sound->Add_To_Scene ();
			sound->Release_Ref();
		}
	}

	return sound_id;
}

void Monitor_Sound( GameObject * game_obj, int sound_id )
{
	SCRIPT_TRACE(("ST>Monitor_Sound( %d, %d )\n", game_obj->Get_ID(), sound_id));

	WWAudioClass* audio = WWAudioClass::Get_Instance();
	if ( audio != NULL ) {

		//
		//	Lookup the sound object by its ID
		//
		SoundSceneObjClass *sound_obj = audio->Find_Sound_Object (sound_id);
		if ( sound_obj != NULL ) {

			//
			//	Register the game object as a callback for the sound object
			//
			sound_obj->Register_Callback( AudioCallbackClass::EVENT_SOUND_ENDED, game_obj );
		}
	}

	return ;
}


void Fade_Background_Music( const char * wav_filename, int fade_out_time, int fade_in_time )
{
	SCRIPT_TRACE(("ST>Fade_Background_Music( %s, %d, %d )\n", wav_filename, fade_out_time, fade_in_time));

	WWAudioClass* audio = WWAudioClass::Get_Instance();
	if ( audio != NULL ) {

		//
		//	Pass the background music onto the sound library
		//
		audio->Fade_Background_Music( wav_filename, fade_out_time, fade_in_time );
	}

	return ;
}


void Set_Background_Music( const char * wav_filename )
{
	SCRIPT_TRACE(("ST>Set_Background_Music( %s )\n", wav_filename));

	WWAudioClass* audio = WWAudioClass::Get_Instance();
	if ( audio != NULL ) {

		//
		//	Pass the background music onto the sound library
		//
		audio->Set_Background_Music( wav_filename );
	}

	return ;
}

void Stop_Background_Music( void )
{
	SCRIPT_TRACE(("ST>Stop_Background_Music( )\n"));

	WWAudioClass* audio = WWAudioClass::Get_Instance();
	if ( audio != NULL ) {

		//
		//	Simply stop the background music
		//
		audio->Set_Background_Music( NULL );
	}

	return ;
}



void Start_Sound( int sound_id )
{
	SCRIPT_TRACE(("ST>Stop_Sound( %d )\n", sound_id));

	WWAudioClass* audio = WWAudioClass::Get_Instance();
	if ( audio != NULL ) {

		//
		//	Lookup the sound object by its ID
		//
		SoundSceneObjClass *sound_obj = audio->Find_Sound_Object( sound_id );
		if ( sound_obj != NULL) {

			//
			//	Add the sound to the scene (if necessary)
			//
			if ( sound_obj->Is_In_Scene() == false ) {
				sound_obj->Add_To_Scene();
			}

			//
			//	Start playing the sound
			//
			AudibleSoundClass *audible_sound = 	sound_obj->As_AudibleSoundClass();
			if (audible_sound != NULL) {
				audible_sound->Play();
			}
		}
	}

	return ;
}


void Stop_Sound( int sound_id, bool destroy_sound )
{
	SCRIPT_TRACE(("ST>Stop_Sound( %d )\n", sound_id));

	WWAudioClass* audio = WWAudioClass::Get_Instance();
	if ( audio != NULL ) {

		//
		//	Lookup the sound object by its ID
		//
		SoundSceneObjClass *sound_obj = audio->Find_Sound_Object( sound_id );
		if ( sound_obj != NULL ) {

			//
			//	Stop playing the sound
			//
			AudibleSoundClass *audible_sound = 	sound_obj->As_AudibleSoundClass();
			if (audible_sound != NULL) {
				audible_sound->Stop();
			}

			//
			//	Remove the sound from the world (this will destroy the sound)
			//
			if ( destroy_sound ) {
				sound_obj->Remove_From_Scene();
			}
		}
	}

	return ;
}


/*
**
*/
float	Get_Health( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, 0 );

	DamageableGameObj * dgobj = obj->As_DamageableGameObj();
	if ( dgobj == NULL ) {
		Debug_Say(( "Not a DamageableGameObj at %s %d\n", __FILE__, __LINE__ ));
		return 0;
	}

	return dgobj->Get_Defense_Object()->Get_Health();
}

float	Get_Max_Health( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, 0 );

	DamageableGameObj * dgobj = obj->As_DamageableGameObj();
	if ( dgobj == NULL ) {
		Debug_Say(( "Not a DamageableGameObj at %s %d\n", __FILE__, __LINE__ ));
		return 0;
	}

	return dgobj->Get_Defense_Object()->Get_Health_Max();
}

void	Set_Health( GameObject * obj, float health )
{
	SCRIPT_PTR_CHECK( obj );

	DamageableGameObj * dgobj = obj->As_DamageableGameObj();
	if ( dgobj == NULL ) {
		Debug_Say(( "Not a DamageableGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}

	dgobj->Get_Defense_Object()->Set_Health( health );
}

float	Get_Shield_Strength( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, 0 );

	DamageableGameObj * dgobj = obj->As_DamageableGameObj();
	if ( dgobj == NULL ) {
		Debug_Say(( "Not a DamageableGameObj at %s %d\n", __FILE__, __LINE__ ));
		return 0;
	}

	return dgobj->Get_Defense_Object()->Get_Shield_Strength();
}

float	Get_Max_Shield_Strength( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, 0 );

	DamageableGameObj * dgobj = obj->As_DamageableGameObj();
	if ( dgobj == NULL ) {
		Debug_Say(( "Not a DamageableGameObj at %s %d\n", __FILE__, __LINE__ ));
		return 0;
	}

	return dgobj->Get_Defense_Object()->Get_Shield_Strength_Max();
}

void	Set_Shield_Strength( GameObject * obj, float strength )
{
	SCRIPT_PTR_CHECK( obj );

	DamageableGameObj * dgobj = obj->As_DamageableGameObj();
	if ( dgobj == NULL ) {
		Debug_Say(( "Not a DamageableGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}

	dgobj->Get_Defense_Object()->Set_Shield_Strength( strength );
}

void	Set_Shield_Type( GameObject * obj, const char * name )
{
	SCRIPT_PTR_CHECK( obj );

	DamageableGameObj * dgobj = obj->As_DamageableGameObj();
	if ( dgobj == NULL ) {
		Debug_Say(( "Not a DamageableGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}

	dgobj->Get_Defense_Object()->Set_Shield_Type( ArmorWarheadManager::Get_Armor_Type( name ) );
}


int	Get_Player_Type( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, 0 );

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return 0;
	}

	return	pgobj->Get_Player_Type();
}

void	Set_Player_Type( GameObject * obj, int type )
{
	SCRIPT_PTR_CHECK( obj );

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
	} else {
		pgobj->Set_Player_Type( type );
	}

}


/*
**
*/
float	Get_Distance( const Vector3 & p1, const Vector3 & p2 )
{
	Vector3 v = p1 - p2;
	return v.Length();
}


/*
**
*/
void	Set_Camera_Host( GameObject * obj )
{
	//SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Set_Camera_Host( %p )\n", obj ));
	if ( obj == NULL ) {
		COMBAT_CAMERA->Set_Host_Model( NULL );
	} else {
		PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
		if ( pgobj == NULL ) {
			Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
			return;
		}

		COMBAT_CAMERA->Set_Host_Model( pgobj->Peek_Model() );
	}
}

void	Force_Camera_Look( const Vector3 & target )
{
	//SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Force_Camera_Look( %f %f %f )\n", target.X, target.Y, target.Z ));
	if ( COMBAT_CAMERA ) {
		COMBAT_CAMERA->Force_Look( target );
	}
}


/*
**
**	Find_Closest_Soldier
**
*/
GameObject * Find_Closest_Soldier( const Vector3 & pos, float min_dist, float max_dist, bool only_human )
{
	AABoxClass box (pos, Vector3 (max_dist / 2, max_dist / 2, max_dist / 2));

	//
	//	Collect all the dynamic objects in this box
	//
	NonRefPhysListClass obj_list;
	PhysicsSceneClass::Get_Instance ()->Collect_Objects (box, false, true, &obj_list);

	float closest_dist		= max_dist;
	GameObject *closest_obj	= NULL;

	//
	//	Loop over all the collected objects
	//
	NonRefPhysListIterator it (&obj_list);
	for (it.First(); !it.Is_Done(); it.Next()) {
		PhysClass *phys_obj = it.Peek_Obj ();
		PhysicalGameObj *game_obj = NULL;

		if ( phys_obj->Get_Observer() != NULL ) {
			game_obj = ((CombatPhysObserverClass *)phys_obj->Get_Observer())->As_PhysicalGameObj();
		}

		if (game_obj != NULL && game_obj->As_PhysicalGameObj() != NULL ) {

			Vector3 obj_pos;
			game_obj->As_PhysicalGameObj()->Get_Position (&obj_pos);

			//
			//	Is this object the closest one we found yet?
			//
			float len = (obj_pos - pos).Length ();
			if (len >= min_dist && len <= closest_dist) {

				//
				//	If only_human, see if this is a human. Duh.
				//
				if (!only_human ||
					(game_obj->As_SmartGameObj() != NULL &&
					 game_obj->As_SmartGameObj()->Is_Human_Controlled())) {

					closest_dist	= len;
					closest_obj		= game_obj;
				}
			}
		}
	}

	return closest_obj;
}


/*
**
*/
GameObject * Get_The_Star( void )
{
	return (GameObject*)COMBAT_STAR;
}

GameObject * Get_A_Star( const Vector3 & pos )
{
	SoldierGameObj * nearest_human_player = NULL;
	Vector3 n_c_pos = Vector3( 1000000,1000000,1000000 );
	n_c_pos += pos;

	SLNode<SmartGameObj> *objnode;
	for (	objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); objnode; objnode = objnode->Next() ) {
		SoldierGameObj * soldier = objnode->Data()->As_SoldierGameObj();
		if ( soldier && soldier->Is_Human_Controlled() ) {
			Vector3 c_pos;
			soldier->Get_Position( &c_pos );
			c_pos -= pos;
			if ( c_pos.Length2() < n_c_pos.Length2() ) {
				nearest_human_player = soldier;
				n_c_pos = c_pos;
			}
		}
	}
	return nearest_human_player;
}

bool Is_A_Star( GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( obj, false );
	if ( obj->As_SmartGameObj() ) {
		SoldierGameObj * soldier = obj->As_SmartGameObj()->As_SoldierGameObj();
		return soldier && soldier->Is_Human_Controlled();
	}
	return false;
}

/*
** HACK
*/
const char * Get_Damage_Bone_Name( void )
{
	return DamageModelName;
}
bool Get_Damage_Bone_Direction( void )
{
	return DamageModelDirection;
}


/*
**
*/
void	Control_Enable( GameObject * obj, bool enable )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Control_Enable( %d, %d )\n", obj->Get_ID(), enable ));
	SmartGameObj *smart = obj->As_SmartGameObj();
	if (smart) {
		smart->Control_Enable( enable );
	} else {
		Debug_Say(( "This object can't Control_Enable\n" ));
	}
}


/*
**
*/
bool	Is_Object_Visible( GameObject * looker, GameObject * obj )
{
	SCRIPT_PTR_CHECK_RET( looker, false );
	SCRIPT_PTR_CHECK_RET( obj, false );
	SCRIPT_TRACE((	"ST>Is_Object_Visible( %d, %d )\n", looker->Get_ID(), obj->Get_ID() ));
	SmartGameObj *smart = looker->As_SmartGameObj();
	if (smart) {

		PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
		if ( pgobj == NULL ) {
			Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
			return false;
		}

		return smart->Is_Obj_Visible( pgobj );
	} else {
		Debug_Say(( "This object can't Is_Object_Visible\n" ));
	}
	return false;
}


void	Enable_Enemy_Seen( GameObject * obj, bool enable )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Enable_Enemy_Seen( %d, %d )\n", obj->Get_ID(), enable ));
	SmartGameObj *smart = obj->As_SmartGameObj();
	if (smart) {
		smart->Set_Enemy_Seen_Enabled( enable );
	} else {
		Debug_Say(( "Only Smart Objs can Enable_Enemy_Seen\n" ));
	}
}

/*
**
*/

Vector3 _DisplayColor( 1,1,1 );

void	Set_Display_Color( unsigned char red, unsigned char green, unsigned char blue )
{
	_DisplayColor = Vector3( (float)red/255.0f, (float)green/255.0f, (float)blue/255.0f );
}

void	Display_Text( int string_id )
{
	TDBObjClass *	 obj = TranslateDBClass::Find_Object( string_id );

	if ( obj ) {
		DebugManager::Display_Text( obj->Get_String(), _DisplayColor );

		int sound_def_id = (int)obj->Get_Sound_ID ();
		if (sound_def_id > 0) {
			//	Create the sound object
			AudibleSoundClass * sound = WWAudioClass::Get_Instance ()->Create_Sound (sound_def_id);
			if (sound != NULL) {
				sound->Add_To_Scene();
				sound->Release_Ref();
			}
		}
	}
}

void	Display_Float( float value, const char * format )
{
	WideStringClass wide_format;
	wide_format.Convert_From( format );
	WideStringClass string;
	string.Format( wide_format, value );
	DebugManager::Display_Text( string, _DisplayColor );
}

void	Display_Int( int value, const char * format )
{
	WideStringClass wide_format;
	wide_format.Convert_From( format );
	WideStringClass string;
	string.Format( wide_format, value );
	DebugManager::Display_Text( string, _DisplayColor );
}

/*
**
*/
void	Save_Data( ScriptSaver & saver, int id, int size, void * data )
{
	SCRIPT_PTR_CHECK( data );
	saver.CSave.Begin_Micro_Chunk(id);
	saver.CSave.Write(data,size);
	saver.CSave.End_Micro_Chunk();
	WWASSERT((unsigned)size <= 250);		// Make sure we don't save too much
}


void Save_Pointer(ScriptSaver& saver, int /* id */, void* pointer)
{
	SCRIPT_PTR_CHECK(pointer);
    const uint32_t pointer_id = SaveLoadSystemClass::Serialize_Pointer(pointer);
	Save_Data(saver, pointer_id, sizeof(uint32_t), pointer);
}


bool	Load_Begin( ScriptLoader & loader, int * id )
{
	SCRIPT_PTR_CHECK_RET( id, false );
	if ( !loader.CLoad.Open_Micro_Chunk()) {
		return false;
	}
	*id = loader.CLoad.Cur_Micro_Chunk_ID();
	return true;
}

void	Load_Data( ScriptLoader & loader, [[maybe_unused]] int size, void * data )
{
	SCRIPT_PTR_CHECK( data );
	unsigned int chunkSize = loader.CLoad.Cur_Micro_Chunk_Length();
	WWASSERT((unsigned)size >= chunkSize);
	loader.CLoad.Read(data, chunkSize);
	WWASSERT((unsigned)size <= 250);		// Make sure we don't save too much
}


void Load_Pointer(ScriptLoader& loader, void** pointer)
{
	SCRIPT_PTR_CHECK(pointer);
	Load_Data(loader, sizeof(void*), pointer);
	REQUEST_POINTER_REMAP(pointer);
}


void	Load_End( ScriptLoader & loader )
{
	loader.CLoad.Close_Micro_Chunk();
}

void Begin_Chunk(ScriptSaver& saver, unsigned int chunkID)
{
	saver.CSave.Begin_Chunk(chunkID);
}


void End_Chunk(ScriptSaver& saver)
{
	saver.CSave.End_Chunk();
}


bool Open_Chunk(ScriptLoader& loader, unsigned int* chunkID)
{
	SCRIPT_PTR_CHECK_RET(chunkID, false);

	if (loader.CLoad.Open_Chunk()) {
		*chunkID = loader.CLoad.Cur_Chunk_ID();
		return true;
	}

	return false;
}


void Close_Chunk(ScriptLoader& loader)
{
	loader.CLoad.Close_Chunk();
}


//
//	Clear_Map_Cell
//
void
Clear_Map_Cell( int cell_x, int cell_y )
{
	SCRIPT_TRACE((	"ST>Clear_Map_Cell( %d, %d)\n", cell_x, cell_y ));
	MapMgrClass::Clear_Cloud_Cell( cell_x, cell_y );
	HUDClass::Add_Map_Reveal();
	return ;
}


//
//	Clear_Map_Cell_By_Pos
//
void
Clear_Map_Cell_By_Pos( const Vector3 &world_space_pos )
{
	SCRIPT_TRACE((	"ST>Clear_Map_Cell_By_Pos( %f, %f, %f)\n", world_space_pos.X, world_space_pos.Y, world_space_pos.Z ));
	MapMgrClass::Clear_Cloud_Cell( world_space_pos );
	HUDClass::Add_Map_Reveal();
	return ;
}


//
//	Clear_Map_Region_By_Pos
//
void
Clear_Map_Region_By_Pos( const Vector3 &world_space_pos, int pixel_radius )
{
	SCRIPT_TRACE((	"ST>Clear_Map_Region_By_Pos( %f, %f, %f, %d)\n", world_space_pos.X, world_space_pos.Y, world_space_pos.Z, pixel_radius ));
	MapMgrClass::Clear_Cloud_Cells( world_space_pos, pixel_radius );
	HUDClass::Add_Map_Reveal();
	return ;
}


//
//	Clear_Map_Cell_By_Pixel_Pos
//
void
Clear_Map_Cell_By_Pixel_Pos( int pixel_pos_x, int pixel_pos_y )
{
	SCRIPT_TRACE((	"ST>Clear_Map_Cell_By_Pixel_Pos( %d, %d)\n", pixel_pos_x, pixel_pos_y ));
	MapMgrClass::Clear_Cloud_Cell_By_Pixel( pixel_pos_x, pixel_pos_y );
	HUDClass::Add_Map_Reveal();
	return ;
}


//
//	Show_Player_Map_Marker
//
void
Show_Player_Map_Marker( bool onoff )
{
	SCRIPT_TRACE((	"ST>Show_Player_Map_Marker()\n" ));
	MapMgrClass::Show_Player_Marker( onoff );
	return ;
}


//
//	Reveal_Map
//
void
Reveal_Map( void )
{
	SCRIPT_TRACE((	"ST>Reveal_Map()\n" ));
	MapMgrClass::Clear_All_Cloud_Cells();
	HUDClass::Add_Map_Reveal();
	return ;
}

//
//	Shroud_Map
//
void
Shroud_Map( void )
{
	SCRIPT_TRACE((	"ST>Shroud_Map()\n" ));
	MapMgrClass::Cloud_All_Cells();
	return ;
}

//
//	Get_Safe_Flight_Height
//
float
Get_Safe_Flight_Height (float x_pos, float y_pos)
{
	SCRIPT_TRACE((	"ST>Get_Safe_Flight_Height( %.2f, %.2f )\n", x_pos, y_pos ));
	return HeightDBClass::Get_Height ( Vector3 ( x_pos, y_pos, 0 ) );
}


/*
**
*/
void Clear_Radar_Markers( void )
{
	SCRIPT_TRACE((	"ST>Clear_Radar_Markers()\n" ));
	RadarManager::Clear_Markers();
}

void	Clear_Radar_Marker( int id )
{
	SCRIPT_TRACE((	"ST>Clear_Radar_Marker( %d )\n", id ));
	RadarManager::Clear_Marker( id );
}

#if 0
void	Add_Radar_Marker( int id, const Vector3& position, const Vector3& color, bool flash )
{
	SCRIPT_TRACE((	"ST>Add_Radar_Marker( %d, (%f,%f,%f), (%f,%f,%f), %d)\n",
			id, position.X, position.Y, position.Z, color.X, color.Y, color.Z, flash ));
/*	RadarMarkerClass marker;
	marker.ID = id;
	marker.Position = position;
	marker.Color = color;
	marker.Flash = flash;
	RadarManager::Add_Marker( marker );*/
}
#else
void	Add_Radar_Marker( int id, const Vector3& position, int shape_type, int color_type )
{
	SCRIPT_TRACE((	"ST>Add_Radar_Marker( %d, (%f,%f,%f), %d, %d)\n",
			id, position.X, position.Y, position.Z, shape_type, color_type ));

	RadarMarkerClass marker;
	marker.ID = id;
	marker.Position = position;
	marker.Type = shape_type;
	marker.Color = color_type;
	RadarManager::Add_Marker( marker );
}
#endif

#if 0
void	Add_Obj_Radar_Marker( int id, GameObject * obj, Vector3 color, bool flash )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Add_Radar_Marker( %d, %d, (%f,%f,%f), %d)\n",
			id, obj->Get_ID(), color.X, color.Y, color.Z, flash ));
	SCRIPT_PTR_CHECK( obj );
/*	RadarMarkerClass marker;
	marker.ID = id;*/

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}

/*	marker.Set_Game_Object( pgobj );
	marker.Color = color;
	marker.Flash = flash;
	RadarManager::Add_Marker( marker );*/
}
#endif

void	Set_Obj_Radar_Blip_Shape( GameObject * obj, int shape_type )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Set_Obj_Radar_Blip_Shape( %d, %d )\n", obj->Get_ID(), shape_type ));

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}
	if ( shape_type < 0 ) {
		pgobj->Reset_Radar_Blip_Shape_Type();
	} else {
		pgobj->Set_Radar_Blip_Shape_Type( shape_type );
	}
}

void	Set_Obj_Radar_Blip_Color( GameObject * obj, int color_type )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Set_Obj_Radar_Blip_Color( %d, %d )\n", obj->Get_ID(), color_type ));

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return;
	}
	if ( color_type < 0 ) {
		pgobj->Reset_Radar_Blip_Color_Type();
	} else {
		pgobj->Set_Radar_Blip_Color_Type( color_type );
	}
}

void	Enable_Radar( bool enable )
{
	SCRIPT_TRACE((	"ST>Enable_Radar( %d )\n", enable ));
	RadarManager::Set_Hidden( !enable );
}


/*
**
*/
void	Create_Explosion( const char * explosion_def_name, const Vector3 & pos, GameObject * creator )
{
	SCRIPT_PTR_CHECK( explosion_def_name );
	SCRIPT_TRACE((	"ST>Create_Explosion( %s, (%f,%f,%f), %d )\n",
		explosion_def_name, pos.X, pos.Y, pos.Z, (creator != NULL ) ? creator->Get_ID() : 0 ));

	ExplosionDefinitionClass * explosion_def = (ExplosionDefinitionClass *)DefinitionMgrClass::Find_Typed_Definition( explosion_def_name, CLASSID_DEF_EXPLOSION );
	if ( explosion_def != NULL )
	{
		int explosion_id = explosion_def->Get_ID();
		SmartGameObj * smart = NULL;
		if ( creator ) {
			smart = creator->As_SmartGameObj();
		}
		ExplosionManager::Create_Explosion_At( explosion_id, pos, smart );
	}
}

void	Create_Explosion_At_Bone( const char * explosion_def_name, GameObject * object, const char * bone_name, GameObject * creator )
{
	SCRIPT_PTR_CHECK( explosion_def_name );
	SCRIPT_PTR_CHECK( object );
	SCRIPT_PTR_CHECK( bone_name );
	SCRIPT_TRACE((	"ST>Create_Explosion_At_Bone( %s, %d, %s, %d )\n",
		explosion_def_name,
		(object != NULL) ? object->Get_ID() : 0 ,
		bone_name,
		(creator != NULL ) ? creator->Get_ID() : 0 ));

	ExplosionDefinitionClass * explosion_def = (ExplosionDefinitionClass *)DefinitionMgrClass::Find_Typed_Definition( explosion_def_name, CLASSID_DEF_EXPLOSION );
	if ( explosion_def != NULL )
	{
		int explosion_id = explosion_def->Get_ID();
		SmartGameObj * smart = NULL;
		if ( creator ) {
			smart = creator->As_SmartGameObj();
		}

		PhysicalGameObj * pgobj = object->As_PhysicalGameObj();
		if ( pgobj == NULL ) {
			Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
			return;
		}

		Matrix3D tm = pgobj->Peek_Model()->Get_Bone_Transform( bone_name );

		ExplosionManager::Create_Explosion_At( explosion_id, tm, smart );
	}

}



/*
**
*/
void	Enable_HUD( bool enable )
{
	SCRIPT_TRACE((	"ST>Enable_HUD( %d )\n", enable ));
	HUDClass::Enable( enable );
}


void	Mission_Complete( bool success )
{
	SCRIPT_TRACE((	"ST>Mission_Complete( %d )\n", success ));
   CombatManager::Mission_Complete( success );
}


void	Give_PowerUp( GameObject * obj, const char * preset_name, bool display_on_hud )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_PTR_CHECK( preset_name );
	SCRIPT_TRACE((	"ST>Give_PowerUp( %d, %s )\n", obj->Get_ID(), preset_name ));
	SmartGameObj *smart = obj->As_SmartGameObj();
	if (smart) {

		PowerUpGameObjDef *	def = (PowerUpGameObjDef*)DefinitionMgrClass::Find_Typed_Definition( preset_name, CLASSID_GAME_OBJECT_DEF_POWERUP );
		if ( def != NULL ) {
			def->Grant( smart, NULL, display_on_hud );
		} else {
			Debug_Say(( "Powerup Definition %s not found\n", preset_name ));
		}

	} else {
		Debug_Say(( "Only Samrt objs can get powerups\n" ));
	}
}


bool Innate_Soldier_Enable( GameObject * obj, int bits, bool state )
{
	SCRIPT_PTR_CHECK_RET(obj, false);
	SCRIPT_TRACE((	"ST>Innate_Soldier_Enable( %d, %d, %d )\n", obj->Get_ID(), bits, state ));

	PhysicalGameObj * pgobj = obj->As_PhysicalGameObj();
	if ( pgobj == NULL ) {
//		Cinematics call this for non-soldiers
//		Debug_Say(( "Not a PhysicalGameObj at %s %d\n", __FILE__, __LINE__ ));
		return false;
	}

	SoldierGameObj * soldier = pgobj->As_SoldierGameObj();
	if ( soldier == NULL ) {
//		Cinematics call this for non-soldiers
//		Debug_Say(( "Object is not a soldier!\n" ));
	}
	SCRIPT_PTR_CHECK_RET(soldier, false);
	bool old_state = soldier->Is_Innate_Enabled( bits );
	if ( state ) {
		soldier->Innate_Enable( bits );
	} else {
		soldier->Innate_Disable( bits );
	}
	return old_state;
}


bool	Innate_Soldier_Enable_Enemy_Seen( GameObject * obj, bool state )
{
	return Innate_Soldier_Enable( obj, SOLDIER_INNATE_EVENT_ENEMY_SEEN, state );
}


bool	Innate_Soldier_Enable_Gunshot_Heard( GameObject * obj, bool state )
{
	return Innate_Soldier_Enable( obj, SOLDIER_INNATE_EVENT_GUNSHOT_HEARD, state );
}


bool	Innate_Soldier_Enable_Footsteps_Heard( GameObject * obj, bool state )
{
	return Innate_Soldier_Enable( obj, SOLDIER_INNATE_EVENT_FOOTSTEP_HEARD, state );
}


bool	Innate_Soldier_Enable_Bullet_Heard( GameObject * obj, bool state )
{
	return Innate_Soldier_Enable( obj, SOLDIER_INNATE_EVENT_BULLET_HEARD, state );
}


bool	Innate_Soldier_Enable_Actions( GameObject * obj, bool state )
{
	return Innate_Soldier_Enable( obj, SOLDIER_INNATE_ACTIONS, state );
}


// Disable all innate abilities
void Innate_Disable(GameObject* object)
{
	Innate_Soldier_Enable(object, 0xFFFFFFFF, false);
}


// Enable all innate abilities
void Innate_Enable(GameObject* object)
{
	Innate_Soldier_Enable(object, 0xFFFFFFFF, true);
}


void	Set_Innate_Soldier_Home_Location( GameObject * obj, const Vector3& home_pos, float home_radius )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Set_Innate_Soldier_Home_Location( %d, (%f %f %f), %f )\n", obj->Get_ID(), home_pos.X, home_pos.Y, home_pos.Z, home_radius ));

	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart != NULL ) {
		SoldierGameObj * soldier = smart->As_SoldierGameObj();
		if ( soldier != NULL ) {
			SoldierObserverClass * innate = soldier->Get_Innate_Controller();
			if ( innate ) {
				innate->Set_Home_Location( home_pos, home_radius );
			}
		}
	}

}

void	Set_Innate_Aggressiveness( GameObject * obj, float aggressiveness )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Set_Innate_Aggressiveness( %d, %f )\n", obj->Get_ID(), aggressiveness ));

	bool set = false;
	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart != NULL ) {
		SoldierGameObj * soldier = smart->As_SoldierGameObj();
		if ( soldier != NULL ) {
			SoldierObserverClass * innate = soldier->Get_Innate_Controller();
			if ( innate ) {
				innate->Set_Aggressiveness( aggressiveness );
				set = true;
			}
		}
	}
	if ( !set ) {
		Debug_Say(( "Can only Set_Innate_Aggressiveness on a soldier\n" ));
	}
}

void	Set_Innate_Take_Cover_Probability( GameObject * obj, float probability )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Set_Innate_Take_Cover_Probability( %d, %f )\n", obj->Get_ID(), probability ));

	bool set = false;
	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart != NULL ) {
		SoldierGameObj * soldier = smart->As_SoldierGameObj();
		if ( soldier != NULL ) {
			SoldierObserverClass * innate = soldier->Get_Innate_Controller();
			if ( innate ) {
				innate->Set_Take_Cover_Probability( probability );
				set = true;
			}
		}
	}
	if ( !set ) {
		Debug_Say(( "Can only Set_Innate_Take_Cover_Probability on a soldier\n" ));
	}
}

void	Set_Innate_Is_Stationary( GameObject * obj, bool stationary )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Set_Innate_Take_Cover_Probability( %d, %d )\n", obj->Get_ID(), stationary ));

	bool set = false;
	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart != NULL ) {
		SoldierGameObj * soldier = smart->As_SoldierGameObj();
		if ( soldier != NULL ) {
			SoldierObserverClass * innate = soldier->Get_Innate_Controller();
			if ( innate ) {
				innate->Set_Is_Stationary( stationary );
				set = true;
			}
		}
	}
	if ( !set ) {
		Debug_Say(( "Can only Set_Innate_Is_Stationary on a soldier\n" ));
	}
}


void	Innate_Force_State_Bullet_Heard( GameObject * obj, const Vector3 & pos )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Innate_Force_State_Bullet_Heard( %d, ( %f, %f, %f, )  )\n", obj->Get_ID(), pos.X, pos.Y, pos.Z ));

	bool set = false;
	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart != NULL ) {
		SoldierGameObj * soldier = smart->As_SoldierGameObj();
		if ( soldier != NULL ) {
			SoldierObserverClass * innate = soldier->Get_Innate_Controller();
			if ( innate ) {
				innate->Set_State( soldier, SoldierObserverClass::SOLDIER_AI_BULLET_HEARD, pos);
				set = true;
			}
		}
	}
	if ( !set ) {
		Debug_Say(( "Can only Innate_Force_State_Bullet_Heard on a soldier with innate\n" ));
	}
}

void	Innate_Force_State_Footsteps_Heard( GameObject * obj, const Vector3 & pos )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Innate_Force_State_Footsteps_Heard( %d, ( %f, %f, %f, )  )\n", obj->Get_ID(), pos.X, pos.Y, pos.Z ));

	bool set = false;
	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart != NULL ) {
		SoldierGameObj * soldier = smart->As_SoldierGameObj();
		if ( soldier != NULL ) {
			SoldierObserverClass * innate = soldier->Get_Innate_Controller();
			if ( innate ) {
				innate->Set_State( soldier, SoldierObserverClass::SOLDIER_AI_FOOTSTEPS_HEARD, pos);
				set = true;
			}
		}
	}
	if ( !set ) {
		Debug_Say(( "Can only Innate_Force_State_Footsteps_Heard on a soldier with innate\n" ));
	}
}

void	Innate_Force_State_Gunshots_Heard( GameObject * obj, const Vector3 & pos )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_TRACE((	"ST>Innate_Force_State_Gunshot_Heard( %d, ( %f, %f, %f, )  )\n", obj->Get_ID(), pos.X, pos.Y, pos.Z ));

	bool set = false;
	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart != NULL ) {
		SoldierGameObj * soldier = smart->As_SoldierGameObj();
		if ( soldier != NULL ) {
			SoldierObserverClass * innate = soldier->Get_Innate_Controller();
			if ( innate ) {
				innate->Set_State( soldier, SoldierObserverClass::SOLDIER_AI_GUNSHOT_HEARD, pos);
				set = true;
			}
		}
	}
	if ( !set ) {
		Debug_Say(( "Can only Innate_Force_State_Gunshot_Heard on a soldier with innate\n" ));
	}
}

void	Innate_Force_State_Enemy_Seen( GameObject * obj, GameObject * enemy )
{
	SCRIPT_PTR_CHECK( obj );
	SCRIPT_PTR_CHECK( enemy );
	SCRIPT_TRACE((	"ST>Innate_Force_State_Enemy_Seen( %d, %d )\n", obj->Get_ID(), enemy->Get_ID() ));

	bool set = false;
	SmartGameObj * smart = obj->As_SmartGameObj();
	if ( smart != NULL ) {
		SoldierGameObj * soldier = smart->As_SoldierGameObj();
		if ( soldier != NULL ) {
			SoldierObserverClass * innate = soldier->Get_Innate_Controller();
			if ( innate ) {
				innate->Set_State( soldier, SoldierObserverClass::SOLDIER_AI_ENEMY_SEEN, Vector3(0,0,0), enemy );
				set = true;
			}
		}
	}
	if ( !set ) {
		Debug_Say(( "Can only Innate_Force_State_Enemy_Seen on a soldier with innate\n" ));
	}
}


/*
**
*/

void	Static_Anim_Phys_Goto_Frame( int obj_id, float frame, const char * anim_name )
{
	SCRIPT_TRACE((	"ST>Static_Anim_Phys_Goto_Frame( %d, %d )\n", obj_id, frame ));

	StaticPhysClass * spc = COMBAT_SCENE->Find_Static_Object( obj_id );
	if ( spc != NULL ) {
		StaticAnimPhysClass * sapc = spc->As_StaticAnimPhysClass();
		if ( sapc  ) {
			if ( anim_name != NULL ) {
				sapc->Get_Animation_Manager().Set_Animation( anim_name );
			}
			sapc->Get_Animation_Manager().Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
			sapc->Get_Animation_Manager().Set_Target_Frame( frame );
		}
	}
}


void	Static_Anim_Phys_Goto_Last_Frame( int obj_id, const char * anim_name )
{
	SCRIPT_TRACE((	"ST>Static_Anim_Phys_Goto_Last_Frame( %d )\n", obj_id ));

	StaticPhysClass * spc = COMBAT_SCENE->Find_Static_Object( obj_id );
	if ( spc != NULL ) {
		StaticAnimPhysClass * sapc = spc->As_StaticAnimPhysClass();
		if ( sapc  ) {
			if ( anim_name != NULL ) {
				sapc->Get_Animation_Manager().Set_Animation( anim_name );
			}
			sapc->Get_Animation_Manager().Set_Animation_Mode( AnimCollisionManagerClass::ANIMATE_TARGET );
			sapc->Get_Animation_Manager().Set_Target_Frame( sapc->Get_Animation_Manager().Peek_Animation()->Get_Num_Frames() - 1 );
		}
	}
}

unsigned int Get_Sync_Time( void )
{
	return CombatManager::Get_Sync_Time();
}

/*
** Objectives
*/
void	Add_Objective( int id, int type,  int status, int short_description_id, const char * description_sound_filename, int long_description_id )
{
	SCRIPT_TRACE((	"ST>Add_Objective( %d, %d, %d, %d %d )\n", id, type, status, short_description_id, long_description_id ));
	ObjectiveManager::Add_Objective( id, type, status, short_description_id, long_description_id, description_sound_filename );
}

void	Remove_Objective( int id )
{
	SCRIPT_TRACE((	"ST>Remove_Objective( %d )\n", id ));
	ObjectiveManager::Remove_Objective( id );
}

void	Set_Objective_Status( int id, int status )
{
	SCRIPT_TRACE((	"ST>Set_Objective_Status( %d, %d )\n", id, status ));
	ObjectiveManager::Set_Objective_Status( id, status );
}

void	Change_Objective_Type( int id, int type )
{
	SCRIPT_TRACE((	"ST>Change_Objective_Type( %d, %d )\n", id, type ));
	ObjectiveManager::Change_Objective_Type( id, type );
}

void	Set_Objective_Radar_Blip( int id, const Vector3 & position )
{
	SCRIPT_TRACE((	"ST>Set_Objective_Radar_Blip( %d, %f %f %f )\n", id, position ));
	ObjectiveManager::Set_Objective_Radar_Blip( id, position );
}

void	Set_Objective_Radar_Blip_Object( int id, ScriptableGameObj * unit )
{
	SCRIPT_PTR_CHECK( unit );
	SCRIPT_TRACE((	"ST>Set_Objective_Radar_Blip_Object( %d, %d )\n", id, unit->Get_ID() ));
	PhysicalGameObj * pobj = NULL;
	if ( unit ) {
		pobj = unit->As_PhysicalGameObj();
	}
	ObjectiveManager::Set_Objective_Radar_Blip( id, pobj );
}

void	Set_Objective_HUD_Info( int id, float priority, const char * texture_name, int message_id )
{
	SCRIPT_PTR_CHECK( texture_name );
	SCRIPT_TRACE((	"ST>Set_Objective_HUD_Info( %d, %f, %s, %d )\n", id, priority, texture_name, message_id ));
	ObjectiveManager::Set_Objective_HUD_Info( id, priority, texture_name, message_id );
}

void	Set_Objective_HUD_Info_Position( int id, float priority, const char * texture_name, int message_id, const Vector3 & position )
{
	SCRIPT_PTR_CHECK( texture_name );
	SCRIPT_TRACE((	"ST>Set_Objective_HUD_Info_Position( %d, %f, %s, %d )\n", id, priority, texture_name, message_id ));
	ObjectiveManager::Set_Objective_HUD_Info( id, priority, texture_name, message_id, position );
}

/*
**
*/
void	Shake_Camera( const Vector3 & pos, float radius, float intensity, float duration )
{
	SCRIPT_TRACE((	"ST>Shake_Camera( (%f, %f, %f), %f, %f, %f\n",pos.X, pos.Y, pos.Z, radius, intensity, duration ));
	if ( COMBAT_SCENE ) {
		COMBAT_SCENE->Add_Camera_Shake( pos, radius, duration, intensity );
	}
}


void	Enable_Spawner( int id, bool enable )
{
	SCRIPT_TRACE((	"ST>Enable_Spawned( %d, %d )\n",id, enable ));
	SpawnManager::Spawner_Enable( id, enable );
}

GameObject * Trigger_Spawner( int id )
{
	SCRIPT_TRACE((	"ST>Trigger_Spawned( %d )\n", id ));
	return SpawnManager::Spawner_Trigger( id );
}

void	Enable_Engine( GameObject* object, bool onoff )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Enable_Engine( %d, %d )\n", object, onoff ));

	PhysicalGameObj *physical_obj = object->As_PhysicalGameObj ();
	if ( physical_obj != NULL ) {
		VehicleGameObj *vehicle = physical_obj->As_VehicleGameObj ();
		if ( vehicle != NULL ) {
			vehicle->Enable_Engine( onoff );
		}
	}
}


/*
**
*/
int	Get_Difficulty_Level( void )
{
	return CombatManager::Get_Difficulty_Level();
}

void	Grant_Key( GameObject* object, int key, bool grant = true )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Grant_Key( %d, %d )\n", object->Get_ID(), key, grant ));

	SoldierGameObj * soldier = NULL;
	if ( object->As_SmartGameObj() != NULL ) {
		soldier = object->As_SmartGameObj()->As_SoldierGameObj();
	}

	if ( soldier == NULL ) {
		Debug_Say(( "Object is not a soldier!\n" ));
	} else {
		if ( grant ) {
			soldier->Give_Key( key );
		} else {
			soldier->Remove_Key( key );
		}
	}
}

bool	Has_Key( GameObject* object, int key )
{
	SoldierGameObj * soldier = NULL;
	if ( object->As_SmartGameObj() != NULL ) {
		soldier = object->As_SmartGameObj()->As_SoldierGameObj();
	}

	return soldier && soldier->Has_Key( key );
}

void	Enable_Hibernation( GameObject * object, bool enable )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Enable_Hibernation( %d, %d )\n", object->Get_ID(), enable ));

	if ( object->As_PhysicalGameObj() ) {
		object->As_PhysicalGameObj()->Enable_Hibernation( enable );
	}
}

void	Attach_To_Object_Bone( GameObject * object, GameObject * host_object, const char * bone_name )
{
	SCRIPT_PTR_CHECK( object );
	if ( host_object == NULL ) {
		SCRIPT_TRACE((	"ST>Attach_To_Object_Bone( %d, NULL )\n", object->Get_ID() ));
	} else {
		SCRIPT_TRACE((	"ST>Attach_To_Object_Bone( %d, %d, %s )\n", object->Get_ID(), host_object->Get_ID(), bone_name ? "NULL" : bone_name  ));
	}

	if ( object->As_PhysicalGameObj() ) {
		if ( host_object == NULL ) {
			object->As_PhysicalGameObj()->Attach_To_Object_Bone( NULL, NULL );
		} else {
			WWASSERT( host_object->As_PhysicalGameObj() ) ;
			object->As_PhysicalGameObj()->Attach_To_Object_Bone( host_object->As_PhysicalGameObj(), bone_name );
		}
	} else {
		Debug_Say(( "Can only Attach_To_Object_Bone for PhysicalObjects\n" ));
	}
}

int	Create_Conversation( const char *conversation_name, int priority, float max_dist, bool is_interruptable )
{
	int active_conversation_id = -1;
	SCRIPT_TRACE((	"ST>Create_Conversation( %s )\n", conversation_name ));

	//
	//	Try to find the requested conversation
	//
	ConversationClass *conversation = ConversationMgrClass::Find_Conversation( conversation_name );
	if ( conversation != NULL ) {

		//
		//	Create a new run-time conversation object
		//
		ActiveConversationClass *active_conversation = ConversationMgrClass::Create_New_Conversation( conversation );
		if (active_conversation != NULL) {

			if (priority > 0) {
				active_conversation->Set_Priority (priority);
			}

			if (max_dist > 0) {
				active_conversation->Set_Max_Dist (max_dist);
			}

			active_conversation->Set_Is_Interruptable (is_interruptable);

			//
			//	Return the ID of the conversation to the caller
			//
			active_conversation_id = active_conversation->Get_ID ();
			REF_PTR_RELEASE( active_conversation );
		}

		REF_PTR_RELEASE( conversation );
	}

	return active_conversation_id;
}

void	Join_Conversation_Facing( GameObject * object, int active_conversation_id, int obj_id_to_face )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Join_Conversation_Facing( %d, %d, %d )\n", object->Get_ID (), active_conversation_id, obj_id_to_face ));

	//
	//	Dig out the soldier pointer (if we have one)
	//
	PhysicalGameObj *game_obj = NULL;
	if ( object != NULL ) {
		game_obj = object->As_PhysicalGameObj();
	}

	//
	//	Lookup the run-time conversation object
	//
	ActiveConversationClass *active_conversation = NULL;
	active_conversation = ConversationMgrClass::Find_Active_Conversation( active_conversation_id );
	if (active_conversation != NULL) {

		//
		//	Add this object to the conversation
		//
		OratorClass *orator = active_conversation->Add_Orator( game_obj );

		//
		//	Set the flags
		//
		orator->Set_Flag( OratorClass::FLAG_DONT_MOVE,			true );
		orator->Set_Flag( OratorClass::FLAG_DONT_TURN_HEAD,	false );
		orator->Set_Flag( OratorClass::FLAG_DONT_FACE,			false );
		orator->Set_Look_At_Obj( obj_id_to_face );

		REF_PTR_RELEASE( active_conversation );
	}

	return ;
}


void	Join_Conversation( GameObject * object, int active_conversation_id, bool allow_move, bool allow_head_turn, bool allow_face )
{
	int obj_id = 0;
	if ( object != NULL ) {
		obj_id = object->Get_ID();
	}

	SCRIPT_TRACE((	"ST>Join_Conversation( %d, %d, %d, %d )\n", obj_id, active_conversation_id, allow_move, allow_head_turn ));

	//
	//	Dig out the soldier pointer (if we have one)
	//
	PhysicalGameObj *game_obj = NULL;
	if ( object != NULL ) {
		game_obj = object->As_PhysicalGameObj();
	}

	//
	//	Lookup the run-time conversation object
	//
	ActiveConversationClass *active_conversation = NULL;
	active_conversation = ConversationMgrClass::Find_Active_Conversation( active_conversation_id);
	if (active_conversation != NULL) {

		//
		//	Add this object to the conversation
		//
		OratorClass *orator = active_conversation->Add_Orator( game_obj );

		//
		//	Set the flags
		//
		orator->Set_Flag( OratorClass::FLAG_DONT_MOVE,			!allow_move );
		orator->Set_Flag( OratorClass::FLAG_DONT_TURN_HEAD,	!allow_head_turn );
		orator->Set_Flag( OratorClass::FLAG_DONT_FACE,			!allow_face );

		REF_PTR_RELEASE( active_conversation );
	}

	return ;
}


void	Stop_All_Conversations( void )
{
	SCRIPT_TRACE((	"ST>Stop_All_Conversations( )\n" ));

	//
	//	Simply ask the conversation manager to end all playing conversations
	//
	ConversationMgrClass::Reset_Active_Conversations();
	return ;
}


void	Stop_Conversation( int active_conversation_id )
{
	SCRIPT_TRACE((	"ST>Stop_Conversation( %d )\n", active_conversation_id ));

	//
	//	Lookup the run-time conversation object
	//
	ActiveConversationClass *active_conversation = NULL;
	active_conversation = ConversationMgrClass::Find_Active_Conversation( active_conversation_id);
	if (active_conversation != NULL) {

		//
		//	Stop the conversation
		//
		active_conversation->Stop_Conversation (ACTION_COMPLETE_CONVERSATION_INTERRUPTED);
	}

	return ;
}


void	Start_Conversation( int active_conversation_id, int action_id )
{
	SCRIPT_TRACE((	"ST>Start_Conversation( %d, %d )\n", active_conversation_id, action_id ));

	//
	//	Lookup the run-time conversation object
	//
	ActiveConversationClass *active_conversation = NULL;
	active_conversation = ConversationMgrClass::Find_Active_Conversation( active_conversation_id);
	if (active_conversation != NULL) {

		//
		//	Start the conversation
		//
		active_conversation->Set_Action_ID( action_id );
		active_conversation->Start_Conversation();
		REF_PTR_RELEASE( active_conversation );
	}

	return ;
}

void	Monitor_Conversation( GameObject * object, int active_conversation_id )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Monitor_Conversation( %d, %d )\n", object->Get_ID (), active_conversation_id ));

	//
	//	Lookup the run-time conversation object
	//
	ActiveConversationClass *active_conversation = NULL;
	active_conversation = ConversationMgrClass::Find_Active_Conversation( active_conversation_id);
	if (active_conversation != NULL) {

		//
		//	Start the conversation
		//
		active_conversation->Register_Monitor( object );
		REF_PTR_RELEASE( active_conversation );
	}

	return ;
}


void	Start_Random_Conversation( GameObject * object )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Start_Random_Conversation( %d )\n", object->Get_ID() ));

	//
	//	Dig the physical game obj out of the game object pointer
	//
	PhysicalGameObj *game_obj = NULL;
	if ( object != NULL ) {
		game_obj = object->As_PhysicalGameObj();
	}

	//
	//	Start any conversation
	//
	if ( game_obj != NULL ) {
		ConversationMgrClass::Start_Conversation( game_obj );
	}

	return ;
}


/*
**
*/
void	Lock_Soldier_Facing( GameObject * object, GameObject * object_to_face, bool turn_body )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Lock_Soldier_Facing( %d, %d )\n", object->Get_ID(), turn_body ));

	//
	//	Is this a physical game object?
	//
	PhysicalGameObj *phys_game_obj = object->As_PhysicalGameObj();
	if (phys_game_obj != NULL) {

		//
		//	Is this a soldier game object?
		//
		SoldierGameObj *soldier = phys_game_obj->As_SoldierGameObj();
		if ( soldier != NULL ) {
			if ( object_to_face != NULL ) {
				soldier->Lock_Facing( object_to_face->As_PhysicalGameObj(), turn_body );
			} else {
				soldier->Lock_Facing( NULL, false );
			}
		}
	}

	return ;
}


/*
**
*/
void	Unlock_Soldier_Facing( GameObject * object )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Unlock_Soldier_Facing( %d )\n", object->Get_ID() ));

	//
	//	Is this a physical game object?
	//
	PhysicalGameObj *phys_game_obj = object->As_PhysicalGameObj();
	if (phys_game_obj != NULL) {

		//
		//	Is this a soldier game object?
		//
		SoldierGameObj *soldier = phys_game_obj->As_SoldierGameObj();
		if ( soldier != NULL ) {
			soldier->Lock_Facing( NULL, false );
		}
	}

	return ;
}


/*
**
*/
void	Apply_Damage( GameObject * object, float amount, const char * warhead_name, GameObject * damager = NULL )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Apply_Damage( %d, %f, %s )\n", object->Get_ID(), amount, warhead_name ));

	DamageableGameObj * damgo = object->As_DamageableGameObj();
	if ( damgo ) {
		int warhead = ArmorWarheadManager::Get_Warhead_Type( warhead_name );
		SmartGameObj * smart = NULL;
		if ( damager ) {
			smart = damager->As_SmartGameObj();
		}
		OffenseObjectClass offense( amount, warhead, smart );
		if ( damgo->As_SmartGameObj() ) {
			damgo->As_SmartGameObj()->Apply_Damage_Extended( offense );
		} else {
			damgo->Apply_Damage( offense );
		}
	} else {
		Debug_Say(( "Can only Apply_Damage to a DamageableGameObj\n" ));
	}
}

void	Set_Loiters_Allowed( GameObject * object, bool allowed )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Set_Loiters_Allowed( %d, %d )\n", object->Get_ID(), allowed ));

	PhysicalGameObj * physgo = object->As_PhysicalGameObj();
	SoldierGameObj * soldier = NULL;
	if ( physgo ) {
		soldier = physgo->As_SoldierGameObj();
	}
	if ( soldier ) {
		soldier->Set_Loiters_Allowed( allowed );
	} else {
		Debug_Say(( "Can only Set_Loiters_Allowed to a SoldierGameObj\n" ));
	}

}

void	Set_Is_Visible( GameObject * object, bool visible )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Set_Is_Visible( %d, %d )\n", object->Get_ID(), visible ));

	PhysicalGameObj * physgo = object->As_PhysicalGameObj();
	SoldierGameObj * soldier = NULL;
	if ( physgo ) {
		soldier = physgo->As_SoldierGameObj();
	}
	if ( soldier ) {
		soldier->Set_Is_Visible( visible );
	} else {
		Debug_Say(( "Can only Set_Is_Visible on a SoldierGameObj\n" ));
	}
}


void	Set_Is_Rendered( GameObject * object, bool rendered )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Set_Is_Rendered( %d, %d )\n", object->Get_ID(), rendered ));

	PhysicalGameObj * physgo = object->As_PhysicalGameObj();
	if ( physgo ) {
		physgo->Peek_Model()->Set_Hidden( !rendered );
		physgo->Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
	} else {
		Debug_Say(( "Can only Set_Is_Rendered on a PhysicalGameObj\n" ));
	}
}


/*
**
*/
float	Get_Points( GameObject * object )
{
	if ( object && object->As_SmartGameObj() && object->As_SmartGameObj()->Get_Player_Data() ) {
		return object->As_SmartGameObj()->Get_Player_Data()->Get_Score();
	}
	Debug_Say(( "Cannot Get Points for object\n" ));
	return 0;
}

void	Give_Points( GameObject * object, float points, bool entire_team )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Give_Points( %d, %f, %d )\n", object->Get_ID(), points, entire_team ));

	SmartGameObj * smart = object->As_SmartGameObj();
	if ( smart ) {
		if ( entire_team ) {
			SLNode<SoldierGameObj> *objnode;
			for (	objnode = GameObjManager::Get_Star_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
				SoldierGameObj * obj = objnode->Data();
				if ( obj && obj->Get_Player_Data() && obj->Is_Teammate( smart ) ) {
					obj->Get_Player_Data()->Increment_Score( points );
				}
			}
		} else {
			if ( smart->Get_Player_Data() ) {
				smart->Get_Player_Data()->Increment_Score( points );
			}
		}
	} else {
		Debug_Say(( "Cannot Give Points to object\n" ));
	}
}

/*
**
*/
float	Get_Money( GameObject * object )
{
	if ( object && object->As_SmartGameObj() && object->As_SmartGameObj()->Get_Player_Data() ) {
		return object->As_SmartGameObj()->Get_Player_Data()->Get_Money();
	}
	Debug_Say(( "Cannot Get Money for object\n" ));
	return 0;
}

void	Give_Money( GameObject * object, float money, bool entire_team )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Give_Money( %d, %f, %d )\n", object->Get_ID(), money, entire_team ));

	SmartGameObj * smart = object->As_SmartGameObj();
	if ( smart ) {
		if ( entire_team ) {
			SLNode<SoldierGameObj> *objnode;
			for (	objnode = GameObjManager::Get_Star_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
				SoldierGameObj * obj = objnode->Data();
				if ( obj && obj->Get_Player_Data() && obj->Is_Teammate( smart ) ) {
					obj->Get_Player_Data()->Increment_Money( money );
				}
			}
		} else {
			if ( smart->Get_Player_Data() ) {
				smart->Get_Player_Data()->Increment_Money( money );
			}
		}
	} else {
		Debug_Say(( "Cannot Give Money to object\n" ));
	}
}

/*
**
*/
bool	Get_Building_Power( GameObject * object )
{
	SCRIPT_PTR_CHECK_RET( object, false );
	BuildingGameObj *building = object->As_BuildingGameObj();
	if ( building ) {
		return ( building->Is_Power_Enabled() );
	}
	Debug_Say(( "Object is not a Building\n" ));
	return false;
}

void	Set_Building_Power( GameObject * object, bool onoff )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Set_Building_Power( %d, %d )\n", object->Get_ID(), onoff ));

	BuildingGameObj *building = object->As_BuildingGameObj();
	if ( building ) {
		building->Enable_Power( onoff );
	} else {
		Debug_Say(( "Object is not a Building\n" ));
	}
}


void	Play_Building_Announcement( GameObject * object, int text_id )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Play_Building_Announcement( %d, %d )\n", object->Get_ID(), text_id ));

	BuildingGameObj *building = object->As_BuildingGameObj();
	if ( building ) {
		building->Play_Announcement( text_id, false );
	} else {
		Debug_Say(( "Object is not a Building\n" ));
	}
}


GameObject * Find_Nearest_Building_To_Pos( const Vector3 & position, const char * mesh_prefix )
{
	SCRIPT_PTR_CHECK_RET( mesh_prefix, NULL );
	SCRIPT_TRACE((	"ST>Find_Nearest_Building_To_Pos( (%f,%f,%f), %s )\n", position.X, position.Y, position.Z, mesh_prefix ));

	float best_dist2						= 99999.0F;
	BuildingGameObj *best_building	= NULL;

	//
	//	Loop over all the buildings in the level
	//
	SLNode<BuildingGameObj> *obj_node = NULL;
	for ( obj_node = GameObjManager::Get_Building_Game_Obj_List()->Head(); obj_node; obj_node = obj_node->Next() ) {
		BuildingGameObj *building = obj_node->Data ()->As_BuildingGameObj ();
		if (building != NULL) {

			//
			//	Is this the type of building we're looking for?
			//
			const char *name_prefix = building->Get_Name_Prefix();
			if (::stricmp( mesh_prefix, name_prefix ) == 0) {

				//
				//	Is this the closest building we've found yet?
				//
				Vector3 building_pos;
				building->Get_Position (&building_pos);
				float dist2 = (building_pos - position).Length2 ();
				if (dist2 < best_dist2) {
					best_building	= building;
					best_dist2		= dist2;
				}
			}
		}
	}

	return best_building;
}


GameObject * Find_Nearest_Building( GameObject * object, const char * mesh_prefix )
{
	SCRIPT_PTR_CHECK_RET( object, NULL );
	SCRIPT_PTR_CHECK_RET( mesh_prefix, NULL );
	SCRIPT_TRACE((	"ST>Find_Nearest_Building( %d, %s )\n", object->Get_ID(), mesh_prefix ));

	//
	//	Get the position of the game object
	//
	Vector3 position;
	object->Get_Position (&position);

	//
	//	Return the building that's nearest to this position
	//
	return Find_Nearest_Building_To_Pos( position, mesh_prefix );
}


int	Team_Members_In_Zone( GameObject * object, int player_type )
{
	SCRIPT_PTR_CHECK_RET( object, 0 );

	ScriptZoneGameObj *script_zone = object->As_ScriptZoneGameObj();
	if ( script_zone ) {
		return script_zone->Count_Team_Members_Inside( player_type );
	} else {
		Debug_Say(( "Object is not a ScriptZone\n" ));
	}

	return 0;
}

void Set_Clouds (float cloudcover, float cloudgloominess, float ramptime)
{
	if (!BackgroundMgrClass::Set_Clouds (cloudcover, cloudgloominess, ramptime)) {
		Debug_Say (("Cannot set clouds\n"));
	}
}

void Set_Lightning (float intensity, float startdistance, float enddistance, float heading, float distribution, float ramptime)
{
	if (!BackgroundMgrClass::Set_Lightning (intensity, startdistance, enddistance, heading, distribution, ramptime)) {
		Debug_Say (("Cannot set lightning\n"));
	}
}

void Set_War_Blitz (float intensity, float startdistance, float enddistance, float heading, float distribution, float ramptime)
{
	if (!BackgroundMgrClass::Set_War_Blitz (intensity, startdistance, enddistance, heading, distribution, ramptime)) {
		Debug_Say (("Cannot set war blitz\n"));
	}
}

void Set_Wind (float heading, float speed, float variability, float ramptime)
{
	if (!WeatherMgrClass::Set_Wind (heading, speed, variability, ramptime)) {
		Debug_Say (("Cannot set wind\n"));
	}
}

void Set_Rain (float density, float ramptime, bool /* prime */)
{
	if (!WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_RAIN, density, ramptime)) {
		Debug_Say (("Cannot set rain\n"));
	}
}

void Set_Snow (float density, float ramptime, bool /* prime */)
{
	if (!WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_SNOW, density, ramptime)) {
		Debug_Say (("Cannot set snow\n"));
	}
}

void Set_Ash (float density, float ramptime, bool /* prime */)
{
	if (!WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_ASH, density, ramptime)) {
		Debug_Say (("Cannot set ash\n"));
	}
}

void Set_Fog_Enable (bool enabled)
{
	WeatherMgrClass::Set_Fog_Enable (enabled);
}

void Set_Fog_Range (float startdistance, float enddistance, float ramptime)
{
	if (!WeatherMgrClass::Set_Fog_Range (startdistance, enddistance, ramptime)) {
		Debug_Say (("Cannot set fog range\n"));
	}
}

void Enable_Stealth(GameObject * object, bool onoff)
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Enable_Stealth( %d, %d )\n", object->Get_ID(), (onoff ? 1 : 0)));

	SmartGameObj * smartobj = object->As_SmartGameObj();
	if (smartobj != NULL) {
		smartobj->Enable_Stealth(onoff);
	}
}

void	Cinematic_Sniper_Control(bool enabled, float zoom)
{
	SCRIPT_TRACE((	"ST>Sniper_Control( %d, %f )\n", enabled, zoom ));

	if ( COMBAT_CAMERA ) {
		COMBAT_CAMERA->Cinematic_Sniper_Control( enabled, zoom );
	}
}

/*
**
*/
void *	Text_File_Open( const char * filename )
{
	FileClass * file = _TheFileFactory->Get_File( filename );
	if ( file ) {
		file->Open();
		if ( !file->Is_Available() ) {
			_TheFileFactory->Return_File( file );
			file = NULL;
		}
	}
	return (void *)file;
}

bool	Text_File_Get_String( void *handle, char * buffer, int size )
{
	FileClass * file = (FileClass *)handle;
	char ch[4];
	char *b = buffer;
	while ( file->Read( &ch[0], 1 ) == 1 ) {
		if ( size > 0 ) {
			*b++ = ch[0];
			size--;
		}
		if ( ch[0] == '\n' ) {
			break;
		}
	}
	*b = 0;
	return (buffer[0] != 0);
}

void	Text_File_Close( void *handle )
{
	FileClass * file = (FileClass *)handle;
	if ( file != NULL ) {
		file->Close();
		_TheFileFactory->Return_File( file );
	}
}


/*
**
*/
void	Enable_Vehicle_Transitions( GameObject * object, bool enable )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Enable_Vehicle_Transitions( %d, %d )\n", object->Get_ID(), enable ));

	PhysicalGameObj * physgo = object->As_PhysicalGameObj();
	VehicleGameObj * vehicle = NULL;
	if ( physgo ) {
		vehicle = physgo->As_VehicleGameObj();
	}
	if ( vehicle ) {
		vehicle->Script_Enable_Transitions( enable );
	} else {
		Debug_Say(( "Can only Enable_Vehicle_Transitions on a Vehicle\n" ));
	}
}


/*
**
*/
void	Display_GDI_Player_Terminal (void)
{
	SCRIPT_TRACE((	"ST>Display_GDI_Player_Terminal( )\n" ));

	PlayerTerminalClass::Get_Instance ()->Display_Terminal (COMBAT_STAR, PlayerTerminalClass::TYPE_GDI);
	return ;
}


/*
**
*/
void	Display_NOD_Player_Terminal (void)
{
	SCRIPT_TRACE((	"ST>Display_NOD_Player_Terminal( )\n" ));

	PlayerTerminalClass::Get_Instance ()->Display_Terminal (COMBAT_STAR, PlayerTerminalClass::TYPE_NOD);
	return ;
}


/*
**
*/
void	Display_Mutant_Player_Terminal (void)
{
	SCRIPT_TRACE((	"ST>Display_Mutant_Player_Terminal( )\n" ));

	PlayerTerminalClass::Get_Instance ()->Display_Terminal (COMBAT_STAR, PlayerTerminalClass::TYPE_MUTANT);
	return ;
}


/*
**
*/
bool	Reveal_Encyclopedia_Character( int object_id )
{
	SCRIPT_TRACE((	"ST>Reveal_Encyclopedia_Character( %d )\n", object_id ));
	return EncyclopediaMgrClass::Reveal_Object( EncyclopediaMgrClass::TYPE_CHARACTER, object_id );
}


/*
**
*/
bool	Reveal_Encyclopedia_Weapon( int object_id )
{
	SCRIPT_TRACE((	"ST>Reveal_Encyclopedia_Weapon( %d )\n", object_id ));
	return EncyclopediaMgrClass::Reveal_Object( EncyclopediaMgrClass::TYPE_WEAPON, object_id );
}


/*
**
*/
bool	Reveal_Encyclopedia_Vehicle( int object_id )
{
	SCRIPT_TRACE((	"ST>Reveal_Encyclopedia_Vehicle( %d )\n", object_id ));
	return EncyclopediaMgrClass::Reveal_Object( EncyclopediaMgrClass::TYPE_VEHICLE, object_id );
}


/*
**
*/
bool	Reveal_Encyclopedia_Building( int object_id )
{
	SCRIPT_TRACE((	"ST>Reveal_Encyclopedia_Building( %d )\n", object_id ));
	return EncyclopediaMgrClass::Reveal_Object( EncyclopediaMgrClass::TYPE_BUILDING, object_id );
}


/*
**
*/
void	Display_Encyclopedia_Event_UI( void )
{
	SCRIPT_TRACE((	"ST>Display_Encyclopedia_Event_UI( )\n" ));
	EncyclopediaMgrClass::Display_Event_UI();

	HUDClass::Add_Data_Link();
	return ;
}


/*
**
*/
void	Scale_AI_Awareness( float sight_scale, [[maybe_unused]] float hearing_scale )
{
	SCRIPT_TRACE((	"ST>Scale_AI_Awareness ( %f %f )\n", sight_scale, hearing_scale ));
	SmartGameObj::Set_Global_Sight_Range_Scale( sight_scale );
//	SoundSystem::Set_Global_Listener_Scale( hearing_scale );
}


/*
**
*/
void	Enable_Cinematic_Freeze( GameObject * object, bool enable )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Enable_Cinematic_Freeze( %d, %d )\n", object->Get_ID(), enable ));

	object->Enable_Cinematic_Freeze( enable );
}

void	Expire_Powerup( GameObject * object )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Expire_Powerup( %d )\n", object->Get_ID() ));

	PowerUpGameObj * powerup = NULL;
	if ( object->As_PhysicalGameObj() != NULL ) {
		powerup = object->As_PhysicalGameObj()->As_PowerUpGameObj();
	}
	if ( powerup ) {
		powerup->Expire();
	}
}

void	Set_HUD_Help_Text( int string_id, const Vector3 &color )
{
	SCRIPT_TRACE((	"ST>Set_HUD_Help_Text( %d )\n", string_id ));

	if (string_id == 0) {

		//
		//	Clear the help text
		//
		HUDInfo::Set_HUD_Help_Text( U_CHAR("") );
	} else {

		//
		//	Set the help text
		//
		HUDInfo::Set_HUD_Help_Text( TRANSLATE( string_id ), color );

		//
		//	Lookup the sound to play
		//
		int sound_id = GlobalSettingsDef::Get_Global_Settings ()->Get_HUD_Help_Text_Sound_ID ();
		if (sound_id != 0) {

			//
			//	Play the sound
			//
			WWAudioClass* audio = WWAudioClass::Get_Instance();
			assert(audio != NULL);
			audio->Create_Instant_Sound( sound_id, Matrix3D(1) );
		}
	}

	return ;
}

/*
**
*/
void	Enable_HUD_Pokable_Indicator( GameObject * object, bool enable )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Enable_HUD_Pokable_Indicator( %d, %d )\n", object->Get_ID(), enable ));
	if ( object->As_PhysicalGameObj() != NULL ) {
		object->As_PhysicalGameObj()->Enable_HUD_Pokable_Indicator( enable );
	} else {
		Debug_Say(( "Can only Enable_HUD_Pokable_Indicator on PhysicalGameObjs\n" ));
	}
}

void	Enable_Innate_Conversations( GameObject * object, bool enable )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Enable_Innate_Conversations( %d, %d )\n", object->Get_ID(), enable ));
	if ( object->As_PhysicalGameObj() != NULL ) {
		object->As_PhysicalGameObj()->Enable_Innate_Conversations( enable );
	} else {
		Debug_Say(( "Can only Enable_Innate_Conversations on PhysicalGameObjs\n" ));
	}
}

void	Display_Health_Bar( GameObject * object, bool display )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE((	"ST>Display_Health_Bar( %d, %d )\n", object->Get_ID(), display ));
	if ( object->As_DamageableGameObj() != NULL ) {
		object->As_DamageableGameObj()->Set_Is_Health_Bar_Displayed( display );
	} else {
		Debug_Say(( "Can only Display_Health_Bar on DamageableGameObjs\n" ));
	}
}

void	Enable_Shadow( GameObject * object, bool enable )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE(( "ST>Enable_Shadow( %d, %d )\n", object->Get_ID(), enable));
	if ( object->As_PhysicalGameObj() != NULL) {
		object->As_PhysicalGameObj()->Peek_Physical_Object()->Enable_Shadow_Generation(enable);
	} else {
		Debug_Say(( "Can only call Enable_Shadow on PhysicalGameObjs\n"));
	}
}

void	Clear_Weapons( GameObject * object )
{
	SCRIPT_PTR_CHECK( object );
	SCRIPT_TRACE(( "ST>Clear_Weapons( %d )\n", object->Get_ID() ));
	if ( object->As_SmartGameObj() != NULL) {
		object->As_SmartGameObj()->Get_Weapon_Bag()->Clear_Weapons();
	} else {
		Debug_Say(( "Can only call Clear_Weapons on SmartGameObjs\n"));
	}
}

void	Set_Num_Tertiary_Objectives( int count )
{
	SCRIPT_TRACE(( "ST>Set_Num_Tertiary_Objectives( %d )\n", count ));
	ObjectiveManager::Set_Num_Specified_Tertiary_Objectives( count );
}


void	Enable_Letterbox( bool onoff, float seconds )
{
	SCRIPT_TRACE(( "ST>Enable_Letterbox( %d, %f)\n", onoff,seconds ));
	ScreenFadeManager::Enable_Letterbox(onoff,seconds);
}

void	Set_Screen_Fade_Color( float r, float g, float b, float seconds )
{
	SCRIPT_TRACE(( "ST>Set_Screen_Fade_Color( %f, %f, %f, %f)\n", r,g,b,seconds ));
	ScreenFadeManager::Set_Screen_Overlay_Color(r,g,b,seconds);
}

void	Set_Screen_Fade_Opacity( float opacity, float seconds )
{
	SCRIPT_TRACE(( "ST>Set_Screen_Fade_Opacity( %f, %f)\n", opacity,seconds ));
	ScreenFadeManager::Set_Screen_Overlay_Opacity(opacity,seconds);
}



/*
**
*/
ScriptCommands* Get_Script_Commands( void )
{
	static ScriptCommands	EngineCommands;

	memset(&EngineCommands, 0, sizeof(EngineCommands));

	// Denzil 4/7/00 - Using the structure size automates the version check.
	EngineCommands.Size = sizeof(EngineCommands);
	EngineCommands.Version = SCRIPT_COMMANDS_VERSION;

	EngineCommands.Debug_Message = &Debug_Message;

	// Action Commands
	EngineCommands.Action_Reset = Action_Reset;
	EngineCommands.Action_Goto = Action_Goto;
	EngineCommands.Action_Attack = Action_Attack;
	EngineCommands.Action_Play_Animation = Action_Play_Animation;
	EngineCommands.Action_Enter_Exit = Action_Enter_Exit;
	EngineCommands.Action_Face_Location = Action_Face_Location;
	EngineCommands.Action_Dock = Action_Dock;
	EngineCommands.Action_Follow_Input = Action_Follow_Input;

	EngineCommands.Modify_Action = Modify_Action;

	EngineCommands.Set_Position = &Set_Position;
	EngineCommands.Get_Position = &Get_Position;
	EngineCommands.Get_Bone_Position = &Get_Bone_Position;
	EngineCommands.Get_Facing = &Get_Facing;
	EngineCommands.Set_Facing = &Set_Facing;

	EngineCommands.Disable_All_Collisions = &Disable_All_Collisions;
	EngineCommands.Disable_Physical_Collisions = &Disable_Physical_Collisions;
	EngineCommands.Enable_Collisions = &Enable_Collisions;

	EngineCommands.Destroy_Object = &Destroy_Object;
	EngineCommands.Find_Object = &Find_Object;
	EngineCommands.Create_Object = &Create_Object;
	EngineCommands.Create_Object_At_Bone = &Create_Object_At_Bone;
	EngineCommands.Attach_Script = Attach_Script;
	EngineCommands.Add_To_Dirty_Cull_List = Add_To_Dirty_Cull_List;

	EngineCommands.Get_ID = &Get_ID;
	EngineCommands.Get_Preset_ID = &Get_Preset_ID;
	EngineCommands.Get_Preset_Name = &Get_Preset_Name;
	EngineCommands.Start_Timer = &Start_Timer;
	EngineCommands.Trigger_Weapon = &Trigger_Weapon;
	EngineCommands.Select_Weapon = &Select_Weapon;
	EngineCommands.Send_Custom_Event = &Send_Custom_Event;
	EngineCommands.Send_Damaged_Event = &Send_Damaged_Event;
	EngineCommands.Get_Random = &Get_Random;
	EngineCommands.Get_Random_Int = &Get_Random_Int;
	EngineCommands.Find_Random_Simple_Object = &Find_Random_Simple_Object;
	EngineCommands.Set_Model = &Set_Model;
	EngineCommands.Set_Animation = &Set_Animation;
	EngineCommands.Set_Animation_Frame = &Set_Animation_Frame;
	EngineCommands.Create_Sound = Create_Sound;
	EngineCommands.Create_2D_Sound = Create_2D_Sound;
	EngineCommands.Create_2D_WAV_Sound = Create_2D_WAV_Sound;
	EngineCommands.Create_3D_WAV_Sound_At_Bone = Create_3D_WAV_Sound_At_Bone;
	EngineCommands.Create_3D_Sound_At_Bone = Create_3D_Sound_At_Bone;
	EngineCommands.Create_Logical_Sound = Create_Logical_Sound;
	EngineCommands.Set_Background_Music = Set_Background_Music;
	EngineCommands.Fade_Background_Music = Fade_Background_Music;
	EngineCommands.Stop_Background_Music = Stop_Background_Music;
	EngineCommands.Monitor_Sound = Monitor_Sound;
	EngineCommands.Stop_Sound = Stop_Sound;
	EngineCommands.Start_Sound = Start_Sound;
	EngineCommands.Get_Health = Get_Health;
	EngineCommands.Get_Max_Health = Get_Max_Health;
	EngineCommands.Set_Health = Set_Health;
	EngineCommands.Get_Shield_Strength = Get_Shield_Strength;
	EngineCommands.Get_Max_Shield_Strength = Get_Max_Shield_Strength;
	EngineCommands.Set_Shield_Strength = Set_Shield_Strength;
	EngineCommands.Set_Shield_Type = Set_Shield_Type;
	EngineCommands.Get_Player_Type = Get_Player_Type;
	EngineCommands.Set_Player_Type = Set_Player_Type;
	EngineCommands.Get_Distance = Get_Distance;
	EngineCommands.Set_Camera_Host = Set_Camera_Host;
	EngineCommands.Force_Camera_Look = Force_Camera_Look;

	EngineCommands.Get_The_Star = Get_The_Star;
	EngineCommands.Get_A_Star = Get_A_Star;

	EngineCommands.Find_Closest_Soldier = Find_Closest_Soldier;
	EngineCommands.Is_A_Star = Is_A_Star;

	EngineCommands.Control_Enable = Control_Enable;
	EngineCommands.Get_Damage_Bone_Name = Get_Damage_Bone_Name;
	EngineCommands.Get_Damage_Bone_Direction = Get_Damage_Bone_Direction;
	EngineCommands.Is_Object_Visible = Is_Object_Visible;
	EngineCommands.Enable_Enemy_Seen = Enable_Enemy_Seen;

	EngineCommands.Set_Display_Color = Set_Display_Color;
	EngineCommands.Display_Text = Display_Text;
	EngineCommands.Display_Float = Display_Float;
	EngineCommands.Display_Int = Display_Int;

	EngineCommands.Save_Data = Save_Data;
	EngineCommands.Save_Pointer = Save_Pointer;
	EngineCommands.Load_Begin = Load_Begin;
	EngineCommands.Load_Data = Load_Data;
	EngineCommands.Load_Pointer = Load_Pointer;
	EngineCommands.Load_End = Load_End;

	EngineCommands.Begin_Chunk = Begin_Chunk;
	EngineCommands.End_Chunk = End_Chunk;
	EngineCommands.Open_Chunk = Open_Chunk;
	EngineCommands.Close_Chunk = Close_Chunk;

	EngineCommands.Clear_Radar_Markers = Clear_Radar_Markers;
	EngineCommands.Clear_Radar_Marker = Clear_Radar_Marker;
	EngineCommands.Add_Radar_Marker = Add_Radar_Marker;
//	EngineCommands.Add_Obj_Radar_Marker = Add_Obj_Radar_Marker;
	EngineCommands.Set_Obj_Radar_Blip_Shape = Set_Obj_Radar_Blip_Shape;
	EngineCommands.Set_Obj_Radar_Blip_Color = Set_Obj_Radar_Blip_Color;
	EngineCommands.Enable_Radar = Enable_Radar;

	EngineCommands.Create_Explosion = Create_Explosion;
	EngineCommands.Create_Explosion_At_Bone = Create_Explosion_At_Bone;

	EngineCommands.Enable_HUD = Enable_HUD;
	EngineCommands.Mission_Complete = Mission_Complete;
	EngineCommands.Give_PowerUp = Give_PowerUp;

	EngineCommands.Innate_Disable = Innate_Disable;
	EngineCommands.Innate_Enable = Innate_Enable;
	EngineCommands.Innate_Soldier_Enable_Enemy_Seen = &Innate_Soldier_Enable_Enemy_Seen;
	EngineCommands.Innate_Soldier_Enable_Gunshot_Heard = &Innate_Soldier_Enable_Gunshot_Heard;
	EngineCommands.Innate_Soldier_Enable_Footsteps_Heard = &Innate_Soldier_Enable_Footsteps_Heard;
	EngineCommands.Innate_Soldier_Enable_Bullet_Heard = &Innate_Soldier_Enable_Bullet_Heard;
	EngineCommands.Innate_Soldier_Enable_Actions = &Innate_Soldier_Enable_Actions;
	EngineCommands.Set_Innate_Soldier_Home_Location = Set_Innate_Soldier_Home_Location;
	EngineCommands.Set_Innate_Aggressiveness = Set_Innate_Aggressiveness;
	EngineCommands.Set_Innate_Take_Cover_Probability = Set_Innate_Take_Cover_Probability;
	EngineCommands.Set_Innate_Is_Stationary = Set_Innate_Is_Stationary;
	EngineCommands.Innate_Force_State_Bullet_Heard = &Innate_Force_State_Bullet_Heard;
	EngineCommands.Innate_Force_State_Footsteps_Heard = &Innate_Force_State_Footsteps_Heard;
	EngineCommands.Innate_Force_State_Gunshots_Heard = &Innate_Force_State_Gunshots_Heard;
	EngineCommands.Innate_Force_State_Enemy_Seen = &Innate_Force_State_Enemy_Seen;

	EngineCommands.Static_Anim_Phys_Goto_Frame = &Static_Anim_Phys_Goto_Frame;
	EngineCommands.Static_Anim_Phys_Goto_Last_Frame = &Static_Anim_Phys_Goto_Last_Frame;

	EngineCommands.Get_Sync_Time = &Get_Sync_Time;

	EngineCommands.Add_Objective = &Add_Objective;
	EngineCommands.Remove_Objective = &Remove_Objective;
	EngineCommands.Set_Objective_Status = &Set_Objective_Status;
	EngineCommands.Change_Objective_Type = &Change_Objective_Type;
	EngineCommands.Set_Objective_Radar_Blip = &Set_Objective_Radar_Blip;
	EngineCommands.Set_Objective_Radar_Blip_Object = &Set_Objective_Radar_Blip_Object;
	EngineCommands.Set_Objective_HUD_Info = &Set_Objective_HUD_Info;
	EngineCommands.Set_Objective_HUD_Info_Position = &Set_Objective_HUD_Info_Position;

	EngineCommands.Shake_Camera = Shake_Camera;

	EngineCommands.Enable_Spawner = Enable_Spawner;
	EngineCommands.Trigger_Spawner = Trigger_Spawner;

	EngineCommands.Enable_Engine = Enable_Engine;
	EngineCommands.Get_Difficulty_Level = Get_Difficulty_Level;

	EngineCommands.Grant_Key = Grant_Key;
	EngineCommands.Has_Key = Has_Key;
	EngineCommands.Enable_Hibernation = Enable_Hibernation;
	EngineCommands.Attach_To_Object_Bone = Attach_To_Object_Bone;

	EngineCommands.Create_Conversation			= Create_Conversation;
	EngineCommands.Join_Conversation				= Join_Conversation;
	EngineCommands.Join_Conversation_Facing	= Join_Conversation_Facing;
	EngineCommands.Start_Conversation			= Start_Conversation;
	EngineCommands.Monitor_Conversation			= Monitor_Conversation;
	EngineCommands.Start_Random_Conversation	= Start_Random_Conversation;
	EngineCommands.Stop_Conversation				= Stop_Conversation;
	EngineCommands.Stop_All_Conversations		= Stop_All_Conversations;

	EngineCommands.Apply_Damage					= Apply_Damage;
	EngineCommands.Set_Loiters_Allowed			= Set_Loiters_Allowed;

	EngineCommands.Set_Is_Visible					= Set_Is_Visible;
	EngineCommands.Set_Is_Rendered				= Set_Is_Rendered;

	EngineCommands.Get_Points						= Get_Points;
	EngineCommands.Give_Points						= Give_Points;

	EngineCommands.Get_Money						= Get_Money;
	EngineCommands.Give_Money						= Give_Money;

	EngineCommands.Get_Building_Power				= Get_Building_Power;
	EngineCommands.Set_Building_Power				= Set_Building_Power;
	EngineCommands.Play_Building_Announcement		= Play_Building_Announcement;
	EngineCommands.Find_Nearest_Building			= Find_Nearest_Building;
	EngineCommands.Find_Nearest_Building_To_Pos	= Find_Nearest_Building_To_Pos;

	EngineCommands.Team_Members_In_Zone			= Team_Members_In_Zone;

	EngineCommands.Set_Clouds						= Set_Clouds;
	EngineCommands.Set_Lightning					= Set_Lightning;
	EngineCommands.Set_War_Blitz					= Set_War_Blitz;

	EngineCommands.Set_Wind							= Set_Wind;
	EngineCommands.Set_Rain							= Set_Rain;
	EngineCommands.Set_Snow							= Set_Snow;
	EngineCommands.Set_Ash							= Set_Ash;
	EngineCommands.Set_Fog_Enable					= Set_Fog_Enable;
	EngineCommands.Set_Fog_Range					= Set_Fog_Range;

	EngineCommands.Clear_Map_Cell						= Clear_Map_Cell;
	EngineCommands.Clear_Map_Cell_By_Pos			= Clear_Map_Cell_By_Pos;
	EngineCommands.Clear_Map_Cell_By_Pixel_Pos	= Clear_Map_Cell_By_Pixel_Pos;
	EngineCommands.Clear_Map_Region_By_Pos			= Clear_Map_Region_By_Pos;
	EngineCommands.Reveal_Map							= Reveal_Map;
	EngineCommands.Shroud_Map							= Shroud_Map;
	EngineCommands.Show_Player_Map_Marker			= Show_Player_Map_Marker;

	EngineCommands.Get_Safe_Flight_Height		= Get_Safe_Flight_Height;

	EngineCommands.Enable_Stealth					= Enable_Stealth;
	EngineCommands.Cinematic_Sniper_Control	= Cinematic_Sniper_Control;

	EngineCommands.Text_File_Open					= Text_File_Open;
	EngineCommands.Text_File_Get_String			= Text_File_Get_String;
	EngineCommands.Text_File_Close				= Text_File_Close;

	EngineCommands.Enable_Vehicle_Transitions	= Enable_Vehicle_Transitions;

	EngineCommands.Display_GDI_Player_Terminal		= Display_GDI_Player_Terminal;
	EngineCommands.Display_NOD_Player_Terminal		= Display_NOD_Player_Terminal;
	EngineCommands.Display_Mutant_Player_Terminal	= Display_Mutant_Player_Terminal;

	EngineCommands.Reveal_Encyclopedia_Character		= Reveal_Encyclopedia_Character;
	EngineCommands.Reveal_Encyclopedia_Weapon			= Reveal_Encyclopedia_Weapon;
	EngineCommands.Reveal_Encyclopedia_Vehicle		= Reveal_Encyclopedia_Vehicle;
	EngineCommands.Reveal_Encyclopedia_Building		= Reveal_Encyclopedia_Building;
	EngineCommands.Display_Encyclopedia_Event_UI		= Display_Encyclopedia_Event_UI;

	EngineCommands.Scale_AI_Awareness			=	Scale_AI_Awareness;
	EngineCommands.Enable_Cinematic_Freeze		= Enable_Cinematic_Freeze;
	EngineCommands.Expire_Powerup					= Expire_Powerup;

	EngineCommands.Get_Action_ID							= Get_Action_ID;
	EngineCommands.Get_Action_Params						= Get_Action_Params;
	EngineCommands.Is_Performing_Pathfind_Action		= Is_Performing_Pathfind_Action;

	EngineCommands.Set_HUD_Help_Text				= Set_HUD_Help_Text;
	EngineCommands.Enable_HUD_Pokable_Indicator	= Enable_HUD_Pokable_Indicator;

	EngineCommands.Enable_Innate_Conversations	= Enable_Innate_Conversations;

	EngineCommands.Lock_Soldier_Facing				= Lock_Soldier_Facing;
	EngineCommands.Unlock_Soldier_Facing			= Unlock_Soldier_Facing;

	EngineCommands.Display_Health_Bar				= Display_Health_Bar;
	EngineCommands.Enable_Shadow						= Enable_Shadow;

	EngineCommands.Clear_Weapons						= Clear_Weapons;

	EngineCommands.Set_Num_Tertiary_Objectives	= Set_Num_Tertiary_Objectives;

	EngineCommands.Enable_Letterbox					= Enable_Letterbox;
	EngineCommands.Set_Screen_Fade_Color			= Set_Screen_Fade_Color;
	EngineCommands.Set_Screen_Fade_Opacity			= Set_Screen_Fade_Opacity;

	return &EngineCommands;
}