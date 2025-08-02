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
 *                     $Archive:: /Commando/Code/Combat/weaponmanager.h                       $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/02/02 3:05p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 64                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	WEAPONMANAGER_H
#define	WEAPONMANAGER_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	WWSTRING_H
	#include "wwstring.h"
#endif

#ifndef	DEFINITION_H
	#include "definition.h"
#endif

#ifndef	_DATASAFE_H
    #include	"../Commando/datasafe.h"
#endif		//_DATASAFE_H

class		WeaponClass;

/*
** Weapon Definition Structure - These describe the basic functionality of
** a weapon, and CANNOT be modified by instances
*/
class	WeaponDefinitionClass : public DefinitionClass {

public:
	WeaponDefinitionClass( void );

	virtual uint32								Get_Class_ID( void ) const;
	virtual PersistClass *					Create( void ) const					{ WWASSERT( 0 ); return NULL; }
	virtual bool								Save( ChunkSaveClass &csave );
	virtual bool								Load( ChunkLoadClass &cload );
	virtual const PersistFactoryClass &	Get_Factory( void ) const;

	DECLARE_EDITABLE( WeaponDefinitionClass, DefinitionClass );

	bool operator == ( const WeaponDefinitionClass & vector) const  { return false; }
	bool operator != ( const WeaponDefinitionClass & vector) const  { return true; }

	int				Style;
	StringClass		Model;
	StringClass		IdleAnim;
	StringClass		FireAnim;
	StringClass		BackModel;
//	int				LegacyFireSoundDefID;
	float				SwitchTime;
	safe_float		ReloadTime;
	float				KeyNumber;
	bool				CanSnipe;
	bool				CanReceiveGenericCnCAmmo;
	float				Rating;
	int				EjectPhysDefID;
	int				MuzzleFlashPhysDefID;
	StringClass		FirstPersonModel;
	Vector3			FirstPersonOffset;
	float				RecoilImpulse;
	StringClass		HUDIconTextureName;
	int				ReloadSoundDefID;
	int				EmptySoundDefID;
	int				PrimaryAmmoDefID;
	int				SecondaryAmmoDefID;
	safe_int			ClipSize;
	float				RecoilTime;
	float				RecoilScale;
	bool				AGiveWeaponsWeapon;
	safe_int			MaxInventoryRounds;

	int				IconNameID;
	StringClass		IconTextureName;
	RectClass		IconTextureUV;
	Vector2			IconOffset;

	StringClass		HumanFiringAnimation;
};


/*
**
*/
class	AmmoDefinitionClass : public DefinitionClass {

public:
	enum {
		AMMO_TYPE_NORMAL,
		AMMO_TYPE_C4_REMOTE,
		AMMO_TYPE_C4_TIMED,
		AMMO_TYPE_C4_PROXIMITY,
	};

	AmmoDefinitionClass( void );

	virtual uint32								Get_Class_ID( void ) const;
	virtual PersistClass *					Create( void ) const					{ WWASSERT( 0 ); return NULL; }
	virtual bool								Save( ChunkSaveClass &csave );
	virtual bool								Load( ChunkLoadClass &cload );
	virtual const PersistFactoryClass &	Get_Factory( void ) const;

	DECLARE_EDITABLE( AmmoDefinitionClass, DefinitionClass );

	bool operator == ( const AmmoDefinitionClass & vector) const { return false; }
	bool operator != ( const AmmoDefinitionClass & vector) const	{ return true; }

	StringClass		ModelFilename;
	StringClass		ModelName;
	safe_int			Warhead;
	safe_float		Damage;
	safe_float		Range;
	safe_float		Velocity;
	float				Gravity;
	float				Elasticity;
	float				RateOfFire;
	float				SprayAngle;
	safe_int			SprayCount;
	StringClass		TrailEmitter;
	float				AquireTime;
	float				BurstDelayTime;
	safe_int			BurstMax;
	int				SoftPierceLimit;
	float				TurnRate;
	bool				TimeActivated;
	bool				TerrainActivated;
	bool				IsTracking;
	safe_float		EffectiveRange;
	int				ExplosionDefID;
	float				RandomTrackingScale;
	bool				DisplayLaser;
	safe_float		ChargeTime;
	int				ContinuousSoundDefID;
	int				FireSoundDefID;
	StringClass		ContinuousEmitterName;
	int				MaxBounces;
	int				SprayBulletCost;
	safe_int			AmmoType;
	float				C4TriggerTime1;
	float				C4TriggerTime2;
	float				C4TriggerTime3;
	float				C4TriggerRange1;
	float				C4TriggerRange2;
	float				C4TriggerRange3;
	int				C4TimingSound1ID;
	int				C4TimingSound2ID;
	int				C4TimingSound3ID;
	float				AliasedSpeed;
	int				HitterType;
	int				BeaconDefID;

	bool				BeamEnabled;						// enable/disable usage of beam for instant bullets
	Vector3			BeamColor;
	float				BeamTime;							// seconds
	float				BeamWidth;							// meters
	bool				BeamEndCaps;						// put end-caps on the beam?
	StringClass		BeamTexture;						// texture filename
	bool				BeamSubdivisionEnabled;			// boolean; we use fixed subdivision count
	float				BeamSubdivisionScale;			// normalized amplitude.
	bool				BeamSubdivisionFrozen;			// animated or frozen subdivision

	int				IconNameID;
	StringClass		IconTextureName;
	RectClass		IconTextureUV;
	Vector2			IconOffset;

	float			GrenadeSafetyTime;
};


/*
** Weapon Manager
*/
class	WeaponManager {

public:
	static	const WeaponDefinitionClass	*Find_Weapon_Definition( const char *name );
	static	const WeaponDefinitionClass	*Find_Weapon_Definition( int id );
	static	const AmmoDefinitionClass		*Find_Ammo_Definition( const char *ammo_name );
	static	const AmmoDefinitionClass		*Find_Ammo_Definition( int id );

	static	bool	Is_Weapon_Help_Disabled( void )	{ return IsWeaponHelpDisabled; }
	static	void	Set_Weapon_Help_Disabled( bool state ) { IsWeaponHelpDisabled = state; }

private:
	static	bool									IsWeaponHelpDisabled;
};

#endif		// WEAPONMANAGER_H
