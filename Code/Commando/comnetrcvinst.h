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
 *                     $Archive:: /Commando/Code/Commando/comnetrcvinst.h                     $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 9/28/01 5:01p                                               $*
 *                                                                                             *
 *                    $Revision:: 63                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSV_VER)
#pragma once
#endif

#ifndef COMNETRCVINST_H
#define COMNETRCVINST_H

#include "comnetrcv.h"
#include "gamedata.h"

/*
** Our Network Packet Receiver
*/
class CombatNetworkReceiverInstanceClass : public	CombatNetworkReceiver {

public:
	// Update Object States
	virtual bool Server_Update_Dynamic_Objects(bool is_urgent = false) override;
	virtual void Server_Send_Delete_Notifications(void) override;
	virtual bool Client_Update_Dynamic_Objects(bool is_urgent = false) override;

	void	Print( const char *format, ... );
	void	Print( const Vector3 & color, const char *format, ... );
};

#endif // COMNETRCVINST_H
