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
 *                     $Archive:: /Commando/Code/Combat/inventory.cpp                         $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/15/01 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "inventory.h"
#include "debug.h"
#include "soldier.h"
#include "weaponbag.h"
#include "weapons.h"

/*
**
*/
InventoryClass::InventoryClass( void )
{
	Reset();
}

/*
**
*/
InventoryClass::~InventoryClass( void )
{
}

/*
**
*/
void InventoryClass::Reset( void )
{
	while ( WeaponAmmoList.Count() != 0 ) {
		WeaponAmmoList.Delete( 0 );
	}
	ShieldType = 0;
	ShieldStrength = 0.0f;
	ShieldStrengthMax = 0.0f;
	Health = 0.0f;
	HealthMax = 0.0f;
}


/*
**
*/
void	InventoryClass::Store_Inventory( SoldierGameObj * soldier )
{
	Reset();

	WWASSERT( soldier );
	ShieldType = soldier->Get_Defense_Object()->Get_Shield_Type();
	ShieldStrength = soldier->Get_Defense_Object()->Get_Shield_Strength();
	ShieldStrengthMax = soldier->Get_Defense_Object()->Get_Shield_Strength_Max();
	Health = soldier->Get_Defense_Object()->Get_Health();
	HealthMax = soldier->Get_Defense_Object()->Get_Health_Max();
	soldier->Get_Weapon_Bag()->Store_Inventory( this );
}

/*
**
*/
void	InventoryClass::Add_Weapon( int id, int rounds, bool has_weapon )
{
	WeaponAmmo entry;
	entry.WeaponID = id;
	entry.AmmoCount = rounds;
	entry.HasWeapon = has_weapon;
	WeaponAmmoList.Add( entry );
}

/*
**
*/
void	InventoryClass::Restore_Inventory( SoldierGameObj * soldier )
{
	DefenseObjectClass * def_obj = soldier->Get_Defense_Object();
	if ( (unsigned int)ShieldType > def_obj->Get_Shield_Type() ) {
		def_obj->Set_Shield_Type( ShieldType );
	}
	if ( ShieldStrengthMax > def_obj->Get_Shield_Strength_Max() ) {
		def_obj->Set_Shield_Strength_Max( ShieldStrengthMax );
	}
	if ( ShieldStrength > def_obj->Get_Shield_Strength() ) {
		def_obj->Set_Shield_Strength( ShieldStrength );
	}
	if ( HealthMax > def_obj->Get_Health_Max() ) {
		def_obj->Set_Health_Max( HealthMax );
	}
	if ( Health > def_obj->Get_Health() ) {
		def_obj->Set_Health( Health );
	}

	WeaponBagClass * bag = soldier->Get_Weapon_Bag();
	for ( int i = 0; i < WeaponAmmoList.Count(); i++ ) {
		const WeaponAmmo & entry = WeaponAmmoList[i];
		WeaponClass * 	weapon = bag->Add_Weapon( entry.WeaponID, 0, entry.HasWeapon );
		if ( entry.AmmoCount == -1 ) {
			weapon->Add_Rounds( -1 );
		} else if ( weapon->Get_Total_Rounds() < entry.AmmoCount ) {
			weapon->Add_Rounds( entry.AmmoCount - weapon->Get_Total_Rounds() );
		}

	}
}

