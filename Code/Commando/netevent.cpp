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
 *                     $Archive:: /Commando/Code/Commando/netevent.cpp               $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 11/10/01 1:03p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "netevent.h"

#include "cnetwork.h"

//-----------------------------------------------------------------------------
void
cNetEvent::Init(void)
{
	WWASSERT(cNetwork::I_Am_Server());

	if (cNetwork::I_Am_Client())
	{
		Act();
	}

	//
	// This is a transient object. It is created, rendered, and destroyed immediately.
	//
	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cNetEvent::Export_Creation(BitStreamClass &packet)
{
	NetworkObjectClass::Export_Creation(packet);
}

//-----------------------------------------------------------------------------
void
cNetEvent::Import_Creation(BitStreamClass &packet)
{
	NetworkObjectClass::Import_Creation(packet);
}




