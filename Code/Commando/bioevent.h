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
 *                     $Archive:: /Commando/Code/Commando/bioevent.h                           $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/18/02 1:23p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __BIOEVENT_H__
#define __BIOEVENT_H__

#include "netevent.h"
#include "netclassids.h"

//-----------------------------------------------------------------------------
//
// A C->S mirrored event object for transmitting initial player bio data.
//
class	cBioEvent : public cNetEvent
{
public:
   cBioEvent(void);

	void						Init(int team_choice, unsigned long clanID);

	virtual void			Export_Creation(BitStreamClass &packet) override;
	virtual void			Import_Creation(BitStreamClass &packet) override;

	virtual uint32			Get_Network_Class_ID(void) const override				{return NETCLASSID_BIOEVENT;}

private:

	virtual void			Act(void) override;

	int						SenderId;
	WideStringClass		Nickname;
	char						MapName[256];
	int TeamChoice;
	unsigned long ClanID;
};

//-----------------------------------------------------------------------------

#endif	// __BIOEVENT_H__


