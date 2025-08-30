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
 *                     $Archive:: /Commando/Code/Combat/staticnetworkobject.h                 $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 10/16/01 3:56p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 11                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	__STATICNETWORKOBJECT_H
#define	__STATICNETWORKOBJECT_H


#include "networkobject.h"
#include "vector.h"


////////////////////////////////////////////////////////////////
//	Forward delcarations
////////////////////////////////////////////////////////////////
class StaticAnimPhysClass;
class DoorNetworkObjectClass;
class ElevatorNetworkObjectClass;
class DSAPONetworkObjectClass;


////////////////////////////////////////////////////////////////
//
//	StaticNetworkObjectClass
//
////////////////////////////////////////////////////////////////
class StaticNetworkObjectClass : public NetworkObjectClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	StaticNetworkObjectClass (void);
	~StaticNetworkObjectClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	virtual void		Initialize (StaticAnimPhysClass *phys_obj);
	virtual void		Delete (void) override	{}

	//
	//	Timestep support
	//
	virtual void		Network_Think (void) override;

	//
	//	Server-to-client state importing/exporting
	//
	virtual void		Import_Rare (BitStreamClass &packet) override;
   virtual void		Export_Rare (BitStreamClass &packet) override;

	//
	//	Filtering support
	//
	virtual bool		Get_World_Position (Vector3 &pos) const override;
	virtual int			Get_Vis_ID (void) override;
	//virtual float		Compute_Object_Priority (int client_id, const Vector3 &client_pos);

	//
	//	Static methods
	//
	static void			Generate_Static_Network_Objects (void);
	static void			Free_Static_Network_Objects (void);

	// 
	// Type identification
	//
	virtual DoorNetworkObjectClass *			As_Door_Network_Object_Class( void ) 	{ return NULL; }
	virtual ElevatorNetworkObjectClass *	As_Elevator_Network_Object_Class( void ) { return NULL; }
	virtual DSAPONetworkObjectClass *		As_DSAPO_Network_Object_Class( void ) 		{ return NULL; }

	//
	//	Diagnostics 
	//
	//virtual bool		Is_Tagged(void)										{ return true; }
	static int			Get_Static_Network_Object_Count (void)			{ return StaticNetworkObjectList.Count (); }
	static const StaticNetworkObjectClass *	Get_Static_Network_Object (int index)			{ return StaticNetworkObjectList[index]; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	StaticAnimPhysClass *	PhysObj;

	static DynamicVectorClass<StaticNetworkObjectClass *> StaticNetworkObjectList;

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	int			AnimationMode;
	float			LoopStart;
	float			LoopEnd;
	float			CurrFrame;
	float			TargetFrame;
};


////////////////////////////////////////////////////////////////
//
//	DoorNetworkObjectClass
//
////////////////////////////////////////////////////////////////
class DoorNetworkObjectClass : public StaticNetworkObjectClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructor
	////////////////////////////////////////////////////////////////
	/*
	DoorNetworkObjectClass (void)	:
		DoorState (0)	{}
	*/
	DoorNetworkObjectClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Intialization
	//
	virtual void		Initialize (StaticAnimPhysClass *phys_obj) override;

	//
	//	Timestep support
	//
	virtual void		Network_Think (void) override;

	//
	//	Server-to-client state importing/exporting
	//
	virtual void		Import_Rare (BitStreamClass &packet) override;
   virtual void		Export_Rare (BitStreamClass &packet) override;

	// 
	// Type identification
	//
	virtual  DoorNetworkObjectClass *			As_Door_Network_Object_Class( void ) override		{ return this; }

	//
	// Diagnostics
	// 
	//virtual bool		Is_Tagged(void)										{ return false; }
	virtual void		Get_Description(StringClass & description) override;

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	int	DoorState;
};


////////////////////////////////////////////////////////////////
//
//	ElevatorNetworkObjectClass
//
////////////////////////////////////////////////////////////////
class ElevatorNetworkObjectClass : public StaticNetworkObjectClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructor
	////////////////////////////////////////////////////////////////
	/*
	ElevatorNetworkObjectClass (void)	:
		State (0),
		DoorStateTop (0),
		DoorStateBottom (0)	{}
	*/
	ElevatorNetworkObjectClass (void);

	//
	//	Intialization
	//
	virtual void		Initialize (StaticAnimPhysClass *phys_obj) override;

	//
	//	Timestep support
	//
	virtual void		Network_Think (void) override;

	//
	//	Server-to-client state importing/exporting
	//
	virtual void		Import_Rare (BitStreamClass &packet) override;
   virtual void		Export_Rare (BitStreamClass &packet) override;

	// 
	// Type identification
	//
	virtual  ElevatorNetworkObjectClass *	As_Elevator_Network_Object_Class( void ) override	{ return this; }

	//
	// Diagnostics
	// 
	//virtual bool		Is_Tagged(void)										{ return true; }
	virtual void		Get_Description(StringClass & description) override;

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	int	State;
	int	DoorStateTop;
	int	DoorStateBottom;
};


////////////////////////////////////////////////////////////////
//
//	DSAPONetworkObjectClass
//
////////////////////////////////////////////////////////////////
class DSAPONetworkObjectClass : public StaticNetworkObjectClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructor
	////////////////////////////////////////////////////////////////
	/*
	DSAPONetworkObjectClass (void)	:
		Health (0) {
	*/
	DSAPONetworkObjectClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	virtual void		Initialize (StaticAnimPhysClass *phys_obj) override;

	//
	//	Timestep support
	//
	virtual void		Network_Think (void) override;

	//
	//	Server-to-client state importing/exporting
	//
	virtual void		Import_Rare (BitStreamClass &packet) override;
   virtual void		Export_Rare (BitStreamClass &packet) override;

	// 
	// Type identification
	//
	virtual				DSAPONetworkObjectClass *		As_DSAPO_Network_Object_Class( void ) override		{ return this; }

private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	float		Health;
};


#endif	// __STATICNETWORKOBJECT_H
