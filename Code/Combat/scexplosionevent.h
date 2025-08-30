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
 *                     $Archive:: /Commando/Code/Combat/scexplosionevent.h                        $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/09/02 3:12p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 3                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __SCEXPLOSIONEVENT_H__
#define __SCEXPLOSIONEVENT_H__

#include "networkobject.h"
#include "netclassids.h"
#include "vector3.h"

//-----------------------------------------------------------------------------
//
// A S->C mirrored object to represent an explosion
//
class	cScExplosionEvent : public NetworkObjectClass
{
public:
   cScExplosionEvent(void);
	void						Init(int def_id, const Vector3 & position, int owner_id, int victim_id );
	virtual void			Delete(void) override										{delete this;}

	virtual void			Export_Creation(BitStreamClass &packet) override;
	virtual void			Import_Creation(BitStreamClass &packet) override;
	virtual uint32			Get_Network_Class_ID(void) const override				{return NETCLASSID_SCEXPLOSIONEVENT;}

private:

	virtual void			Act(void);

	int						DefID;
	Vector3					Position;
	int						OwnerID;
	int						VictimID;

};

//-----------------------------------------------------------------------------

#endif	// __SCEXPLOSIONEVENT_H__
