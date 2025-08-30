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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/basecontroller.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 7:18p                                                $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __BASE_CONTROLLER_H
#define __BASE_CONTROLLER_H

#include "building.h"
#include "obbox.h"
#include "vector.h"
#include "characterclasssettings.h"
#include "networkobject.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;
class ConversationClass;
class ActiveConversationClass;
class SoldierGameObj;
class BuildingGameObj;
class VehicleGameObj;
class BeaconGameObj;

////////////////////////////////////////////////////////////////
//
//	BaseControllerClass
//
////////////////////////////////////////////////////////////////
class BaseControllerClass : public NetworkObjectClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Friends
	////////////////////////////////////////////////////////////////
	friend class BuildingMonitorClass;

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	BaseControllerClass (void);
	virtual ~BaseControllerClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	void		Initialize ( int player_type );
	void		Shutdown (void);
	void		Delete (void) override {};

	//
	// Networking.
	//
	//	Set_Delete_Pending -
	//
	// The base controllers persist between levels so they should never be marked for deletion. They will be deleted
	// when needed by their owner, the game mode
	//
	virtual void Set_Delete_Pending (void) override {};

	//
	//	Building access
	//
	void		Add_Building (BuildingGameObj *building);

	//
	//	Timestep
	//
	void		Think (void);

	//
	//	Power related methods
	//
	void		Set_Base_Powered (bool onoff);
	bool		Is_Base_Powered (void) const		{ return BasePowered; }
	void		Check_Base_Power (void);
	void		Power_Base (bool onoff);
	void		Set_Operation_Time_Factor (float factor);
	float		Get_Operation_Time_Factor (void)	{ return OperationTimeFactor; }

	//
	//	Harvester support
	//
	void					Request_Harvester (int def_id);
	VehicleGameObj *	Get_Harvester_Vehicle (void);

	//
	//	State accessors
	//
	bool		Can_Generate_Soldiers (void) const		{ return CanGenerateSoldiers; }
	bool		Can_Generate_Vehicles (void) const		{ return CanGenerateVehicles; }

	void		Set_Can_Generate_Soldiers (bool onoff);
	void		Set_Can_Generate_Vehicles (bool onoff);

	//
	//	Data accessors
	//
	int			Get_Player_Type (void) const			{  return PlayerType; }

	//
	//	Base destruction
	//
	void		Set_Base_Destroyed ( bool onoff );
	bool		Is_Base_Destroyed (void) const			{ return IsBaseDestroyed; }
	void		Destroy_Base (void);
	void		Set_Beacon_Destroyed_Base ( bool onoff );
	bool		Did_Beacon_Destroy_Base ( void ) const	{ return DidBeaconDestroyBase; }

	//
	//	Beacon zone access
	//
	const OBBoxClass &	Get_Beacon_Zone (void)		{ return BeaconZone; }

	//
	//	Radar support
	//
	void		Check_Radar (void);
	bool		Is_Radar_Enabled (void) const	{ return IsRadarEnabled; }

	void Play_Announcement(int text_id);

	//
	//	Notifications
	//
	void		On_Building_Damaged (BuildingGameObj *building);
	void		On_Building_Destroyed (BuildingGameObj *building);

	void		On_Vehicle_Generated(VehicleGameObj *vehicle);
	void		On_Vehicle_Delivered(VehicleGameObj *vehicle);
	void		On_Vehicle_Damaged(VehicleGameObj* vehicle);
	void		On_Vehicle_Destroyed(VehicleGameObj* vehicle);

	void		On_Beacon_Armed(BeaconGameObj* beacon);
	void		On_Beacon_Disarmed(BeaconGameObj* beacon);
	void		On_Beacon_Warning(BeaconGameObj* beacon);

	//
	//	Fund support
	//
	//void		Deposit_Funds (int funds);
	void		Distribute_Funds_To_Each_Teammate (int funds);

	//
	//	Network support
	//
	void		Import_Occasional (BitStreamClass &packet) override;
   void		Export_Occasional (BitStreamClass &packet) override;

	//
	//	Component lookup
	//
	static BaseControllerClass *	Find_Base ( int playertype );
	static BaseControllerClass *	Find_Base_For_Star (void);
	BuildingGameObj *				Find_Building (BuildingConstants::BuildingType type);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected constants
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		BASE_UNDER_ATTACK		= 0,
		BUILDING_DESTROYED
	}	Notification;


	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void		Initialize_Building_List (void);
	void		Reset_Building_List (void);
	void		Enable_Radar (bool onoff);

	//
	//	Team notification
	//
	void		Notify_Team (Notification event, BuildingConstants::BuildingType type);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	bool							Are_All_Buildings_Destroyed (void);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	float										OperationTimeFactor;
	int											PlayerType;				// Team
	DynamicVectorClass<BuildingGameObj *>		BuildingList;
	bool										BasePowered;
	bool										CanGenerateSoldiers;
	bool										CanGenerateVehicles;
	bool										IsRadarEnabled;
	bool										IsBaseDestroyed;
	bool										DidBeaconDestroyBase;
	OBBoxClass									BeaconZone;

	int AnnounceInterval;
	float AnnouncedAlliedBldgDamageTime;
	float AnnouncedEnemyBldgDamageTime;
	float AnnouncedAlliedVehicleDamageTime;
	float AnnouncedEnemyVehicleDamageTime;

	static BaseControllerClass	*		CurrentBases[BuildingConstants::BASE_COUNT];
};


#endif //__BASE_CONTROLLER_H
