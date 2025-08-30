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
 *                     $Archive:: /Commando/Code/Commando/requestkillevent.h                           $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 10/09/01 10:49a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 1                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __REQUESTKILLEVENT_H__
#define __REQUESTKILLEVENT_H__

#include "netevent.h"
#include "netclassids.h"

//-----------------------------------------------------------------------------
//
// A C->S mirrored object for requesting an object to be destroyed. Useful when 
// someone notices 2 vehicles stuck together, etc.
//
class	cRequestKillEvent : public cNetEvent
{
public:
   cRequestKillEvent(void);

	void						Init(int object_id);

	virtual void			Export_Creation(BitStreamClass &packet) override;
	virtual void			Import_Creation(BitStreamClass &packet) override;

	virtual uint32			Get_Network_Class_ID(void) const override				{return NETCLASSID_REQUESTKILLEVENT;}

private:

	virtual void			Act(void) override;

	int						ObjectId;
};

//-----------------------------------------------------------------------------

#endif	// __REQUESTKILLEVENT_H__
