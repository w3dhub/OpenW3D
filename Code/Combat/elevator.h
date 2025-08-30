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
 *                     $Archive:: /Commando/Code/Combat/elevator.h                            $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 9/13/01 11:42a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 16                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	ELEVATOR_H
#define	ELEVATOR_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	__ACCESSIBLE_PHYS_H
	#include "accessiblephys.h"
#endif

#include "gameobjref.h"

class AudibleSoundClass;
class SmartGameObj;

/*
** Call zone IDs
*/
typedef enum
{
	ZONE_LOWER_CALL		= 0,
	ZONE_LOWER_INSIDE,
	ZONE_UPPER_CALL,
	ZONE_UPPER_INSIDE,
	ZONE_MAX
} ELEVATOR_ZONE;


/*
** ElevatorPhysDefClass
*/
class ElevatorPhysDefClass : public AccessiblePhysDefClass 
{
public:
	ElevatorPhysDefClass(void);
	
	virtual uint32								Get_Class_ID (void) const override;
	virtual const char *						Get_Type_Name(void) override				{ return "ElevatorPhysDef"; }
	virtual bool								Is_Type(const char *) override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	const OBBoxClass &						Get_Zone (ELEVATOR_ZONE id) const	{ return CallZones[id]; }

	DECLARE_EDITABLE( ElevatorPhysDefClass, AccessiblePhysDefClass );

protected:

	OBBoxClass									CallZones[4];
	float											CloseDelay;

	float											DoorClosedTop_FrameNum;
	float											DoorOpeningBottom_FrameNum;
	float											ElevatorStartTop_FrameNum;
	float											ElevatorStoppedBottom_FrameNum;

	int											DoorOpenSoundDefID;
	int											DoorCloseSoundDefID;
	int											DoorUnlockSoundDefID;
	int											DoorAccessDeniedSoundDefID;
	int											ElevatorMovingSoundDefID;

	friend	class								ElevatorPhysClass;
};


/*
** ElevatorPhysClass
*/
class	ElevatorPhysClass : public AccessiblePhysClass 
{
public:
	//	Constructor and Destructor
	ElevatorPhysClass( void );
	virtual ~ElevatorPhysClass( void );

	// RTTI
	virtual ElevatorPhysClass *	As_ElevatorPhysClass(void) override	{ return this; }

	// Definitions
	void	Init( const ElevatorPhysDefClass & definition );
	const ElevatorPhysDefClass * Get_ElevatorPhysDef( void ) const { WWASSERT( Definition ); return (ElevatorPhysDefClass *)Definition; }

	// Save / Load
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual	const	PersistFactoryClass & Get_Factory( void ) const override;

	virtual	void	Save_State( ChunkSaveClass & csave ) override;
	virtual	void	Load_State( ChunkLoadClass & cload ) override;

	// Timestep
	virtual void	Timestep( float dt ) override;

	// State import/export
	static void		Set_Precision(void);

	// AI support
	bool				Can_Object_Enter (SmartGameObj *game_obj);
	bool				Can_Object_Exit (SmartGameObj *game_obj);
	void				Request_Elevator (SmartGameObj *game_obj);
	int				Get_Floor (void);
	bool				Is_Moving (void) const	{ return (State == STATE_MOVING_UP || State == STATE_MOVING_DOWN); }
	void						Set_Current_Rider (ScriptableGameObj *rider)	{ CurrentAIRider = rider; }
	ScriptableGameObj *	Get_Current_Rider (void)							{ return CurrentAIRider; }

	enum {
		STATE_DOWN						= 0,
		STATE_MOVING_UP,
		STATE_UP,
		STATE_MOVING_DOWN,
		STATE_MAX
	};

	enum {
		DOOR_STATE_NORMAL				= 0,
		DOOR_STATE_UNLOCKED,
		DOOR_STATE_ACCESS_DENIED,
		DOOR_STATE_MAX
	};

protected:

	void	Get_Door_Transform( bool is_top, Matrix3D &tm );
	void	Update_Sound_Effects( void );
	void	Play_Effect( int effect_id, bool is_top );

	void	Update_State(void);
	void	Set_State( int new_state );
	void	Set_Door_State( int new_state, int door_id );

	enum {
		TOP		= 0,
		BOTTOM	= 1
	};

	int						State;
	int						DoorStates[2];
	float						CheckTimer;
	AudibleSoundClass *	MovingSoundObj;
	float						PrevFrame;
	bool						IsCallTimerSet;
	float						CallTimer;
	int						TriggerRequest;
	GameObjReference		CurrentAIRider;

	bool		Triggered( int zone_id );

	friend class ElevatorNetworkObjectClass;
};

#endif	// ELEVATOR_H

