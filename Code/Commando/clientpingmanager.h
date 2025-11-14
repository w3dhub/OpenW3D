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
 *                     $Archive:: /Commando/Code/Commando/clientpingmanager.h                           $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 10/15/01 4:34p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 2                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __CLIENTPINGMANAGER_H__
#define __CLIENTPINGMANAGER_H__

#include "bittype.h"

//-----------------------------------------------------------------------------
class	cClientPingManager
{
public:
	static void		Init(void);
	static void		Think(void);
	static unsigned int	Get_Last_Round_Trip_Ping_Ms(void);
	static unsigned int	Get_Avg_Round_Trip_Ping_Ms(void);
	static void		Response_Received(int ping_number);

private:
	static void		Compute_Average_Round_Trip_Ping_Ms(void);

	enum				{MAX_SAMPLES			= 3};
	enum				{MIN_PING_DELAY_MS	= 1000};

	static int		PingNumber;
	static unsigned int	TimeSentMs;
	static unsigned int	LastRoundTripPingMs;
	static unsigned int	AvgRoundTripPingMs;
	static bool		IsAwaitingResponse;
	static unsigned int	RoundTripPingSamplesMs[MAX_SAMPLES];
};

//-----------------------------------------------------------------------------

#endif	// __CLIENTPINGMANAGER_H__



