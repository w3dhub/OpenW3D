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
 *                     $Archive:: /Commando/Code/Commando/gamespycschallengeresponseevent.h                      $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 2/21/02 4:35p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 1                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __GAMESPYCSCHALLENGERESPONSEEVENT_H__
#define __GAMESPYCSCHALLENGERESPONSEEVENT_H__

#include "netevent.h"
#include "netclassids.h"

//-----------------------------------------------------------------------------
//
// A C->S mirrored object to respond to the server's GameSpy auth. challenge.
//
class	cGameSpyCsChallengeResponseEvent : public cNetEvent
{
public:

   cGameSpyCsChallengeResponseEvent(void);

	void						Init(StringClass & challenge_string);

	virtual void			Export_Creation(BitStreamClass &packet) override;
	virtual void			Import_Creation(BitStreamClass &packet) override;
	virtual uint32			Get_Network_Class_ID(void) const override				{return NETCLASSID_GAMESPYCSCHALLENGERESPONSEEVENT;}

private:

	virtual void			Act(void) override;

	int						ClientId;
	enum						{MAX_CHALLENGE_RESPONSE_STRING_LENGTH = 256};
	StringClass				ChallengeResponseString;
};

//-----------------------------------------------------------------------------

#endif	// __GAMESPYCSCHALLENGERESPONSEEVENT_H__
