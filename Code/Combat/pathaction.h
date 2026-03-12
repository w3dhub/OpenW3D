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
 *                     $Archive:: /Commando/Code/Combat/pathaction.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/16/01 3:26p                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PATHACTION_H
#define __PATHACTION_H

#include "elevator.h"
#include "vector3.h"


////////////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////////////////////////////////
class SmartGameObj;
class PathClass;
class ChunkSaveClass;
class ChunkLoadClass;


////////////////////////////////////////////////////////////////////////////////////////////
//
//	PathActionClass
//
////////////////////////////////////////////////////////////////////////////////////////////
class PathActionClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		STATE_FINISHED						= 0,
		STATE_WAITING,
		STATE_MOVING

	}	STATE;

	typedef enum
	{
		TYPE_UNKNOWN						= 0,
		TYPE_JUMPING,
		TYPE_ELEVATOR,
		TYPE_DOOR,
		TYPE_LADDER

	}	TYPE;


	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	PathActionClass (void);
	~PathActionClass (void);


	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Configuration
	//
	void				Initialize (TYPE type, PathClass *path, SmartGameObj *game_obj, StaticPhysClass *mechanism = NULL);
	void				Set_Ladder_Index (int index)	{ LadderIndex = index; }
	void				Reset (void);

	//
	//	Run-time interface
	//
	bool				Process (void);
	STATE				Get_State (void) const			{ return State; }
	const Vector3 &Get_Destination (void) const	{ return Destination; }

	//
	//	Save/Load support
	//
	void				Save (ChunkSaveClass &csave);
	void				Load (ChunkLoadClass &cload);

	//
	//	Ladder occupant access
	//
	static ScriptableGameObj	*	Get_Ladder_Occupant (int ladder_index);
	static void							Set_Ladder_Occupant (int ladder_index, ScriptableGameObj	*object);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void				Handle_Jump (void);
	void				Handle_Ladder (void);
	void				Handle_Door (void);
	void				Handle_Elevator (void);

	bool				Has_Arrived (void);
	void				Get_Elevator_Zone_Pos (ELEVATOR_ZONE zone_id, Vector3 *position);

	void				Load_Variables (ChunkLoadClass &cload);

	void				Set_Finished (void);

	////////////////////////////////////////////////////////////////
	//	Private constants
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		ELEVATOR_STATE_NONE				= 0,
		ELEVATOR_STATE_WAITING,
		ELEVATOR_STATE_RIDING,
		ELEVATOR_STATE_ENTERING,
		ELEVATOR_STATE_EXITING,

	}	ELEVATOR_STATE;

	typedef enum
	{
		DOOR_STATE_NONE						= 0,
		DOOR_STATE_GETTING_IN_POSITION,
		DOOR_STATE_WAITING,
		DOOR_STATE_ENTERING

	}	DOOR_STATE;

	typedef enum
	{
		LADDER_STATE_NONE						= 0,
		LADDER_STATE_WAITING,
		LADDER_STATE_GETTING_ON,
		LADDER_STATE_CLIMBING

	}	LADDER_STATE;

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	ELEVATOR_STATE			ElevatorState;
	DOOR_STATE				DoorState;
	LADDER_STATE			LadderState;
	STATE						State;
	TYPE						Type;
	PathClass *				Path;
	StaticPhysClass *		Mechanism;
	SmartGameObj *			GameObj;
	Vector3					Destination;
	Vector3					FacePos;
	float						Timer;
	int						LadderIndex;

	static DynamicVectorClass<GameObjReference>	LadderList;
};

#endif //__PATHACTION_H
