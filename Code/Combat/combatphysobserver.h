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
 *                     $Archive:: /Commando/Code/Combat/combatphysobserver.h                  $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/08/01 11:23a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	COMBATPHYSOBSERVER_H
#define	COMBATPHYSOBSERVER_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	PHYSOBSERVER_H
	#include "physobserver.h"
#endif

class	DamageableGameObj;
class	PhysicalGameObj;
class BuildingGameObj;

class	CombatPhysObserverClass : public PhysObserverClass {

public:
	virtual	DamageableGameObj *	As_DamageableGameObj( void )	{ return NULL; }
	virtual	PhysicalGameObj *		As_PhysicalGameObj( void )		{ return NULL; }
	virtual	BuildingGameObj *		As_BuildingGameObj( void )		{ return NULL; }
};

#endif	//	COMBATPHYSOBSERVER_H
