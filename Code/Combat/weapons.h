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
 *                     $Archive:: /Commando/Code/Combat/weapons.h                             $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/25/02 2:08p                                               $*
 *                                                                                             *
 *                    $Revision:: 108                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	WEAPONS_H
#define	WEAPONS_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	GAMEOBJREF_H
   #include "gameobjref.h"
#endif

#ifndef	MATRIX3D_H
	#include "matrix3d.h"
#endif

#ifndef	WEAPONMANAGER_H
	#include "weaponmanager.h"
#endif

#ifndef	SIMPLEVEC_H
	#include "simplevec.h"
#endif

class	BitStreamClass;
class	ChunkSaveClass;
class	ChunkLoadClass;
class	RenderObjClass;
class	ArmedGameObj;
class	ParticleEmitterClass;
class	AudibleSoundClass;
class	PhysicalGameObj;

/*
**
*/
typedef	enum {
	WEAPON_HOLD_STYLE_C4					= 0,
	WEAPON_HOLD_STYLE_NOT_USED,
	WEAPON_HOLD_STYLE_AT_SHOULDER,	// 2
	WEAPON_HOLD_STYLE_AT_HIP,
	WEAPON_HOLD_STYLE_LAUNCHER,
	WEAPON_HOLD_STYLE_HANDGUN,
	WEAPON_HOLD_STYLE_BEACON,
	WEAPON_HOLD_STYLE_EMPTY_HANDS,
	WEAPON_HOLD_STYLE_AT_CHEST,
	WEAPON_HOLD_STYLE_HANDS_DOWN,
	NUM_WEAPON_HOLD_STYLES
} WeaponHoldStyleType;


/*
**
*/
typedef	enum {
	WEAPON_MODEL_UPDATE_WILL_BE_NEEDED,
	WEAPON_MODEL_UPDATE_IS_NEEDED,
	WEAPON_MODEL_UPDATE_NOT_NEEDED,
} WeaponModelUpdateState;

/*
**
*/
typedef	enum {
	WEAPON_ANIM_NOT_FIRING,
	WEAPON_ANIM_FIRING_0,
	WEAPON_ANIM_FIRING_1
} WeaponAnimState;


/*
**
*/
class	MuzzleFlashClass {

public:
	MuzzleFlashClass( void );
	~MuzzleFlashClass( void );

	void	Init( RenderObjClass * robj );

	void	Update( bool flashA0, bool flashA1 );

private:

	int					MuzzleA0Bone;
	int					MuzzleA1Bone;
	float					Rotation;
	RenderObjClass *	Model;

	bool					LastFlashA0;
	bool					LastFlashA1;
};

/*
**
*/
class	WeaponClass {

public:
	WeaponClass( const WeaponDefinitionClass *weapon_def = nullptr );
	~WeaponClass( void );

	void				Init( const WeaponDefinitionClass *weapon_def );

	bool				Save( ChunkSaveClass & csave );
	bool				Load( ChunkLoadClass & cload );

	void				Set_Model( RenderObjClass *model );
	void				Set_Owner( ArmedGameObj *owner );
	ArmedGameObj	*Get_Owner( void )				{ return (ArmedGameObj *)Owner.Get_Ptr();	}

	enum {
		MUZZLE_PRIMARY_0,
		MUZZLE_PRIMARY_1,
		MUZZLE_SECONDARY_0,
		MUZZLE_SECONDARY_1,
	};

	void				Select( void );
	void				Deselect( void );

	void				Update( void );

	bool				Is_Muzzle_Clear( void );
	void				Compute_Bullet_Start_Point(const Matrix3D & muzzle,Vector3 * set_start_point);

	void				Next_C4_Detonation_Mode( void );			// Take the weapon to the next state (for C4)

	const WeaponDefinitionClass *Get_Definition( void )		{ return Definition; }
	const char * 	Get_Name( void )		  		{ return Definition->Get_Name(); }
	int				Get_ID( void )		  			{ return Definition->Get_ID(); }
	const char *	Get_HUD_Icon_Texture_Name( void )	{ return Definition->HUDIconTextureName; }
	const char *	Get_Model_Name( void ) 		{ return Definition->Model; }
	const char *	Get_Back_Model_Name( void ){ return Definition->BackModel; }
	const char *	Get_Anim_Name( void )  		{ return (NextAnimState != WEAPON_ANIM_NOT_FIRING) ? Definition->FireAnim : Definition->IdleAnim; }
	int				Get_Style( void )		  		{ return Definition->Style; }
	float			Get_Key_Number( void )		{ return Definition->KeyNumber; }
	bool				Get_Can_Snipe( void )		{ return Definition->CanSnipe; }
	float				Get_Rating( void )		  	{ return Definition->Rating; }
	float				Get_Effective_Range( void ){ return PrimaryAmmoDefinition->EffectiveRange; }
	float				Get_Primary_Fire_Rate( void ){ return PrimaryAmmoDefinition->RateOfFire; }

	const char * 	Get_First_Person_Model_Name( void )	const		{ return Definition->FirstPersonModel; }
	const Vector3& Get_First_Person_Model_Offset( void ) const	{ return Definition->FirstPersonOffset; }

	float				Get_Recoil_Time( void )		{ return Definition->RecoilTime; }
	float				Get_Recoil_Scale( void )	{ return Definition->RecoilScale; }

	// =========== Ammo
	void				Set_Total_Rounds( int num );
	int				Get_Total_Rounds( void );
	void				Add_Rounds( int num );
	void				Set_Clip_Rounds( int rounds )			{ ClipRounds = rounds; }
	int				Get_Clip_Rounds( void )					{ return ClipRounds; }
	void				Set_Inventory_Rounds( int rounds )	{ InventoryRounds = rounds; }
	int				Get_Inventory_Rounds( void )			{ return InventoryRounds; }
	void				Do_Reload( void );
	void				Decrement_Rounds( int rounds = 1 );
	bool				Is_Ammo_Maxed( void );
	bool				Is_Loaded( void )							{ return (ClipRounds != 0); }
	bool				Is_Reload_OK( void );
	bool				Is_Reload_Needed( void )				{ return !Is_Loaded() && Is_Reload_OK(); }
	float				Get_Range( void );

   bool				Is_Model_Update_Needed( void )	{ return (UpdateModel == WEAPON_MODEL_UPDATE_IS_NEEDED); }
	void				Reset_Model_Update( void )			{ UpdateModel = WEAPON_MODEL_UPDATE_NOT_NEEDED; CurrentAnimState = (WeaponAnimState)-1; }
   bool				Is_Anim_Update_Needed( void )		{ return ( NextAnimState != CurrentAnimState ); }
	void				Reset_Anim_Update( void )			{ CurrentAnimState = NextAnimState; }
	WeaponAnimState	Get_Anim_State( void )			{ return CurrentAnimState; }

	int				Get_Total_Rounds_Fired( void )	{ return TotalRoundsFired; }

	void				Display_Targeting( void );

	void				Set_Target( const Vector3 &target)	{ Target = target; }
	const Vector3 &Get_Target( void )						{ return Target; }

	void				Set_Target_Object( PhysicalGameObj * obj );
	PhysicalGameObj *	Get_Target_Object( PhysicalGameObj * obj );

	/*
	** Weapons States
	*/
	typedef	enum {
		STATE_IDLE,
		STATE_READY,
		STATE_CHARGE,
		STATE_FIRE_PRIMARY,
		STATE_FIRE_SECONDARY,
		STATE_RELOAD,
		STATE_START_SWITCH,
		STATE_END_SWITCH,
//		NUM_STATES,
	} WeaponStateType;

	WeaponStateType	Get_State( void )					{ return State; }
   bool				Is_Firing( void )						{ return DidFire; }
   bool				Is_Reloading( void )					{ return (State == STATE_RELOAD); }
   bool				Is_Switching( void )					{ return (State == STATE_START_SWITCH) || (State == STATE_END_SWITCH); }
	void				Force_Reload( void );

	// Trigger State
	void				Set_Safety( bool saftey )			{ SafetySet = saftey; }
	bool				Is_Safety_Set( void )				{ return SafetySet; }
	void				Set_Primary_Triggered( bool triggered );
	void				Set_Secondary_Triggered( bool triggered );
	bool				Is_Triggered( void )	{ return IsSecondaryTriggered || IsPrimaryTriggered; }

	void				Init_Muzzle_Flash( RenderObjClass * robj )	{ MuzzleFlash.Init( robj ); }
	void				Update_Muzzle_Flash( bool flashA0, bool flashA1 ) 	{ MuzzleFlash.Update( flashA0, flashA1 ); }

	bool				Does_Weapon_Exist( void )				{ return WeaponExists; }
	void				Set_Weapon_Exists( bool exists )		{ WeaponExists = exists; }

	PhysicalGameObj * Cast_Weapon( const Vector3 & target );
	bool				Cast_Weapon_Down_Muzzle( Vector3 & hit_pos );

	void				Make_Shell_Eject( const Matrix3D & tm );

	//
	//	Sound support
	//
	void				Stop_Firing_Sound( void );

private:

	void				Fire_C4( const AmmoDefinitionClass *ammo_def );
	bool				Fire_Beacon( const AmmoDefinitionClass *ammo_def );
	void				Fire_Bullet( const AmmoDefinitionClass *ammo_def, bool primary );

	// Weapon Info
	const WeaponDefinitionClass *Definition;
	GameObjReference				Owner;				// who gets credit for this weapon's kills?
	RenderObjClass					*Model;
	int								C4DetonationMode;

	// Ammo Info
	const AmmoDefinitionClass *PrimaryAmmoDefinition;
	const AmmoDefinitionClass *SecondaryAmmoDefinition;
	safe_int							ClipRounds;
	safe_int							InventoryRounds;		// -1 for unlimited
	safe_float						BurstDelayTimer;
	safe_int							BurstCount;
	float								BulletBumpTime;
	bool								WeaponExists;			// allows us to have ammo with no weapon

	// Weapon States
   WeaponStateType				State;				// The state of the weapon
	float								StateTimer;			// in seconds. Used for switching, reloading, and idle
	bool								DidFire;

	bool								LastFrameIsPrimaryTriggered;		// is the Trigger pulled
	bool								LastFrameIsSecondaryTriggered;	// is the Trigger pulled
	bool								IsPrimaryTriggered;					// is the Trigger pulled
	bool								IsSecondaryTriggered;				// is the Trigger pulled
	int								TotalRoundsFired;
	bool								SafetySet;
	bool								LockTriggers;
	float								EmptySoundTimer;

	// Weapon Looks
	WeaponAnimState				NextAnimState;		// Which anim should we be on
	WeaponAnimState				CurrentAnimState;	// Which anim are we on
	WeaponModelUpdateState		UpdateModel;		// Does the weapon Model need to be updated

	Vector3							Target;
	GameObjReference				TargetObject;

	// Continuous Muzzle Effects (flames)
	SimpleVecClass<ParticleEmitterClass *>	ContinuousEmitters;
	AudibleSoundClass *			ContinuousSound;

	AudibleSoundClass *			FiringSound;
	int								FiringSoundDefID;

	// Functions
	const Matrix3D & Get_Muzzle( int index = MUZZLE_PRIMARY_0 );
	void				Do_Fire( bool primary );

	// Weapon State
	void				Set_State( WeaponStateType new_state );
	void				Update_State( float dtime );
	void				Do_Firing_Effects( void );
	void				Do_Continuous_Effects( bool enable );

	// Muzzle Flash
	MuzzleFlashClass		MuzzleFlash;

	void				Ignore_Owner( void );
	void				Unignore_Owner( void );

};

#endif		// WEAPONS_H
