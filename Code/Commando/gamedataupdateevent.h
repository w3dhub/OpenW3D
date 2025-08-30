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
 *                     $Archive:: /Commando/Code/Commando/gamedataupdateevent.h                      $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/17/02 8:32p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __GAMEDATAUPDATEEVENT_H__
#define __GAMEDATAUPDATEEVENT_H__

#include "netevent.h"
#include "netclassids.h"

//-----------------------------------------------------------------------------
//
// A S->C mirrored object to represent gamedata updates
//
class	cGameDataUpdateEvent : public cNetEvent
{
public:
   cGameDataUpdateEvent(void);

	void						Init(int client_id = -1);
	virtual void			Export_Creation(BitStreamClass &packet) override;
	virtual void			Import_Creation(BitStreamClass &packet) override;
	virtual uint32			Get_Network_Class_ID(void) const override				{return NETCLASSID_GAMEDATAUPDATEEVENT;}

private:

	virtual void			Act(void) override;

	int						TimeRemainingSeconds;
	//bool						ServerIsGameplayPermitted;
	int						HostedGameNumber;
};

//-----------------------------------------------------------------------------

#endif	// __GAMEDATAUPDATEEVENT_H__
