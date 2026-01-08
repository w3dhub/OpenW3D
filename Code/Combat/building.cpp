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
 *                     $Archive:: /Commando/Code/Combat/building.cpp                          $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 6/20/02 3:09p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 68                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "building.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "debug.h"
#include "wwphysids.h"
#include "meshmdl.h"
#include "gameobjmanager.h"
#include "WWAudio.h"
#include "AudibleSound.h"
#include "Sound3D.h"
#include "translateobj.h"
#include "translatedb.h"
#include "combat.h"
#include "messagewindow.h"
#include "buildingmonitor.h"
#include "basecontroller.h"
#include "bitpackids.h"
#include "soldier.h"
#include "playerdata.h"
#include "encyclopediamgr.h"
#include "apppackettypes.h"
#include "wwprofile.h"
#include <algorithm>


/////////////////////////////////////////////////////////////////////////////
//	Namespaces
/////////////////////////////////////////////////////////////////////////////
using namespace BuildingConstants;


/////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
/////////////////////////////////////////////////////////////////////////////
static const char *	BULDING_TYPE_NAMES[TYPE_COUNT + 1] =
{
	"<None>",
	"Power Plant",
	"Soldier Factory",
	"Vehicle Factory",
	"Refinery",
	"Com Center",
	"Repair Bay",
	"Shrine",
	"Helipad",
	"Conyard",
	"Base Defense",
};


static const char *	BULDING_TEAM_NAMES[BASE_COUNT] =
{
	"GDI",
	"NOD"
};

//
// Class statics
//
bool	BuildingGameObj::CanRepairBuildings	= true;


/************************************************************************************************
**
** BuildingGameObjDef
**
************************************************************************************************/
SimplePersistFactoryClass<BuildingGameObjDef, CHUNKID_GAME_OBJECT_DEF_BUILDING>	_BuildingGameObjDefPersistFactory;

DECLARE_DEFINITION_FACTORY(BuildingGameObjDef, CLASSID_GAME_OBJECT_DEF_BUILDING, "<Generic Building>") _BuildingGameObjDefDefFactory;


/////////////////////////////////////////////////////////////////////////////
//
//	BuildingGameObjDef
//
/////////////////////////////////////////////////////////////////////////////
BuildingGameObjDef::BuildingGameObjDef (void)	:
	MCTSkin(0),
	Type(TYPE_NONE),
	GDIDamageReportID(0),
	NodDamageReportID(0),
	GDIDestroyReportID(0),
	NodDestroyReportID(0)
{
	EDITABLE_PARAM( BuildingGameObjDef, ParameterClass::TYPE_STRING,	MeshPrefix );

#ifdef PARAM_EDITING_ON
	int skin_type_counter;
	EnumParameterClass * mct_skin_param = new EnumParameterClass( (int*)&MCTSkin );												
	mct_skin_param->Set_Name("MCTSkin");																																
	for ( skin_type_counter = 0; skin_type_counter < ArmorWarheadManager::Get_Num_Armor_Types(); skin_type_counter++ ) {																			
		mct_skin_param->Add_Value(ArmorWarheadManager::Get_Armor_Name(skin_type_counter), skin_type_counter);
	}																																									
	GENERIC_EDITABLE_PARAM(BuildingGameObjDef,mct_skin_param);
#endif

//
//	Configure the building type parameter
//
#ifdef PARAM_EDITING_ON
	EnumParameterClass *building_type_param = new EnumParameterClass( (int*)&Type );
	building_type_param->Set_Name("Building Type");
	for ( int index = TYPE_NONE; index < TYPE_COUNT; index++ ) {																			
		building_type_param->Add_Value(BULDING_TYPE_NAMES[index+1], index);
	}																																									
	GENERIC_EDITABLE_PARAM(BuildingGameObjDef,building_type_param);
#endif

	EDITABLE_PARAM(BuildingGameObjDef, ParameterClass::TYPE_STRINGSDB_ID, GDIDamageReportID);
	EDITABLE_PARAM(BuildingGameObjDef, ParameterClass::TYPE_STRINGSDB_ID, NodDamageReportID);
	EDITABLE_PARAM(BuildingGameObjDef, ParameterClass::TYPE_STRINGSDB_ID, GDIDestroyReportID);
	EDITABLE_PARAM(BuildingGameObjDef, ParameterClass::TYPE_STRINGSDB_ID, NodDestroyReportID);
}


/////////////////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
/////////////////////////////////////////////////////////////////////////////
uint32
BuildingGameObjDef::Get_Class_ID (void) const	
{ 
	return CLASSID_GAME_OBJECT_DEF_BUILDING; 
}


/////////////////////////////////////////////////////////////////////////////
//
//	Create
//
/////////////////////////////////////////////////////////////////////////////
PersistClass *
BuildingGameObjDef::Create (void) const 
{
	BuildingGameObj * obj = new BuildingGameObj;
	obj->Init( *this );
	return obj;
}


/////////////////////////////////////////////////////////////////////////////
//	Save/Load constants
/////////////////////////////////////////////////////////////////////////////
enum	{
	CHUNKID_DEF_PARENT							=	207011030,
	CHUNKID_DEF_VARIABLES,

	MICROCHUNKID_DEF_MESHPREFIX				= 1,
	MICROCHUNKID_DEF_MCTSKIN,
	MICROCHUNKID_DEF_BUILDING_TYPE,
	LEGACY_MICROCHUNKID_DEF_BUILDING_TEAM,
	MICROCHUNKID_DEF_GDI_DAMAGE_REPORT_ID, 
	MICROCHUNKID_DEF_NOD_DAMAGE_REPORT_ID, 
	MICROCHUNKID_DEF_GDI_DESTROY_REPORT_ID, 
	MICROCHUNKID_DEF_NOD_DESTROY_REPORT_ID, 
};


/////////////////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////////////////
bool
BuildingGameObjDef::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk( CHUNKID_DEF_PARENT );
		DamageableGameObjDef::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_DEF_VARIABLES );
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_DEF_MESHPREFIX, MeshPrefix );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_MCTSKIN, MCTSkin );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_BUILDING_TYPE, Type );		
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GDI_DAMAGE_REPORT_ID, GDIDamageReportID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_NOD_DAMAGE_REPORT_ID, NodDamageReportID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_GDI_DESTROY_REPORT_ID, GDIDestroyReportID );
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_DEF_NOD_DESTROY_REPORT_ID, NodDestroyReportID );
	csave.End_Chunk();

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////////////////
bool
BuildingGameObjDef::Load (ChunkLoadClass &cload)
{
	int legacy_team = -1;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_DEF_PARENT:
				DamageableGameObjDef::Load( cload );
				break;
	  
			case CHUNKID_DEF_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK_WWSTRING( cload, MICROCHUNKID_DEF_MESHPREFIX, MeshPrefix );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_MCTSKIN, MCTSkin );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_BUILDING_TYPE, Type );		
						READ_MICRO_CHUNK( cload, LEGACY_MICROCHUNKID_DEF_BUILDING_TEAM, legacy_team );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GDI_DAMAGE_REPORT_ID, GDIDamageReportID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_NOD_DAMAGE_REPORT_ID, NodDamageReportID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_GDI_DESTROY_REPORT_ID, GDIDestroyReportID );
						READ_MICRO_CHUNK( cload, MICROCHUNKID_DEF_NOD_DESTROY_REPORT_ID, NodDestroyReportID );

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	if ( legacy_team != -1 ) {
		DefaultPlayerType = PLAYERTYPE_GDI;
		if ( legacy_team == BuildingConstants::LEGACY_TEAM_NOD ) {
			DefaultPlayerType = PLAYERTYPE_NOD;
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
/////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
BuildingGameObjDef::Get_Factory (void) const 
{ 
	return _BuildingGameObjDefPersistFactory; 
}


int BuildingGameObjDef::Get_Damage_Report(int team) const
{
	if (PLAYERTYPE_GDI == team) {
		return GDIDamageReportID;
	} else if (PLAYERTYPE_NOD == team) {
		return NodDamageReportID;
	}

	return 0;
}


int BuildingGameObjDef::Get_Destroy_Report(int team) const
{
	if (PLAYERTYPE_GDI == team) {
		return GDIDestroyReportID;
	} else if (PLAYERTYPE_NOD == team) {
		return NodDestroyReportID;
	}

	return 0;
}



/************************************************************************************************
**
** BuildingGameObj
**
************************************************************************************************/
SimplePersistFactoryClass<BuildingGameObj, CHUNKID_GAME_OBJECT_BUILDING>	_BuildingGameObjPersistFactory;


/////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
/////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
BuildingGameObj::Get_Factory (void) const 
{
	return _BuildingGameObjPersistFactory;
}


/////////////////////////////////////////////////////////////////////////////
//
//	BuildingGameObj
//
/////////////////////////////////////////////////////////////////////////////
BuildingGameObj::BuildingGameObj (void)	:
	Position (0, 0, 0),
	IsPowerOn (true),
	CurrentState (-1),
	AnnouncementSphere (Vector3 (0, 0, 0), 1.0F),
	CollectionSphere (Vector3 (0, 0, 0), 50.0F),
	CurrentAnnouncement (NULL),
	IsDestroyed (false),
	BuildingMonitor (NULL),
	BaseController (NULL)
{
	GameObjManager::Add_Building (this);
	Update_State ();
	Set_App_Packet_Type(APPPACKETTYPE_BUILDING);
}


/////////////////////////////////////////////////////////////////////////////
//
//	~BuildingGameObj
//
/////////////////////////////////////////////////////////////////////////////
BuildingGameObj::~BuildingGameObj (void)
{
	Stop_Current_Announcement();

	//
	//	Free the building monitor (if we have one)
	//
	if (BuildingMonitor != NULL) {
		Remove_Observer (BuildingMonitor);
		delete BuildingMonitor;
		BuildingMonitor = NULL;
	}

	GameObjManager::Remove_Building( this );
	Reset_Components();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Init
//
/////////////////////////////////////////////////////////////////////////////
void BuildingGameObj::Init( void )
{
	Init( Get_Definition() );
}


/////////////////////////////////////////////////////////////////////////////
//
//	Init
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Init (const BuildingGameObjDef &definition)
{
	DamageableGameObj::Init (definition);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Get_Definition
//
/////////////////////////////////////////////////////////////////////////////
const BuildingGameObjDef &
BuildingGameObj::Get_Definition (void) const
{
	return (const BuildingGameObjDef &)BaseGameObj::Get_Definition();
}


/////////////////////////////////////////////////////////////////////////////
//	Save/Load constants
/////////////////////////////////////////////////////////////////////////////
enum	{
	CHUNKID_PARENT							=	207011120,
	CHUNKID_VARIABLES,

	MICROCHUNKID_POSITION				=	1,
	MICROCHUNKID_ISPOWERON,
	MICROCHUNKID_COLLECTION_SPHERE
};


/////////////////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////////////////
bool
BuildingGameObj::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		DamageableGameObj::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_POSITION,				Position);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_ISPOWERON,				IsPowerOn);
		WRITE_MICRO_CHUNK (csave, MICROCHUNKID_COLLECTION_SPHERE,	CollectionSphere);		
	csave.End_Chunk ();

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////////////////
bool
BuildingGameObj::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				DamageableGameObj::Load (cload);
				break;
	  
			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk ()) {
					switch (cload.Cur_Micro_Chunk_ID ()) {
						
						READ_MICRO_CHUNK (cload, MICROCHUNKID_POSITION,				Position);
						READ_MICRO_CHUNK (cload, MICROCHUNKID_ISPOWERON,			IsPowerOn);
						READ_MICRO_CHUNK (cload, MICROCHUNKID_COLLECTION_SPHERE,	CollectionSphere);

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID(),__FILE__,__LINE__));
							break;

					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}

		cload.Close_Chunk ();
	}

	// Fix player_type (legacy)
	Set_Player_Type( Get_Definition().DefaultPlayerType );

	//
	//	Hack to get legacy code to work...
	//
	if (CollectionSphere.Center.X == 0 && CollectionSphere.Center.Y == 0 && CollectionSphere.Center.Z == 0) {
		CollectionSphere.Center = Position;
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Enable_Power
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Enable_Power (bool onoff)
{
	if ( IsPowerOn != onoff ) {
		// Notify the observers
		const GameObjObserverList & observer_list = Get_Observers();
		for( int index = 0; index < observer_list.Count(); index++ ) {
			observer_list[ index ]->Custom( this, CUSTOM_EVENT_BUILDING_POWER_CHANGED, onoff, NULL );
		}
	}

	IsPowerOn = onoff;

	Update_State ();

	//
	//	Mark the object as dirty so it's state will be mirrored on the client
	//
	if (CombatManager::I_Am_Server ()) {
		Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
	}

	return ;
}

void BuildingGameObj::Apply_Damage( const OffenseObjectClass & damager, float scale, int alternate_skin )
{
	/*
	** If we're already completely damaged, just return
	*/
	if ( DefenseObject.Get_Health() <= 0 ) {
		return;
	}

	// if gameplay not permitted, skip
	if ( !CombatManager::Is_Gameplay_Permitted() ) {
		return;
	}
	//
	// TSS102501
	//
	if ( !CanRepairBuildings && DefenseObject.Is_Repair(damager) ) {
		return;
	}

	float old_health = Get_Defense_Object()->Get_Health();

	DamageableGameObj::Apply_Damage( damager, scale, alternate_skin );

	if ( old_health != Get_Defense_Object()->Get_Health() ) {
		Set_Object_Dirty_Bit( NetworkObjectClass::BIT_OCCASIONAL, true );
	}

//	WWDEBUG_SAY(("Applied damage to a building (prefix=%s) new health = %f\r\n",Get_Definition().MeshPrefix,DefenseObject.Get_Health()));

	// Stats
	if ( DefenseObject.Get_Health() <= 0 ) {
		if ( damager.Get_Owner() != NULL && 
			  damager.Get_Owner()->As_SoldierGameObj() != NULL && 
			  damager.Get_Owner()->As_SoldierGameObj()->Get_Player_Data() != NULL ) {
			damager.Get_Owner()->As_SoldierGameObj()->Get_Player_Data()->Stats_Add_Building_Destroyed();
		}

		//
		//	Reveal this building to the encyclopedia
		//
		EncyclopediaMgrClass::Reveal_Object (this);
	}

	/*
	** If our health status changed states, switch all of our aggregates
	*/
	Update_State();
}

void BuildingGameObj::Apply_Damage_Building(const OffenseObjectClass & offense, StaticPhysClass * component )
{
	/*
	** If the MCT is being damaged, use an alternate skin type
	*/
	ArmorType skin = DefenseObject.Get_Skin();
	if (component->Get_Factory().Chunk_ID() == PHYSICS_CHUNKID_BUILDINGAGGREGATE) {
		if (((BuildingAggregateClass *)component)->Is_MCT()) {
			skin = Get_Definition().MCTSkin;
		}
	}

	/*
	** Apply the damage
	*/
	Apply_Damage( offense, 1.0, skin );
}

void  BuildingGameObj::Apply_Damage_Building(	const OffenseObjectClass & offense,	bool mct_damage )
{
	/*
	** If the MCT is being damaged, use an alternate skin type
	*/
	ArmorType skin = DefenseObject.Get_Skin();
	if ( mct_damage ) {
		skin = Get_Definition().MCTSkin;
	}

	/*
	** Apply the damage
	*/
	Apply_Damage( offense, 1.0, skin );
}

/////////////////////////////////////////////////////////////////////////////
//
//	Is_Interior_Mesh_Name
//
/////////////////////////////////////////////////////////////////////////////
bool
BuildingGameObj::Is_Interior_Mesh_Name (const char * name)
{
// (gth) Renegade day 120 patch, accept tiles as building "parts"
#if 0
	char * meshname = strchr(name,'.');
	if (meshname != NULL) {
		return strchr(meshname,'#') != NULL;
	} else {
		return false;
	}
#else
	return strchr(name,'#') != NULL;
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//	Is_Exterior_Mesh_Name
//
/////////////////////////////////////////////////////////////////////////////
bool
BuildingGameObj::Is_Exterior_Mesh_Name (const char * name)
{
// (gth) Renegade day 120 patch, accept tiles as building "parts"
#if 0
	char * meshname = strchr(name,'.');
	if (meshname != NULL) {
		return strchr(meshname,'^') != NULL;
	} else {
		return false;
	}
#else
	return strchr(name,'^') != NULL;
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//	Name_Prefix_Matches_This_Building
//
/////////////////////////////////////////////////////////////////////////////
bool
BuildingGameObj::Name_Prefix_Matches_This_Building (const char * name)
{
	bool retval = false;

	if (name != NULL) {
		StringClass prefex(Get_Definition().MeshPrefix,true);
		const char * meshname = strchr(name,'.');
		if (meshname != NULL) {
			meshname++;
			retval = (strnicmp(meshname,prefex,strlen(prefex)) == 0);
		} else {
			retval = (strnicmp(name,prefex,strlen(prefex)) == 0);
		}
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Set_Normalized_Health
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Set_Normalized_Health (float health)
{
	//
	//	Note:  We convert from normalized to absolute values
	//
	DefenseObject.Set_Health (health * DefenseObject.Get_Health_Max ());
	Update_State ();


	WWDEBUG_SAY(("Building State Set:\r\n"));
	WWDEBUG_SAY(("  prefix = %s mesh count = %d aggregate count = %d light count = %d\r\n",
		Get_Definition().MeshPrefix,
		InteriorMeshes.Count() + ExteriorMeshes.Count(),
		Aggregates.Count(),
		PowerOnLights.Count() + PowerOffLights.Count()));
}


/////////////////////////////////////////////////////////////////////////////
//
//	Update_State
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Update_State (bool force_update)
{
	/*
	** Evaluate our current state
	*/
	float health_percentage = 100.0f * DefenseObject.Get_Health()/DefenseObject.Get_Health_Max();
	int health_state = BuildingStateClass::Percentage_To_Health_State(health_percentage);
	int new_state = BuildingStateClass::Compose_State(health_state,IsPowerOn);
	
	/*
	** If the state has changed, inform all of our components
	*/
	if ((new_state != CurrentState) || (force_update == true)) {

		int old_health_state = BuildingStateClass::Get_Health_State( CurrentState );

		if (health_state != old_health_state) {
			int event = CUSTOM_EVENT_BUILDING_DAMAGED;
			if (health_state < old_health_state) {
				event = CUSTOM_EVENT_BUILDING_REPAIRED;
			}

			const GameObjObserverList & observer_list = Get_Observers();
			for( int index = 0; index < observer_list.Count(); index++ ) {
				observer_list[ index ]->Custom( this, event, health_percentage, NULL );
			}
		}

		CurrentState = new_state;

		//
		//	Mark the object as dirty so it's state will be mirrored on the client
		//
		if (CombatManager::I_Am_Server ()) {
			Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
		}

		/*
		** Aggregates: inform all aggregates of the new state
		*/
		RefMultiListIterator<BuildingAggregateClass> aggregate_iterator(&Aggregates);
		for (aggregate_iterator.First(); !aggregate_iterator.Is_Done(); aggregate_iterator.Next()) {
			aggregate_iterator.Peek_Obj()->Set_Current_State(CurrentState,force_update);
		}

		/*
		** Interior meshes: switch to alternate materials if either the power is out or
		** the building is at zero health
		*/
		Enable_Alternate_Materials(InteriorMeshes,((health_state == BuildingStateClass::HEALTH_0) || (!IsPowerOn)));
		

		/*
		** Exterior meshes: use alternate materials if the building is destroyed
		*/
		Enable_Alternate_Materials(ExteriorMeshes,(health_state == BuildingStateClass::HEALTH_0));

		/*
		** Lights: enable the lights which match the current power status, invalidate all
		** lighting caches within the bounds of the lights that changed.
		*/
		bool disable = ((IsPowerOn == false) || (health_state == BuildingStateClass::HEALTH_0));
		disable &= (!PowerOffLights.Is_Empty());	// for lights, power is not disabled if there are not power-off lights

		AABoxClass light_bounds(Position,Vector3(1,1,1));
		
		RefMultiListIterator<LightPhysClass> light_iterator(&PowerOnLights);
		for (light_iterator.First(); !light_iterator.Is_Done(); light_iterator.Next()) {
			light_iterator.Peek_Obj()->Set_Disabled(disable);
			light_bounds.Add_Box(light_iterator.Peek_Obj()->Peek_Model()->Get_Bounding_Box());
		}

		disable = !disable;
		for (light_iterator.First(&PowerOffLights); !light_iterator.Is_Done(); light_iterator.Next()) {
			light_iterator.Peek_Obj()->Set_Disabled(disable);
			light_bounds.Add_Box(light_iterator.Peek_Obj()->Peek_Model()->Get_Bounding_Box());
		}

		PhysicsSceneClass::Get_Instance()->Invalidate_Lighting_Caches(light_bounds);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Play_Announcement
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Play_Announcement(int text_id, bool broadcast)
{
	Stop_Current_Announcement();

	// Lookup the translation object from the strings database
	TDBObjClass* translate_obj = TranslateDBClass::Find_Object(text_id);

	if (translate_obj) {
		const unichar_t* string = translate_obj->Get_String();
		int sound_def_id = (int)translate_obj->Get_Sound_ID();
		float duration = 2.0F;

		//	Play the sound effect
		bool display_text = true;

		if (sound_def_id > 0) {
			
			//	Create the sound object
			CurrentAnnouncement = WWAudioClass::Get_Instance()->Create_Sound(sound_def_id);

			if (CurrentAnnouncement) {
				duration = (CurrentAnnouncement->Get_Duration() / 1000.0F);

				if (broadcast == false) {
					// Tweak the radii of the sound so it can be heard throughout the building
					CurrentAnnouncement->Set_DropOff_Radius(AnnouncementSphere.Radius);
					Sound3DClass* sound3D = CurrentAnnouncement->As_Sound3DClass();

					if (sound3D) {
						sound3D->Set_Max_Vol_Radius(AnnouncementSphere.Radius * 0.9F);
					}

					//	Play the sound effect at the annoucement position for the building
					CurrentAnnouncement->Set_Transform(Matrix3D(AnnouncementSphere.Center));
					display_text = (CurrentAnnouncement->Is_Sound_Culled() == false);
				}

				CurrentAnnouncement->Add_To_Scene();
			}
		}

		//	Display the text on the screen
		if (display_text && string) {
			float message_duration = std::max(duration, 5.0F);
			CombatManager::Get_Message_Window()->Add_Message(string, Vector3(1, 1, 1), NULL, message_duration);
		}		
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//	Stop_Current_Announcement
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Stop_Current_Announcement (void)
{
	if (CurrentAnnouncement) {		
		
		//	Stop the sound object and remove it from the world
		CurrentAnnouncement->Stop();
		CurrentAnnouncement->Remove_From_Scene();
		CurrentAnnouncement->Release_Ref();
		CurrentAnnouncement = NULL;
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//	Initialize_Building
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Initialize_Building (void)
{
	RefMultiListIterator<StaticPhysClass> mesh_iterator (&InteriorMeshes);

	//
	//	Generate a bounding box that contains all the building interior meshes
	//
	AABoxClass bounding_box (Position, Vector3 (1, 1, 1));
	for (mesh_iterator.First (); !mesh_iterator.Is_Done (); mesh_iterator.Next ()) {
		bounding_box.Add_Box (mesh_iterator.Peek_Obj ()->Peek_Model ()->Get_Bounding_Box());
	}

	//
	//	Turn the bounding box into a sphere that we can use to play building
	// announcement sound effects in
	//
	AnnouncementSphere.Center	= bounding_box.Center;
	AnnouncementSphere.Radius	= std::max (bounding_box.Extent.X, bounding_box.Extent.Y);
	AnnouncementSphere.Radius	= std::max (AnnouncementSphere.Radius, bounding_box.Extent.Z);
	
	//
	//	If we can find a base for our team, then add ourselves to it.
	//
	BaseControllerClass *base = BaseControllerClass::Find_Base ( Get_Player_Type() );
	if (base != NULL) {
		base->Add_Building (this);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Reset_Components
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Reset_Components (void)
{
	RefMultiListIterator<StaticPhysClass> mesh_iterator(&InteriorMeshes);
	
	for (mesh_iterator.First(); !mesh_iterator.Is_Done(); mesh_iterator.Next()) {
		WWASSERT(mesh_iterator.Peek_Obj()->Get_Observer() == this);
		mesh_iterator.Peek_Obj()->Set_Observer(NULL);
	}
	for (mesh_iterator.First(&ExteriorMeshes); !mesh_iterator.Is_Done(); mesh_iterator.Next()) {
		WWASSERT(mesh_iterator.Peek_Obj()->Get_Observer() == this);
		mesh_iterator.Peek_Obj()->Set_Observer(NULL);
	}

	RefMultiListIterator<BuildingAggregateClass> agg_iterator(&Aggregates);
	for (agg_iterator.First(); !agg_iterator.Is_Done(); agg_iterator.Next()) {
		WWASSERT(agg_iterator.Peek_Obj()->Get_Observer() == this);
		agg_iterator.Peek_Obj()->Set_Observer(NULL);
	}
						  
	InteriorMeshes.Reset_List();
	ExteriorMeshes.Reset_List();
	PowerOnLights.Reset_List();
	PowerOffLights.Reset_List();
	Aggregates.Reset_List();
}


/////////////////////////////////////////////////////////////////////////////
//
//	Add_Mesh
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Add_Mesh (StaticPhysClass * terrain)
{
	WWASSERT(terrain != NULL);
	if (Is_Interior_Mesh_Name(terrain->Peek_Model()->Get_Name())) {
		WWASSERT(terrain->Get_Observer() == NULL);
		InteriorMeshes.Add(terrain);
		terrain->Set_Observer(this);
	} else if (Is_Exterior_Mesh_Name(terrain->Peek_Model()->Get_Name())) {
		WWASSERT(terrain->Get_Observer() == NULL);
		ExteriorMeshes.Add(terrain);
		terrain->Set_Observer(this);
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//	Remove_Mesh
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Remove_Mesh (StaticPhysClass * terrain)
{
	WWASSERT(terrain != NULL);
	if (Is_Interior_Mesh_Name(terrain->Peek_Model()->Get_Name())) {
		WWASSERT(terrain->Get_Observer() == this);
		terrain->Set_Observer(NULL);
		InteriorMeshes.Remove(terrain);
	} else if (Is_Exterior_Mesh_Name(terrain->Peek_Model()->Get_Name())) {
		WWASSERT(terrain->Get_Observer() == this);
		terrain->Set_Observer(NULL);
		ExteriorMeshes.Remove(terrain);
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//	Add_Aggregate
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Add_Aggregate (BuildingAggregateClass * aggregate)
{
	WWASSERT(aggregate != NULL);
	Aggregates.Add(aggregate);
	WWASSERT(aggregate->Get_Observer() == NULL);
	aggregate->Set_Observer(this);
}


/////////////////////////////////////////////////////////////////////////////
//
//	Remove_Aggregate
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Remove_Aggregate (BuildingAggregateClass * aggregate)
{
	WWASSERT(aggregate != NULL);
	WWASSERT(aggregate->Get_Observer() == this);
	aggregate->Set_Observer(NULL);
	Aggregates.Remove(aggregate);
}


/////////////////////////////////////////////////////////////////////////////
//
//	Add_Light
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Add_Light (LightPhysClass * light)
{
	WWASSERT(light != NULL);
	if (light->Get_Group_ID() == 0) {
		PowerOnLights.Add(light);
	} else {
		PowerOffLights.Add(light);
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//	Enable_Alternate_Materials
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Enable_Alternate_Materials (RefMultiListClass<StaticPhysClass> & models, bool onoff)
{
	RefMultiListIterator<StaticPhysClass> it(&models);
	for (it.First(); !it.Is_Done(); it.Next()) {
		Enable_Alternate_Materials(it.Peek_Obj()->Peek_Model(),onoff);
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//	Enable_Alternate_Materials
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Enable_Alternate_Materials (RenderObjClass * model,bool onoff)
{
	if (model == NULL) return;

	for (int i=0; i<model->Get_Num_Sub_Objects(); i++) {
		RenderObjClass * subobj = model->Get_Sub_Object(i);
		Enable_Alternate_Materials(subobj,onoff);
		REF_PTR_RELEASE(subobj);
	}

	if (model->Class_ID() == RenderObjClass::CLASSID_MESH) {
		MeshModelClass * mesh_model = ((MeshClass *)model)->Get_Model();
		mesh_model->Enable_Alternate_Material_Description(onoff);
		REF_PTR_RELEASE(mesh_model);
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//	CnC_Initialize
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::CnC_Initialize (BaseControllerClass *base)
{
	BaseController	= base;

	//
	//	Create an observer that we can use to monitor the building's health
	//
	BuildingMonitor = new BuildingMonitorClass;
	BuildingMonitor->Set_Building (this);
	Add_Observer (BuildingMonitor);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	On_Damaged
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::On_Damaged (void)
{
	//
	//	Notify the controller that the building has been damaged
	//
	if (BaseController != NULL) {
		BaseController->On_Building_Damaged (this);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	On_Destroyed
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::On_Destroyed (void)
{
	IsDestroyed = true;	

	//
	//	Notify the controller that the building has been destroyed
	//
	if (BaseController != NULL) {
		BaseController->On_Building_Destroyed (this);
	}
	
	//
	//	Mark the object as dirty so it's state will be mirrored on the client
	//
	if (CombatManager::I_Am_Server ()) {
		Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
	}

	//
	//	Reveal this building to the encyclopedia
	//
	EncyclopediaMgrClass::Reveal_Object (this);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Import_Rare
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Import_Rare (BitStreamClass &packet)
{
	DamageableGameObj::Import_Rare (packet);

	//
	//	Read the state information from the server
	//	
	bool is_destroyed	= IsDestroyed;
	bool is_power_on	= IsPowerOn;
	int current_state	= CurrentState;
	packet.Get (is_destroyed);
	packet.Get (is_power_on);
	packet.Get (current_state, BITPACK_BUILDING_STATE);

	//
	//	Is the power state of the building changing?
	//
	if (is_power_on != IsPowerOn) {
		Enable_Power (is_power_on);
	} 

	// 
	// Update our health if we are destroyed
	// 
	if (is_destroyed) {
		Get_Defense_Object()->Set_Health(0.0f);
	}

	//
	//	Recalculate our state (if necessary)
	//
	if (current_state != CurrentState) {
		Update_State ();
	}

	//
	//	Become destroyed (if necessary)
	//
	if (is_destroyed && IsDestroyed == false) {
		On_Destroyed ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Export_Rare
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Export_Rare (BitStreamClass &packet)
{
	DamageableGameObj::Export_Rare (packet);

	//
	//	Transmit the state information
	//
	packet.Add (IsDestroyed);
	packet.Add (IsPowerOn);
	packet.Add (CurrentState, BITPACK_BUILDING_STATE);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Set_Precision
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Set_Precision (void)
{
	cEncoderList::Set_Precision (BITPACK_BUILDING_STATE, -1, (int)BuildingStateClass::STATE_COUNT);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Collect_Building_Components
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Collect_Building_Components (void)
{
	Reset_Components ();

	//
	// Iterate through all static objects in the level, assigning the building components to
	// the appropriate building controller
	//
	RefPhysListIterator staticobj_iterator = PhysicsSceneClass::Get_Instance()->Get_Static_Object_Iterator();
	for (staticobj_iterator.First(); !staticobj_iterator.Is_Done(); staticobj_iterator.Next()) {
		
		StaticPhysClass * obj = staticobj_iterator.Peek_Obj()->As_StaticPhysClass ();
		WWASSERT (obj != NULL);
		WWASSERT (obj->Peek_Model() != NULL);

		const char *obj_name = obj->Peek_Model()->Get_Name();

		if (obj->Get_Factory().Chunk_ID() == PHYSICS_CHUNKID_BUILDINGAGGREGATE) {

			//
			//	Does this aggregate match the prefix that this building is expecting?
			//
			if (Name_Prefix_Matches_This_Building (obj_name)) {

				//
				//	Calculate how far the object is from the center of this building's
				// collection sphere
				//
				Vector3 obj_pos = obj->Peek_Model()->Get_Bounding_Box().Center;
				float dist2 = (CollectionSphere.Center - obj_pos).Length2 ();

				//
				//	Is this object within the collection sphere?
				//
				if (dist2 <= (CollectionSphere.Radius * CollectionSphere.Radius)) {

					// Check for an existing building owner
					CombatPhysObserverClass * old_observer = (CombatPhysObserverClass *)obj->Get_Observer();
					BuildingGameObj * existing_building = NULL;
					if ( old_observer != NULL ) {
						existing_building = old_observer->As_BuildingGameObj();
					}
					if ( existing_building != NULL ) {
						// Another building already has this
						float existing_dist2 = (existing_building->CollectionSphere.Center - obj_pos).Length2 ();

						if ( dist2 < existing_dist2 ) {
							// But we are closer, so remove from the existing
							existing_building->Remove_Aggregate ((BuildingAggregateClass *)obj);
							// and Take him
							Add_Aggregate ((BuildingAggregateClass *)obj);
						}

					} else {
						// We are the first/only
						Add_Aggregate ((BuildingAggregateClass *)obj);
					}

				}				
			}

		} else if (Is_Interior_Mesh_Name (obj_name) || Is_Exterior_Mesh_Name (obj_name)) {
			
			//
			//	Does this mesh match the prefix that this building is expecting?
			//
			if (Name_Prefix_Matches_This_Building (obj_name)) {

				//
				//	Calculate how far the object is from the center of this building's
				// collection sphere
				//
				Vector3 obj_pos = obj->Peek_Model()->Get_Bounding_Box().Center;
				float dist2 = (CollectionSphere.Center - obj_pos).Length2 ();

				//
				//	Is this object within the collection sphere?
				//
				if (dist2 <= (CollectionSphere.Radius * CollectionSphere.Radius)) {

					// Check for an existing building owner
					CombatPhysObserverClass * old_observer = (CombatPhysObserverClass *)obj->Get_Observer();
					BuildingGameObj * existing_building = NULL;
					if ( old_observer != NULL ) {
						existing_building = old_observer->As_BuildingGameObj();
					}
					if ( existing_building != NULL ) {
						// Another building already has this
						float existing_dist2 = (existing_building->CollectionSphere.Center - obj_pos).Length2 ();

						if ( dist2 < existing_dist2 ) {
							// But we are closer, so remove from the existing
							existing_building->Remove_Mesh (obj);
							// and Take him
							Add_Mesh (obj);
						}

					} else {
						// We are the first/only
						Add_Mesh (obj);
					}

				}
			}
		}
	}

	//
	// Iterate through all static lights in the level, assigning each building light to 
	// the appropriate building controller
	//
	RefPhysListIterator staticlight_iterator = PhysicsSceneClass::Get_Instance ()->Get_Static_Light_Iterator ();
	for (staticlight_iterator.First (); !staticlight_iterator.Is_Done (); staticlight_iterator.Next ()) {
		
		LightPhysClass * light = staticlight_iterator.Peek_Obj()->As_LightPhysClass();
		WWASSERT(light != NULL);
		WWASSERT(light->Peek_Model() != NULL);

		//
		//	Does this light match the prefix that this building is expecting?
		//
		if (Name_Prefix_Matches_This_Building (light->Get_Name ())) {

			//
			//	Calculate how far the light is from the center of this building's
			// collection sphere
			//
			Vector3 light_pos;
			light->Get_Position (&light_pos);			
			float dist2 = (CollectionSphere.Center - light_pos).Length2 ();

			//
			//	Is this light within the collection sphere?
			//
			if (dist2 <= (CollectionSphere.Radius * CollectionSphere.Radius)) {
				Add_Light (light);
			}
		}
	}

	//
	//	Update the building's state
	//
	Initialize_Building ();
	Update_State (true);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Export_Creation
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Export_Creation (BitStreamClass &packet)
{
	DamageableGameObj::Export_Creation (packet);

	//
	//	Send the object's position
	//
	Vector3 position (0, 0, 0);
	Get_Position (&position);
	packet.Add (position.X, BITPACK_WORLD_POSITION_X);
	packet.Add (position.Y, BITPACK_WORLD_POSITION_Y);
	packet.Add (position.Z, BITPACK_WORLD_POSITION_Z);

	//
	//	Send the collection sphere's definition
	//
	packet.Add (CollectionSphere.Center.X,		BITPACK_WORLD_POSITION_X);
	packet.Add (CollectionSphere.Center.Y,		BITPACK_WORLD_POSITION_Y);
	packet.Add (CollectionSphere.Center.Z,		BITPACK_WORLD_POSITION_Z);
	packet.Add (CollectionSphere.Radius,		BITPACK_BUILDING_RADIUS);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Import_Creation
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Import_Creation (BitStreamClass &packet)
{
	DamageableGameObj::Import_Creation (packet);

	//
	//	Read the object's position
	//
	Vector3 position (0, 0, 0);
	packet.Get (position.X, BITPACK_WORLD_POSITION_X);
	packet.Get (position.Y, BITPACK_WORLD_POSITION_Y);
	packet.Get (position.Z, BITPACK_WORLD_POSITION_Z);
	Set_Position (position);

	//
	//	Read the collection sphere's definition
	//
	packet.Get (CollectionSphere.Center.X,		BITPACK_WORLD_POSITION_X);
	packet.Get (CollectionSphere.Center.Y,		BITPACK_WORLD_POSITION_Y);
	packet.Get (CollectionSphere.Center.Z,		BITPACK_WORLD_POSITION_Z);
	packet.Get (CollectionSphere.Radius,		BITPACK_BUILDING_RADIUS);

	//
	//	Now initialize the object
	//
	Collect_Building_Components ();
	Update_State ();
	Initialize_Building ();
	return ;
}


/*
////////////////////////////////////////////////////////////////////////////
//
//	Compute_Object_Priority
//
/////////////////////////////////////////////////////////////////////////////
float
BuildingGameObj::Compute_Object_Priority (int client_id, const Vector3 &client_pos)
{
	float priority = 1.0F;

	if (Get_Object_Dirty_Bit (client_id, BIT_RARE) == false) {
		priority = DamageableGameObj::Compute_Object_Priority (client_id, client_pos);
	}

	// 
	// Priority value is cached, update the cached value.
	//
	Set_Cached_Priority(priority);

	return priority;
}
*/


////////////////////////////////////////////////////////////////////////////
//
//	Get_Description
//
/////////////////////////////////////////////////////////////////////////////
void 
//BuildingGameObj::Get_Extended_Information(StringClass & description)
BuildingGameObj::Get_Description(StringClass & description)
{
	//
	// Construct a diagnostic string
	//

	StringClass line(0,true);

   line.Format("ID:    %d\n", Get_ID());
	description += line;

   line.Format("Name:  %s\n", Get_Definition().Get_Name());
	description += line;

   line.Format("Team:  %d\n", Get_Player_Type());
	description += line;

	Vector3 position;
	Get_Position(&position);
	line.Format("POS:   %-5.2f, %-5.2f, %-5.2f\n", position.X, position.Y, position.Z);
	description += line;
	
	if (Get_Defense_Object() != NULL) {
		line.Format("HLTH:  %-5.2f\n", Get_Defense_Object()->Get_Health());
		description += line;
	}

	line.Format("HIB:   %d\n", Is_Hibernating());
	description += line;

	line.Format("Destr: %d\n", IsDestroyed);
	description += line;

	line.Format("Power: %d\n", IsPowerOn);
	description += line;

	line.Format("ISC:   %d\n", Get_Import_State_Count());
	description += line;
}


////////////////////////////////////////////////////////////////////////////
//
//	Find_Closest_Poly_For_Model
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Find_Closest_Poly_For_Model
(
	RenderObjClass *	model,
	const Vector3 &	pos,
	float *				distance2
)
{
	if (model == NULL) {
		return ;
	}

	//
	//	Convert the point from world space to object space
	//
	Vector3 obj_space_point;
	Matrix3D::Inverse_Transform_Vector (model->Get_Transform (), pos, &obj_space_point);
	int index;
	for (index = 0; index < model->Get_Num_Sub_Objects (); index++) {
		RenderObjClass *sub_obj = model->Get_Sub_Object (index);
		
		//
		//	Recurse into this sub-object
		//
		if (sub_obj != NULL) {
			Find_Closest_Poly_For_Model (sub_obj, pos, distance2);
		}

		REF_PTR_RELEASE (sub_obj);
	}

	//
	//	Is this object a mesh?  If so, then do a distance check against all its polygons
	//
	if (model->Class_ID () == RenderObjClass::CLASSID_MESH) {
		MeshModelClass *mesh_model = ((MeshClass *)model)->Peek_Model ();
		if (mesh_model != NULL) {

			const TriIndex *tri_array	= mesh_model->Get_Polygon_Array ();
			Vector3 *vert_array			= mesh_model->Get_Vertex_Array ();
			
			//
			//	Check each polygon to see which is the closest
			//
			int poly_count = mesh_model->Get_Polygon_Count ();
			for (index = 0; index < poly_count; index ++) {
				
				int vert1 = tri_array[index][0];
				int vert2 = tri_array[index][1];
				int vert3 = tri_array[index][2];

				//
				//	Compute the distance from the center of this polygon to the
				// 
				//
				Vector3 poly_center	= (vert_array[vert1] + vert_array[vert2] + vert_array[vert3]) * 0.33F;
				Vector3 delta			= (poly_center - obj_space_point);
				float dist2				= delta.Length2 ();
				if (dist2 < (*distance2)) {
					(*distance2) = dist2;
				}
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Find_Closest_Poly
//
/////////////////////////////////////////////////////////////////////////////
void
BuildingGameObj::Find_Closest_Poly (const Vector3 &pos, float *distance2)
{
	(*distance2) = 9999.0F;

	RefMultiListIterator<StaticPhysClass> int_iterator (&InteriorMeshes);
	RefMultiListIterator<StaticPhysClass> ext_iterator (&ExteriorMeshes);

	//
	//	Check all the interior meshes first
	//
	for (int_iterator.First (); !int_iterator.Is_Done (); int_iterator.Next ()) {
		Find_Closest_Poly_For_Model (int_iterator.Peek_Obj ()->Peek_Model (), pos, distance2);
	}

	//
	//	Now, check all the exterior meshes
	//
	for (ext_iterator.First (); !ext_iterator.Is_Done (); ext_iterator.Next ()) {
		Find_Closest_Poly_For_Model (ext_iterator.Peek_Obj ()->Peek_Model (), pos, distance2);
	}

	return ;
}
