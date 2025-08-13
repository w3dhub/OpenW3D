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
 *                     $Archive:: /Commando/Code/Commando/clientfps.cpp               $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 9/20/01 11:28a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 2                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "clientfps.h"

#include "networkobjectfactory.h"
#include "networkobjectmgr.h"
#include "cnetwork.h"
#include "playermanager.h"
#include "apppackettypes.h"


CClientFps *		PClientFps = NULL;

DECLARE_NETWORKOBJECT_FACTORY(CClientFps, NETCLASSID_CLIENTFPS);

// FIXME (TSS) high priority for me to fix this CClientFps bug...
//
// TSS2001 problem: destruction of this object on the server. Quitting and rejoining 
// a game will crash the server.
//

//-----------------------------------------------------------------------------
CClientFps::CClientFps(void)
{
	ClientId	= -1;
	Fps		= 0;

	Set_App_Packet_Type(APPPACKETTYPE_CLIENTFPS);
}

//-----------------------------------------------------------------------------
CClientFps::~CClientFps(void)
{
}

//-----------------------------------------------------------------------------
void
CClientFps::Init(void)
{
	WWASSERT(cNetwork::I_Am_Client());

	ClientId = cNetwork::Get_My_Id();

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	Set_Object_Dirty_Bit(0, NetworkObjectClass::BIT_CREATION, true);
}

//-----------------------------------------------------------------------------
void
CClientFps::Set_Fps(int fps)
{
	WWASSERT(cNetwork::I_Am_Client());

	WWASSERT(fps >= 0);

	Fps = (BYTE) fps;

	Set_Object_Dirty_Bit(0, NetworkObjectClass::BIT_FREQUENT, true);
}

//-----------------------------------------------------------------------------
void
CClientFps::Act(void)
{
	WWASSERT(cNetwork::I_Am_Server());

	cPlayer * p_player = cPlayerManager::Find_Player(ClientId);
	if (p_player != NULL)
	{
		p_player->Set_Fps(Fps);
	}
}

//-----------------------------------------------------------------------------
void
CClientFps::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Client());

	NetworkObjectClass::Export_Creation(packet);

	packet.Add(ClientId);
}

//-----------------------------------------------------------------------------
void
CClientFps::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	NetworkObjectClass::Import_Creation(packet);

	packet.Get(ClientId);
}

//-----------------------------------------------------------------------------
void
CClientFps::Export_Frequent(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Client());

	packet.Add(Fps);
}

//-----------------------------------------------------------------------------
void
CClientFps::Import_Frequent(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	packet.Get(Fps);

	Act();
}

