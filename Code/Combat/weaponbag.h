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
 *                     $Archive:: /Commando/Code/Combat/weaponbag.h                           $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/10/01 11:52a                                             $*
 *                                                                                             *
 *                    $Revision:: 33                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	WEAPONBAG_H
#define	WEAPONBAG_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	VECTOR_H
	#include "vector.h"
#endif

/*
**
*/
class	WeaponClass;
class	ChunkSaveClass;
class	ChunkLoadClass;
class	ArmedGameObj;
class	WeaponDefinitionClass;
class	AmmoDefinitionClass;
class BitStreamClass;
class InventoryClass;

/*
** WeaponBags manage collections of weapons
*/
class	WeaponBagClass {

public:
	WeaponBagClass( ArmedGameObj * owner );
	~WeaponBagClass( void );

	bool				Save( ChunkSaveClass & csave );
	bool				Load( ChunkLoadClass & cload );

	bool				Is_Weapon_Owned( int weapon_id );
	bool				Is_Ammo_Full( int weapon_id );

	WeaponClass * 	Add_Weapon( const WeaponDefinitionClass * def, int rounds = 0, bool give_weapon = true );
	WeaponClass * 	Add_Weapon( int id, int rounds = 0, bool give_weapon = true );
	WeaponClass * 	Add_Weapon( const char *weapon_name, int rounds = 0, bool give_weapon = true );
	void				Remove_Weapon( int index );

	void				Clear_Weapons( void );

	int				Get_Count( void )					{ return WeaponList.Count(); }
	WeaponClass *	Peek_Weapon( int index )		{ return WeaponList[ index ]; }
	WeaponClass	*	Get_Weapon( void )				{ return	WeaponList[ WeaponIndex ]; }
	WeaponClass	*	Get_Next_Weapon( void );

	void				Import_Weapon_List(BitStreamClass & packet);
	void				Export_Weapon_List(BitStreamClass & packet);

	int				Get_Index( void )					{ return WeaponIndex; }
	void				Select_Index( int index );
	void				Select_Next( void );
	void				Select_Prev( void );
	void				Select_Key_Number( int key_number );
	void				Select_Weapon( WeaponClass * weapon );
	void				Select_Weapon_ID( int weapon_id );
	void				Select_Weapon_Name( const char * name );
	void				Deselect( void );

 	bool				Is_Changed( void )					{ return IsChanged; }
 	void				Force_Changed( void )				{ IsChanged = true; }
   void				Reset_Changed( void )				{ IsChanged = false; }

 	bool				HUD_Is_Changed( void )					{ return HUDIsChanged; }
   void				HUD_Reset_Changed( void )				{ HUDIsChanged = false; }

	// returns if anything was moved
	bool				Move_Contents( WeaponBagClass * source );

	void				Store_Inventory( InventoryClass * );
	void				Restore_Inventory( InventoryClass * );

private:
	ArmedGameObj						*	Owner;
	DynamicVectorClass<WeaponClass*>	WeaponList;
	int										WeaponIndex;
	bool										IsChanged;
	bool										HUDIsChanged;

	WeaponClass *	Find_Weapon( const WeaponDefinitionClass * def );

	void		Mark_Owner_Dirty( void );

};


#endif	// WEAPONBAG_H
