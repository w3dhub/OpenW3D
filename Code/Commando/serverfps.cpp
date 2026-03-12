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
 *                     $Archive:: /Commando/Code/Commando/serverfps.cpp               $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 11/10/01 1:06p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "serverfps.h"

#include "cnetwork.h"
#include "gameobjmanager.h"
#include "apppackettypes.h"

//
// Class statics
//
cServerFps *	cServerFps::TheInstance	= NULL;

//-----------------------------------------------------------------------------
cServerFps::cServerFps(void)
{
	Set_Network_ID(NETID_SERVER_FPS);

	Fps = 0;

	Set_App_Packet_Type(APPPACKETTYPE_SERVERFPS);
}

//-----------------------------------------------------------------------------
void
cServerFps::Set_Fps(int fps)
{
	WWASSERT(fps >= 0);
	WWASSERT(cNetwork::I_Am_Server());

	Fps = fps;

	Set_Object_Dirty_Bit(NetworkObjectClass::BIT_FREQUENT, true);
}

//-----------------------------------------------------------------------------
void
cServerFps::Export_Frequent(BitStreamClass &packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	packet.Add(Fps);
}

//-----------------------------------------------------------------------------
void
cServerFps::Import_Frequent(BitStreamClass &packet)
{
	WWASSERT(cNetwork::I_Am_Client());

	packet.Get(Fps);
}

//-----------------------------------------------------------------------------
void
cServerFps::Create_Instance
(
	void
)
{
	WWASSERT(TheInstance == NULL);
	TheInstance = new cServerFps;
}

//-----------------------------------------------------------------------------
void
cServerFps::Destroy_Instance
(
	void
)
{
	WWASSERT(TheInstance != NULL);
	delete TheInstance;
	TheInstance = NULL;
}

//-----------------------------------------------------------------------------
cServerFps *
cServerFps::Get_Instance
(
	void
)
{
	return TheInstance;
}



