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
 *                     $Archive:: /Commando/Code/Combat/csdamageevent.h                             $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 11/24/01 10:34a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 3                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __CSDAMAGEEVENT_H__
#define __CSDAMAGEEVENT_H__

#include "networkobject.h"
#include "netclassids.h"

//-----------------------------------------------------------------------------
//
// A C->S reliably mirrored event for client authoritative damage
//
class	cCsDamageEvent : public NetworkObjectClass
{
public:
   cCsDamageEvent(void);

	void						Init( int damager_go_id, int damagee_go_id, float damage, int warhead );

	virtual void			Export_Creation(BitStreamClass &packet) override;
	virtual void			Import_Creation(BitStreamClass &packet) override;

	virtual uint32			Get_Network_Class_ID(void) const override				{return NETCLASSID_CSDAMAGEEVENT;}
	virtual void			Delete(void) override										{delete this;}

	static void				Set_Are_Clients_Trusted(bool flag)			{AreClientsTrusted = flag;}
	static bool				Get_Are_Clients_Trusted(void)					{return AreClientsTrusted;}

private:

	virtual void			Act(void);

	int						SenderId;
	int						DamagerGOID;
	int						DamageeGOID;
	float						Damage;
	int						Warhead;

	static bool				AreClientsTrusted;
};

//-----------------------------------------------------------------------------

#endif	// __CSDAMAGEEVENT_H__
