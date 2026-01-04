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
 *                     $Archive:: /Commando/Code/Combat/vehicle.h                             $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/25/02 2:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 120                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	VEHICLE_H
#define	VEHICLE_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef SMARTGAMEOBJ_H
	#include "smartgameobj.h"
#endif

#ifndef VECTOR_H
	#include "vector.h"
#endif

#ifndef TRANSITION_H
	#include "transition.h"
#endif

#ifndef	WWSTRING_H
	#include "wwstring.h"
#endif

class	Sound3DClass;
class	VehiclePhysClass;
class	SoldierGameObj;
class	PersistantSurfaceEmitterClass;
class PersistantSurfaceSoundClass;


/*
**
*/
typedef enum {
	VEHICLE_TYPE_CAR,
	VEHICLE_TYPE_TANK,
	VEHICLE_TYPE_BIKE,
	VEHICLE_TYPE_FLYING,
	VEHICLE_TYPE_TURRET,
} VehicleType;

enum
{
	ENGINE_SOUND_STATE_STARTING = 0,
	ENGINE_SOUND_STATE_RUNNING,
	ENGINE_SOUND_STATE_STOPPING,
	ENGINE_SOUND_STATE_OFF,

	MAX_ENGINE_SOUND_STATES
};


/*
** VehicleGameObjDef - Defintion class for a VehicleGameObj
*/
class VehicleGameObjDef : public SmartGameObjDef
{
public:
	VehicleGameObjDef( void );
	virtual ~VehicleGameObjDef( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( VehicleGameObjDef, SmartGameObjDef );

	TRANSITION_DATA_LIST *					Get_Transition_List( void ) { return &Transitions; }
	const TRANSITION_DATA_LIST &			Get_Transition_List( void ) const { return Transitions; }

	void											Free_Transition_List( void );

	int Get_Damage_Report(int team) const;
	int Get_Destroy_Report(int team) const;

protected:

	VehicleType									Type;
	StringClass									TypeName;
	StringClass									Fire0Anim;
	StringClass									Fire1Anim;
	StringClass									Profile;
	TRANSITION_DATA_LIST						Transitions;
	float											TurnRadius;
	bool											OccupantsVisible;
	bool											SightDownMuzzle;
	bool											Aim2D;

	float											EngineSoundMaxPitchFactor;
	int											EngineSound[MAX_ENGINE_SOUND_STATES];

	float											SquishVelocity;
	int											VehicleNameID;

	int											NumSeats;

	int GDIDamageReportID;
	int NodDamageReportID;
	int GDIDestroyReportID;
	int NodDestroyReportID;

	friend	class								VehicleGameObj;
};

/*
**
*/
class VehicleGameObj : public SmartGameObj {

public:
	VehicleGameObj();
	virtual	~VehicleGameObj();

	// Definitions
	virtual	void	Init( void ) override;
	void	Init( const VehicleGameObjDef & definition );
	const VehicleGameObjDef & Get_Definition( void ) const ;

	// Save / Load / Construction Factory
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual	void	On_Post_Load( void ) override;
	virtual	const	PersistFactoryClass & Get_Factory( void ) const override;

	virtual	void	Startup( void ) override;

	// Physics
	VehiclePhysClass * Peek_Vehicle_Phys( void ) const;

	// Think
	virtual	void	Think( void ) override;
	virtual	void	Post_Think( void ) override;

	// Control
	virtual	void	Apply_Control( void ) override;
   virtual	int	Get_Weapon_Control_Owner(void) override;

	// Weapon
	virtual bool		Set_Targeting( const Vector3 & pos, bool do_tilt = true ) override;
	virtual bool		Use_2D_Aiming( void ) 				{ return Get_Definition().Aim2D; }
	virtual Matrix3D	Get_Look_Transform(void) override;
	bool					Has_Turret( void)						{ return TurretBone != 0; }

	// Type ID
	virtual VehicleGameObj		*As_VehicleGameObj() override		{ return this; }
	virtual	bool	Is_Aircraft( void )						{ return Get_Definition().Type == VEHICLE_TYPE_FLYING; }
	virtual  bool  Is_Turret( void )							{ return Get_Definition().Type == VEHICLE_TYPE_TURRET; }

   virtual int Get_Player_Type(void) const override;


	// Occupants
	void	Add_Occupant( SoldierGameObj * occupant, int seat_id );
	void	Add_Occupant( SoldierGameObj * occupant );
	void	Remove_Occupant( SoldierGameObj * occupant );
	bool	Contains_Occupant( SoldierGameObj * occupant );
	int	Get_Occupant_Count(void);
	int	Find_Seat( SoldierGameObj * occupant );
   SoldierGameObj * Get_Driver(void);
   SoldierGameObj * Get_Gunner(void);
   SoldierGameObj * Get_Actual_Gunner(void);
	virtual bool Is_Entry_Permitted(SoldierGameObj * p_soldier);

	void	Passenger_Entering( void );	// used to begin animation
	void	Passenger_Exiting( void );	// used to begin animation

	void Set_Vehicle_Delivered(void);

	// State
	virtual	void	Export_Creation( BitStreamClass & packet ) override;
	virtual	void	Import_Creation( BitStreamClass & packet ) override;
   virtual	void	Export_Rare( BitStreamClass &packet ) override;
	virtual	void	Import_Rare( BitStreamClass &packet ) override;
	virtual	void	Import_Frequent( BitStreamClass & packet ) override;
   virtual	void	Export_Frequent( BitStreamClass & packet ) override;
   virtual	void	Import_State_Cs( BitStreamClass & packet ) override;
   virtual	void	Export_State_Cs( BitStreamClass & packet ) override;

	// Profile
	const char *Get_Profile( void )						{ return Get_Definition().Profile; }

	// Velocity
	void Get_Velocity(Vector3 & vel) override;
	void Set_Velocity(Vector3 & vel);

	//virtual	void	Get_Extended_Information( StringClass & description );
	virtual	void	Get_Description( StringClass & description ) override;

	// Pathfinding
	float Get_Turn_Radius (void) const	{ return Get_Definition ().TurnRadius; }

	// Engine control
	bool	Is_Engine_Enabled (void) const;
	void	Enable_Engine (bool onoff);

	// Wheel Effects
	void	Init_Wheel_Effects( void );
	void	Shutdown_Wheel_Effects( void );
	void	Update_Wheel_Effects( void );

	// Damage
	virtual	void	Apply_Damage( const OffenseObjectClass & damager, float scale = 1.0f, int alternate_skin = -1 ) override;
	void	Update_Damage_Meshes( void );
	float	Get_Squish_Velocity( void) { return Get_Definition().SquishVelocity; }

	// Script enabling of transitions
	void		Script_Enable_Transitions( bool enable )	{	TransitionsEnabled = enable; Create_And_Destroy_Transitions(); }

	static void Set_Precision(void);

	static bool	Toggle_Target_Steering( void );
	static void	Set_Target_Steering( bool onoff );
	static bool Is_Target_Steering( void );

	const unichar_t *		Get_Vehicle_Name( void );

	// Vehicles expire when they are flipped.
	virtual ExpirationReactionType	Object_Expired( PhysClass * observed_obj ) override;

	// Vehicle locking.  In network games, a vehicle can be locked for all but the
	// owner for a certain amount of time.
	// NOTE: for network play, locking a vehicle only works at creation time and only
	// if the lock is set up prior to the creation packet being sent.
	void						Lock_Vehicle( ScriptableGameObj *lockowner, float locktime ) { LockOwner = lockowner, LockTimer = locktime; }
	bool						Is_Locked(void) { return ((LockOwner.Get_Ptr() != NULL) && (LockTimer > 0.0f)); }
	ScriptableGameObj *	Get_Lock_Owner(void) { return LockOwner.Get_Ptr(); }

	static void	Set_Default_Driver_Is_Gunner(bool flag)		{DefaultDriverIsGunner = flag;}
	bool	Get_Driver_Is_Gunner(void)			{return DriverIsGunner;}
	void	Toggle_Driver_Is_Gunner(void)		{ DriverIsGunner = !DriverIsGunner; }

	static void	Set_Camera_Locked_To_Turret(bool flag)		{CameraLockedToTurret = flag;}
	static bool	Get_Camera_Locked_To_Turret(void)			{return CameraLockedToTurret;}

	// Stealth
	virtual float		Get_Stealth_Fade_Distance(void) const override;

	virtual float		Get_Filter_Distance(void) const override;

	// Ignore occupants for raycasts
	void					Ignore_Occupants( void );
	void					Unignore_Occupants( void );


protected:

	Sound3DClass			*Sound;

	int						EngineSoundState;
	AudibleSoundClass		*CachedEngineSound;

	int						TurretBone;
	int						BarrelBone;
	float						TurretTurn;
	float						BarrelTilt;
	float						BarrelOffset;		// For Vehicles with offset turrets

	bool						TransitionsEnabled;
	bool						HasEnterTransitions;
	bool						HasExitTransitions;

	bool						VehicleDelivered;

	bool						DriverIsGunner;

	static bool				DefaultDriverIsGunner;
	static bool				CameraLockedToTurret;

	// Wheel Effects
	VectorClass<PersistantSurfaceEmitterClass *>	WheelSurfaceEmitters;
	PersistantSurfaceSoundClass *						WheelSurfaceSound;

	// Occupants
	enum {
		DRIVER_SEAT = 0,
		GUNNER_SEAT,
	};

	VectorClass<SoldierGameObj *>	SeatOccupants;

	int		OccupiedSeats;

	DynamicVectorClass<TransitionInstanceClass *> TransitionInstances;

	// Lock feature.  Vehicle can be locked to all but the purchaser for some number of seconds
	GameObjReference		LockOwner;	// who owns the lock for this vehicle?
	float						LockTimer;

	void		Remove_Transitions( TransitionDataClass::StyleType transition_type );
	void		Create_New_Transitions( TransitionDataClass::StyleType transition_type );
	void		Destroy_Transitions( void );
	void		Update_Transitions( void );

	void		Create_And_Destroy_Transitions( void );


	// Turrets
	void		Aquire_Turret_Bones( void );
	void		Release_Turret_Bones( void );
	void		Update_Turret( float weapon_turn, float weapon_tilt );


	// Sound Effects (wheels, engine)
	void		Update_Sound_Effects( void );
	void		Change_Engine_Sound_State( int new_state );
	void		Update_Engine_Sound_Pitch( void );

};


#endif	//	VEHICLE_H
