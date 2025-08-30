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
 *                     $Archive:: /Commando/Code/Combat/powerup.h                             $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 10/25/01 2:44p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 43                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	POWERUP_H
#define	POWERUP_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef SIMPLEGAMEOBJ_H
	#include "simplegameobj.h"
#endif

class		PowerUpGameObj;
class		AudibleSoundClass;
class		WeaponBagClass;

/*
** PowerUpGameObjDef - Defintion class for a PowerUpGameObj
*/
class PowerUpGameObjDef : public SimpleGameObjDef
{
public:
	PowerUpGameObjDef( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( PowerUpGameObjDef, SimpleGameObjDef );

	// Grant returns true if anything was granted that the grantee didn't already have
	bool											Grant( SmartGameObj * obj, PowerUpGameObj * p_powerup = NULL, bool hud_display = true ) const ;

	int											Get_Grant_Weapon_ID (void) const	{ return GrantWeaponID; }

protected:
	int											GrantShieldType;
	float											GrantShieldStrength;
	float											GrantShieldStrengthMax;
	float											GrantHealth;
	float											GrantHealthMax;
	int											GrantWeaponID;
	bool											GrantWeapon;
	int											GrantWeaponRounds;
	bool											GrantWeaponClips;
	bool											Persistent;
	//bool											IsCaptureTheFlag;
	int											GrantKey;
	bool											AlwaysAllowGrant;

	int											GrantSoundID;
	StringClass									GrantAnimationName;
	int											IdleSoundID;
	StringClass									IdleAnimationName;

	friend	class								PowerUpGameObj;

};


/*
**
*/
class PowerUpGameObj : public SimpleGameObj {

public:
				PowerUpGameObj( void );
	virtual ~PowerUpGameObj( void );

	// Definitions
	virtual	void	Init( void ) override;
	void	Init( const PowerUpGameObjDef & definition );
	const PowerUpGameObjDef & Get_Definition( void ) const ;

	// Save / Load / Construction Factory
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual	void	On_Post_Load( void ) override;		// MOVED
	virtual	const	PersistFactoryClass & Get_Factory( void ) const override;

	// Think
	virtual	void	Think( void ) override;
	void	Grant( SmartGameObj * obj );

	// Type identification
	virtual PowerUpGameObj		*As_PowerUpGameObj() override		{ return this; }

	static	PowerUpGameObj *	Create_Backpack( ArmedGameObj * provider );

	// Network support
	virtual	bool	Is_Always_Dirty( void ) override					{ return false; }
	//virtual	void	Get_Extended_Information( StringClass & description );
	virtual	void	Get_Description( StringClass & description ) override;

	void	Expire( void );

protected:

	// State types
	enum
	{
		STATE_BECOMING_IDLE			= 0,
		STATE_IDLING,
		STATE_GRANTING,
		STATE_EXPIRING,
	};

	void						Set_State( int state );
	void						Update_State( void );

	AudibleSoundClass *	IdleSoundObj;
	int						State;
	float						StateEndTimer;

	// For backpacks, which can hold multiple weapons and ammo
	WeaponBagClass *		WeaponBag;				

	friend	class			PowerUpGameObjDef;
};

#endif	// POWERUP_H

