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
 *                     $Archive:: /Commando/Code/Combat/building.h                            $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 1/07/02 3:52p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 42                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	BUILDING_H
#define	BUILDING_H

#include "always.h"
#include "damageablegameobj.h"
#include "combatphysobserver.h"
#include "buildingaggregate.h"
#include "lightphys.h"
#include "playertype.h"

/*
** Forward declarations
*/
class AudibleSoundClass;
class BaseControllerClass;
class BuildingMonitorClass;
class PowerPlantGameObj;
class SoldierFactoryGameObj;
class VehicleFactoryGameObj;
class AirStripGameObj;
class WarFactoryGameObj;
class RefineryGameObj;
class ComCenterGameObj;
class RepairBayGameObj;


/**
** BuildingConstants
** Convienent namespace declaration for the constants used with buildings
*/
namespace BuildingConstants
{
	typedef enum
	{
		TYPE_NONE					= -1,
		TYPE_POWER_PLANT,
		TYPE_SOLDIER_FACTORY,
		TYPE_VEHICLE_FACTORY,
		TYPE_REFINERY,
		TYPE_COM_CENTER,
		TYPE_REPAIR_BAY,		
		TYPE_SHRINE,
		TYPE_HELIPAD,
		TYPE_CONYARD,
		TYPE_BASE_DEFENSE,
		TYPE_COUNT
	} BuildingType;

	typedef enum
	{
		LEGACY_TEAM_GDI						= 0,
		LEGACY_TEAM_NOD,
	} LegacyBuildingTeam;

	enum
	{
		BASE_COUNT = 2,
	};
}

/**
** Building Game Obj Def
** This class is an editable definition for a building.  It contains all constant data needed to initialize
** a BuildingGameObj. 
*/
class BuildingGameObjDef : public DamageableGameObjDef
{
public:

	BuildingGameObjDef( void );

	virtual uint32								Get_Class_ID( void ) const;
	virtual PersistClass *						Create( void ) const ;
	virtual bool								Save( ChunkSaveClass &csave );
	virtual bool								Load( ChunkLoadClass &cload );
	virtual const PersistFactoryClass &	Get_Factory( void ) const;

	DECLARE_EDITABLE( BuildingGameObjDef, DamageableGameObjDef );
	
	void										Set_Type (BuildingConstants::BuildingType type) { Type = type; }
	BuildingConstants::BuildingType				Get_Type (void) const	{ return Type; }

	const StringClass &							Get_Mesh_Prefix (void) const	{ return MeshPrefix; }

	int Get_Damage_Report(int team) const;
	int Get_Destroy_Report(int team) const;

protected:

	StringClass									MeshPrefix;
	ArmorType									MCTSkin;
	BuildingConstants::BuildingType				Type;

	int GDIDamageReportID;
	int NodDamageReportID;
	int GDIDestroyReportID;
	int NodDestroyReportID;

	friend	class								BuildingGameObj;
};

/*
** Building Game Obj
** This class encapsulates the basic building functionality.  
**
** At initialization, the following things need to happen.
** - Collects all of the meshes that are part of the building using proximity, prefix matching
**   and the "house" naming convention (# indicates an interior mesh, ^ indicates an exterior mesh).
** - Collect all of the BuildingAggregates that are part of the building using proximity and
**   prefix matching.
** - Collect all light sources associated with the building using proximity and prefix matching.  
** - Install itself as an observer into each StaticPhysClass (meshes and aggregates) that is part 
**   of the building.
**
** During gameplay, the building code does the following things:
** - Responds to all bullet collisions and explosions on any of its meshes/aggregates by
**   applying damage to the building.
** - When destroyed, applies the alternate materials on its exterior meshes
** - When the power goes out, applies the alternate materials on its interior meshes
** - When the state changes (power on/off, health 100,75,50,25,0) switches the behaviors of
**   all aggregates.
**
*/
class BuildingGameObj : public DamageableGameObj, public CombatPhysObserverClass
{
public:
	//	Constructor and Destructor
	BuildingGameObj( void );
	virtual	~BuildingGameObj( void );

	// Definitions
	virtual	void	Init( void );
	void	Init( const BuildingGameObjDef & definition );
	const BuildingGameObjDef & Get_Definition( void ) const ;

	// Save / Load
	virtual	bool	Save( ChunkSaveClass & csave );
	virtual	bool	Load( ChunkLoadClass & cload );
	virtual	const	PersistFactoryClass & Get_Factory( void ) const;

	// Type identification
	virtual	BuildingGameObj	*		As_BuildingGameObj( void )			{ return this; }
	virtual	DamageableGameObj	*		As_DamageableGameObj( void )		{ return this; }
	virtual	PowerPlantGameObj *		As_PowerPlantGameObj (void)		{ return NULL; }
	virtual	SoldierFactoryGameObj *	As_SoldierFactoryGameObj (void)	{ return NULL; }
	virtual	VehicleFactoryGameObj *	As_VehicleFactoryGameObj (void)	{ return NULL; }
	virtual	AirStripGameObj *			As_AirStripGameObj (void)			{ return NULL; }
	virtual	WarFactoryGameObj *		As_WarFactoryGameObj (void)		{ return NULL; }
	virtual	RefineryGameObj *			As_RefineryGameObj (void)			{ return NULL; }
	virtual	ComCenterGameObj *		As_ComCenterGameObj (void)			{ return NULL; }
	virtual	RepairBayGameObj *		As_RepairBayGameObj (void)			{ return NULL; }

	/****/

	/*
	** Position interface, the position of the building controller influences which meshes
	** and other components are automatically assigned to this building.  When there are two
	** BuildingGameObj's that can accept a particular component, the closest one wins.
	*/
	void											Get_Position(Vector3 * pos) const		{ *pos = Position; }
	void											Set_Position(const Vector3 & pos)		{ Position = pos; CollectionSphere.Center = pos; }

	/*
	** Damage interface, designed to work similarly to a game object
	*/
	virtual	void								Apply_Damage( const OffenseObjectClass & damager, 
																	  float scale = 1.0f, 
																	  int alternate_skin = -1 );
	void											Apply_Damage_Building(	const OffenseObjectClass & offense,
																				StaticPhysClass * component ); 
	void											Apply_Damage_Building(	const OffenseObjectClass & offense,
																				bool mct_damage ); 
	void											Set_Normalized_Health (float health);

	/*
	** Power interface, turn the power on and off
	*/
	void											Enable_Power(bool onoff);
	bool											Is_Power_Enabled(void) const			{ return IsPowerOn; }

	/*
	** Building announcment support
	*/
	void											Play_Announcement( int text_id, bool broadcast );
	void											Stop_Current_Announcement( void );

	/*
	** Utility functions for the building mesh naming convention.
	** Exterior meshes contain a ^ as the first character following their prefix.
	** Interior meshes contain a # as the first character following their prefix.
	*/
	bool											Name_Prefix_Matches_This_Building(const char * name);
	static bool									Is_Interior_Mesh_Name(const char * name);
	static bool									Is_Exterior_Mesh_Name(const char * name);
	const char *								Get_Name_Prefix (void) const { return Get_Definition().MeshPrefix; }

	/*
	** Building component support
	*/
	void											Collect_Building_Components (void);
	void											Get_Collection_Sphere (SphereClass *sphere) const	{ *sphere = CollectionSphere; }
	void											Set_Collection_Sphere (const SphereClass &sphere)	{ CollectionSphere = sphere; }


	/*
	** CnC mode suport
	*/
	virtual void								CnC_Initialize (BaseControllerClass *base);
	virtual void								On_Destroyed (void);
	virtual void								On_Damaged (void);
	bool											Is_Destroyed (void) const { return IsDestroyed; }

	/*
	** Network support
	*/
   virtual	void								Import_Rare (BitStreamClass &packet);
   virtual	void								Export_Rare (BitStreamClass &packet);
	virtual	void								Export_Creation (BitStreamClass &packet);
	virtual	void								Import_Creation (BitStreamClass &packet);
	static void									Set_Precision (void);
	virtual bool								Get_World_Position (Vector3 &pos) const	{ pos = Position; return true; }
	//virtual float								Compute_Object_Priority (int client_id, const Vector3 &client_pos);
	//virtual	void								Get_Extended_Information( StringClass & description );
	virtual	void								Get_Description( StringClass & description );
	virtual bool								Is_Tagged(void)									{ return false; }


	/*
	**
	*/
	bool	Is_GDI( void )						{ return Get_Player_Type() == PLAYERTYPE_GDI; }
	bool	Is_Nod( void )						{ return Get_Player_Type() == PLAYERTYPE_NOD; }
																			
	static void	Set_Can_Repair_Buildings(bool flag)		{CanRepairBuildings = flag;}
	static bool	Get_Can_Repair_Buildings(void)			{return CanRepairBuildings;}

	//
	//	Informational
	//
	void											Find_Closest_Poly (const Vector3 &pos, float *distance2);	

protected:

	/*
	**	CnC mode stuff
	*/
	BuildingMonitorClass *					BuildingMonitor;
	BaseControllerClass *					BaseController;
	bool											IsDestroyed;

//private:

	Vector3										Position;
	bool											IsPowerOn;
	int											CurrentState;						// derived from the health and IsPowerOn

	AudibleSoundClass *						CurrentAnnouncement;
	SphereClass									AnnouncementSphere;
	SphereClass									CollectionSphere;

	RefMultiListClass<StaticPhysClass>				InteriorMeshes;
	RefMultiListClass<StaticPhysClass>				ExteriorMeshes;
	RefMultiListClass<BuildingAggregateClass>		Aggregates;
	RefMultiListClass<LightPhysClass>				PowerOnLights;
	RefMultiListClass<LightPhysClass>				PowerOffLights;

	static bool									CanRepairBuildings;

	void											Initialize_Building(void);	

	void											Reset_Components(void);
	void											Add_Mesh(StaticPhysClass * terrain);
	void											Remove_Mesh(StaticPhysClass * terrain);
	void											Add_Aggregate(BuildingAggregateClass * aggregate);
	void											Remove_Aggregate(BuildingAggregateClass * aggregate);
	void											Add_Light(LightPhysClass * light);
	void											Find_Closest_Poly_For_Model (RenderObjClass *model, const Vector3 &pos, float *distance2);

	void											Update_State(bool force_update = false);
	void											Enable_Alternate_Materials(RefMultiListClass<StaticPhysClass> & models, bool onoff);
	void											Enable_Alternate_Materials(RenderObjClass * model,bool onoff);

	friend class GameObjManager;
};

#endif // BUILDING_H

