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
 *                     $Archive:: /Commando/Code/Commando/gamespyauthmgr.h                           $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 2/22/02 4:35p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 2                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __GAMESPYAUTHMGR_H__
#define __GAMESPYAUTHMGR_H__

#include "bittype.h"

//-----------------------------------------------------------------------------
enum GAMESPY_AUTH_STATE_ENUM
{
	GAMESPY_AUTH_STATE_INITIAL,			// We haven't begun yet.
	GAMESPY_AUTH_STATE_CHALLENGED,		// We are waiting for a response from the client.
	GAMESPY_AUTH_STATE_VALIDATING,		// We are waiting for validation from the validation server.
	GAMESPY_AUTH_STATE_ACCEPTED,			// This client passed.
	GAMESPY_AUTH_STATE_REJECTING,			// This client failed, we are in the process of booting him.
	GAMESPY_AUTH_STATE_REJECTED,			// Poor bugger.

	GAMESPY_AUTH_STATE_COUNT
};

//-----------------------------------------------------------------------------
class	cGameSpyAuthMgr
{
public:
	static void		Think(void);
	static void		Initiate_Auth_Rejection(int player_id);
	static const char *Describe_Auth_State(GAMESPY_AUTH_STATE_ENUM state);

private:
	static void		Evict_Player(int player_id);

	enum				{CHALLENGE_RESPONSE_TIMEOUT_MS = 30000};
	enum				{REJECTION_DELAY_MS = 3000};
};

//-----------------------------------------------------------------------------

#endif	// __GAMESPYAUTHMGR_H__



