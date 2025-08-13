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
 *                     $Archive:: /Commando/Code/Combat/clientcontrol.cpp                 $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 9/21/01 10:44a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 6                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "clientcontrol.h"

#include "networkobjectfactory.h"
#include "combat.h"
#include "smartgameobj.h"
#include "gameobjmanager.h"
#include "apppackettypes.h"

CClientControl *		PClientControl = NULL;

DECLARE_NETWORKOBJECT_FACTORY(CClientControl, NETCLASSID_CLIENTCONTROL);

// FIXME (TSS) high priority for me to fix this CClientControl bug...
//
// TSS2001 problem: destruction of this object on the server. Quitting and rejoining 
// a game will crash the server.
//

//-----------------------------------------------------------------------------
CClientControl::CClientControl(void)
{
	ClientId		= -1;
	SmartObjId	= -1;

	Set_App_Packet_Type(APPPACKETTYPE_CLIENTCONTROL);
}

//-----------------------------------------------------------------------------
CClientControl::~CClientControl(void)
{
}

//-----------------------------------------------------------------------------
void
CClientControl::Init(void)
{
	WWASSERT(CombatManager::I_Am_Client());

	ClientId = CombatManager::Get_My_Id();

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	Set_Object_Dirty_Bit(0, NetworkObjectClass::BIT_CREATION, true);
}

//-----------------------------------------------------------------------------
void
CClientControl::Set_Update_Flag(int id)
{
	WWASSERT(CombatManager::I_Am_Client());

	SmartObjId = id;

	if (id != -1) {
		Set_Object_Dirty_Bit(0, NetworkObjectClass::BIT_FREQUENT, true);
	}
}

//-----------------------------------------------------------------------------
void
CClientControl::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(CombatManager::I_Am_Client());

	NetworkObjectClass::Export_Creation(packet);

	packet.Add(ClientId);
}

//-----------------------------------------------------------------------------
void
CClientControl::Import_Creation(BitStreamClass & packet)
{
	WWASSERT(CombatManager::I_Am_Server());

	NetworkObjectClass::Import_Creation(packet);

	packet.Get(ClientId);
}

//-----------------------------------------------------------------------------
void
CClientControl::Export_Frequent(BitStreamClass & packet)
{
	WWASSERT(CombatManager::I_Am_Client());

	/*
	packet.Add(SmartObjId);

	if (SmartObjId != -1) {
		SmartGameObj * p_smart_go = GameObjManager::Find_SmartGameObj(SmartObjId);
		WWASSERT(p_smart_go != NULL);

		p_smart_go->Export_Control_Cs(packet);
		p_smart_go->Export_State_Cs(packet);
	}
	*/

	SmartGameObj * p_smart_go = NULL;
	if (SmartObjId != -1) {
		p_smart_go = GameObjManager::Find_SmartGameObj(SmartObjId);
		if (p_smart_go == NULL) {
			SmartObjId = -1;
		}
	}

	packet.Add(SmartObjId);

	if (SmartObjId != -1) {
		WWASSERT(p_smart_go != NULL);

		p_smart_go->Export_Control_Cs(packet);
		p_smart_go->Export_State_Cs(packet);
	}
	
	SmartObjId = -1;//hack
}

//-----------------------------------------------------------------------------
void
CClientControl::Import_Frequent(BitStreamClass & packet)
{
	WWASSERT(CombatManager::I_Am_Server());

	packet.Get(SmartObjId);

	if (SmartObjId != -1) {
		SmartGameObj * p_smart_go = GameObjManager::Find_SmartGameObj(SmartObjId);
		//WWASSERT(p_smart_go != NULL);

		if (p_smart_go == NULL) {
			packet.Flush();
		} else {
			p_smart_go->Import_Control_Cs(packet);
			
			//
			// Note: org code had control owner test here, reapply if problems arise.
			//
			p_smart_go->Import_State_Cs(packet);
		}
	}
}

