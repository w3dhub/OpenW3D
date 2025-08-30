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
 *                     $Archive:: /Commando/Code/Commando/nethandler.h                        $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 1/05/02 11:57a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 35                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSV_VER)
#pragma once
#endif

#ifndef NETHANDLER_H
#define NETHANDLER_H

#ifndef ALWAYS_H
	#include "always.h"
#endif

#ifndef COMBAT_H
   #include "combat.h"
#endif

//
// The combat lib calls these commando application-level methods.
// Other applications using the combat lib will not implement a class 
// like this one.
//

class	GameCombatNetworkHandlerClass : public CombatNetworkHandlerClass
{
public:
	bool	Can_Damage(ArmedGameObj * p_armed_damager, PhysicalGameObj * p_phys_victim) override;
	float	Get_Damage_Factor(ArmedGameObj * p_armed_damager, PhysicalGameObj * p_phys_victim) override;
	bool	Is_Gameplay_Permitted(void) override;
	void	On_Soldier_Kill(SoldierGameObj * p_soldier, SoldierGameObj * p_victim) override;
	void	On_Soldier_Death(SoldierGameObj * p_soldier) override;
};

#endif // NETHANDLER_H


