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
 *                     $Archive:: /Commando/Code/Combat/inventory.h                           $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/15/01 11:36a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	INVENTORY_H
#define	INVENTORY_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	SIMPLEVEC_H
	#include "simplevec.h"
#endif

#ifndef	DAMAGE_H
	#include "damage.h"
#endif

class	SoldierGameObj;

/*
** InventoryClass is a black box used to remember the inventory of a soldier across levels
*/
class	InventoryClass {

public:
	InventoryClass( void );
	~InventoryClass( void );

	void	Reset( void );

	void	Store_Inventory( SoldierGameObj * soldier );
	void	Restore_Inventory( SoldierGameObj * soldier );

	void	Add_Weapon( int id, int rounds, bool has_weapon );

private:

	struct WeaponAmmo {
		int	WeaponID;
		int	AmmoCount;
		bool	HasWeapon;
	};

	SimpleDynVecClass<WeaponAmmo>	WeaponAmmoList;
	ArmorType	ShieldType;
	float			ShieldStrength;
	float			ShieldStrengthMax;
	float			Health;
	float			HealthMax;
};

#endif


