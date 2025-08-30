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
 *                     $Archive:: /Commando/Code/Combat/armedgameobj.h                        $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 9/17/01 4:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 23                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	ARMEDGAMEOBJ_H
#define	ARMEDGAMEOBJ_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef PHYSICALGAMEOBJ_H
	#include "physicalgameobj.h"
#endif

#ifndef MUZZLERECOIL_H
	#include "muzzlerecoil.h"
#endif

/*
**
*/
class	WeaponClass;
class	WeaponBagClass;


/*
** ArmedGameObjDef - Defintion class for a ArmedGameObj
*/
class ArmedGameObjDef : public PhysicalGameObjDef
{
public:
	ArmedGameObjDef( void );

//	virtual uint32								Get_Class_ID (void) const override;
//	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;

//	virtual const PersistFactoryClass &	Get_Factory( void ) const;

	DECLARE_EDITABLE( ArmedGameObjDef, PhysicalGameObjDef );

protected:
	float											WeaponTiltRate;
	float											WeaponTiltMin;
	float											WeaponTiltMax;
	float											WeaponTurnRate;
	float											WeaponTurnMin;
	float											WeaponTurnMax;
	float											WeaponError;

  	int											WeaponDefID;
  	int											SecondaryWeaponDefID;
  	safe_int										WeaponRounds;

	friend	class								ArmedGameObj;
};


/*
**
*/
class ArmedGameObj : public PhysicalGameObj {

public:
	//	Constructor and Destructor
									ArmedGameObj( void );
	virtual						~ArmedGameObj( void );

	// Definitions
	void							Init( const ArmedGameObjDef & definition );
	void							Copy_Settings( const ArmedGameObjDef & definition );
	void							Re_Init( const ArmedGameObjDef & definition );
	const	ArmedGameObjDef & Get_Definition( void ) const ;

	// Save / Load
	virtual	bool				Save( ChunkSaveClass & csave ) override;
	virtual	bool				Load( ChunkLoadClass & cload ) override;
	virtual	void				On_Post_Load( void ) override;

	// State import/export
   virtual	void				Import_Frequent( BitStreamClass & packet ) override;
   virtual	void				Export_Frequent( BitStreamClass & packet ) override;

   virtual	void				Import_State_Cs( BitStreamClass & packet );
   virtual	void				Export_State_Cs( BitStreamClass & packet );

	//	Thinking
	virtual	void				Post_Think() override;

	// Weapon
	WeaponClass	*				Get_Weapon( void );
	WeaponBagClass	*			Get_Weapon_Bag( void )						{ return WeaponBag; }
	bool							Muzzle_Exists( int index = 0 );
   virtual const Matrix3D &Get_Muzzle( int index = 0 );
	void							Start_Recoil( int muzzle_index,float recoil_scale,float recoil_time );
	float							Get_Weapon_Error( void )					{ return Get_Definition().WeaponError; }

	// Targeting
	Vector3						Get_Targeting_Pos( void )					{ return TargetingPos; }
   virtual bool				Set_Targeting( const Vector3 & pos, bool do_tilt = true );

	// Type identification
	virtual	ArmedGameObj	*As_ArmedGameObj( void ) override					{ return this; }

protected:
	WeaponBagClass *			WeaponBag;				// Weapon & Ammo collection

private:
	Vector3						TargetingPos;

	int							MuzzleA0Bone;	// YUCK!!!
	int							MuzzleA1Bone;
	int							MuzzleB0Bone;
	int							MuzzleB1Bone;

	enum { MAX_MUZZLES = 4 };
	MuzzleRecoilClass			MuzzleRecoilController[MAX_MUZZLES];

	void							Init_Muzzle_Bones( void );
};


#endif	//	ARMEDGAMEOBJ_H
