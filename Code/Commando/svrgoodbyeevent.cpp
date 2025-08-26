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
 *                     $Archive:: /Commando/Code/Commando/svrgoodbyeevent.cpp               $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 2/14/02 3:10p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 19                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "svrgoodbyeevent.h"

#include <stdio.h>
#include <stdlib.h>

#include "networkobjectfactory.h"
#include "gamemode.h"
#include "cnetwork.h"
#include "translatedb.h"
#include "string_ids.h"
#include "textdisplay.h"
#include "WWAudio.h"
#include "devoptions.h"
//#include "helptext.h"
#include "DlgMessageBox.h"
#include "apppackettypes.h"
#include "string_ids.h"
#include "dlgcncwinscreen.h"
#include "gamespyadmin.h"
#include "DlgMPConnectionRefused.h"
#include "specialbuilds.h"
#include "gamespyadmin.h"

// 
// TSS2001 Problem - we have lost the unreliable multiblast effect for this message
//

DECLARE_NETWORKOBJECT_FACTORY(cSvrGoodbyeEvent, NETCLASSID_SVRGOODBYEEVENT);

//-----------------------------------------------------------------------------
cSvrGoodbyeEvent::cSvrGoodbyeEvent(void)
{
	IsQuickFullExitRequested = false;

	Set_App_Packet_Type(APPPACKETTYPE_SVRGOODBYEEVENT);
}

//-----------------------------------------------------------------------------
void
cSvrGoodbyeEvent::Init(bool flag)
{
	WWASSERT(cNetwork::I_Am_Server());

	IsQuickFullExitRequested = flag;

	Set_Object_Dirty_Bit(BIT_CREATION, true);

	/*
	if (cNetwork::I_Am_Client())
	{
		Act();
	}
	*/
}

//-----------------------------------------------------------------------------
void
cSvrGoodbyeEvent::Act(void)
{
	if (cNetwork::I_Am_Only_Client()) {

		cNetwork::PClientConnection->Destroy_Connection(0);	// destroy connection to server 
		
		if (cGameSpyAdmin::Get_Is_Launched_From_Gamespy()) {
			DlgMPConnectionRefused::DoDialog(TRANSLATION(IDS_MP_SERVER_SHUT_DOWN), true);
		} else {
			DlgMsgBox::DoDialog(TRANSLATION(IDS_MENU_SERVER_MESSAGE_TITLE), TRANSLATION(IDS_MP_SERVER_SHUT_DOWN)); 
		}

		//
		//	Close out the win screen dialog (if its up)
		//
		CNCWinScreenMenuClass::Close_Dialog ();

		WWAudioClass::Get_Instance()->Create_Instant_Sound("System_Message", Matrix3D(1));

      if (IsQuickFullExitRequested)
		{
			WWDEBUG_SAY(("Quick full exit instructed from server.\n"));
			cDevOptions::QuickFullExit.Set(true);
		} 
		else 
		{
			//TSS090401
			//
			// The client needs to quit back to the game list
			//
			if (!cGameSpyAdmin::Get_Is_Launched_From_Gamespy()) {
				extern bool g_client_quit;
				g_client_quit = true;
			}
		}
	}

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cSvrGoodbyeEvent::Export_Creation(BitStreamClass & packet)
{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Export_Creation(packet);

	packet.Add(IsQuickFullExitRequested);

	Set_Delete_Pending();
}

//-----------------------------------------------------------------------------
void
cSvrGoodbyeEvent::Import_Creation(BitStreamClass & packet)
{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Only_Client());

	packet.Get(IsQuickFullExitRequested);

	Act();
}




















		/*
		else if (cGameSpyAdmin::Get_Is_Launched_From_Gamespy()) 
		{
			extern void Stop_Main_Loop (int);
			Stop_Main_Loop(EXIT_SUCCESS);
		} 
		*/
