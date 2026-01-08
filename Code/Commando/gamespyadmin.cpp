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

//
// Filename:     gamespyadmin.cpp
// Author:       Tom Spencer-Smith
// Date:         Jan 2002
// Description:  
//

#include "gamespyadmin.h"

#include "wwdebug.h"
#include "widestring.h"
#include "gameinitmgr.h"
#include "gamedata.h"
#include "cnetwork.h"
#include "DlgMPConnect.h"
#include "GameSpy_QnR.h"
#include <gamespy/ghttp/ghttp.h>
#include "useroptions.h"
#include "renegadedialogmgr.h"
#include "dialogtests.h"
#include "translatedb.h"
#include "string_ids.h"
#include "bandwidthcheck.h"
#include "gamespyauthmgr.h"
#include "specialbuilds.h"

//
// Class statics
//

bool					cGameSpyAdmin::DetectingBandwidth				= false;
bool					cGameSpyAdmin::IsUnderGamespyMenuing			= false;
bool					cGameSpyAdmin::IsLaunchFromGamespyRequested	= false;
bool					cGameSpyAdmin::IsLaunchedFromGamespy			= false;
bool					cGameSpyAdmin::IsServerGamespyListed			= false;
ULONG					cGameSpyAdmin::GameHostIp							= 0;
USHORT				cGameSpyAdmin::GameHostPort						= 0;
WideStringClass	cGameSpyAdmin::PasswordAttempt;

// It's 2:00am....see DoDialog below..
cGameSpyAdmin theGameSpy;

//----------------------------------------------------------------------------------
void 
cGameSpyAdmin::Think
(
	void
)
{
	WWASSERT(Is_Gamespy_Game());

	if (IsLaunchFromGamespyRequested && SplashIntroMenuDialogClass::Is_Complete ())
	{
		if (!DetectingBandwidth) {
			RefPtr<SerialWait> serverWait = SerialWait::Create();
			WWASSERT(serverWait.IsValid());

			DetectingBandwidth = true;
			RefPtr<WaitCondition> bandwidth_wait = BandwidthCheckerClass::Detect();

			if (cUserOptions::DoneClientBandwidthTest.Is_True()) {
				//DlgWOLWait::DoDialog(TRANSLATE (IDS_MENU_DETECTING_BANDWIDTH), U_CHAR("Skip"), bandwidth_wait, &theGameSpy);
				DlgWOLWait::DoDialog(TRANSLATE (IDS_MENU_DETECTING_BANDWIDTH), TRANSLATE (IDS_MP_SKIP), bandwidth_wait, &theGameSpy);
			} else {
				DlgWOLWait::DoDialog(TRANSLATE (IDS_MENU_DETECTING_BANDWIDTH), bandwidth_wait, &theGameSpy);
			}
		}
	}

	ghttpThink();


#ifndef MULTIPLAYERDEMO
	if (cNetwork::I_Am_Server()) 
	{
		cGameSpyAuthMgr::Think();
	}
#endif // MULTIPLAYERDEMO
}

//----------------------------------------------------------------------------------
void cGameSpyAdmin::HandleNotification(DlgWOLWaitEvent& event) {

	switch (event.Result()) {
		case WaitCondition::ConditionMet:
		{
			if (DetectingBandwidth) {
				DetectingBandwidth = false;
				cUserOptions::Set_Bandwidth_Type(BANDWIDTH_AUTO);
				cUserOptions::DoneClientBandwidthTest.Set(true);
				Join_Server();
			}
		}
		break;
			
		case WaitCondition::Waiting:
		{
			// Do nothing
		}
		break;
	
		case WaitCondition::UserCancel:
		{
			if (DetectingBandwidth) {
				if (cUserOptions::DoneClientBandwidthTest.Is_True()) {
					// Skip Bandwidth test...
					DetectingBandwidth = false;
					BandwidthCheckerClass::Force_Upstream_Bandwidth(cUserOptions::BandwidthBps.Get());
					cUserOptions::Set_Bandwidth_Type(BANDWIDTH_AUTO);
					Join_Server();
				} else { // This must be an Abort...
					DetectingBandwidth = false;
// FIXME Is Stop_Main_Loop() safe here?
					extern void Stop_Main_Loop (int);
					Stop_Main_Loop(EXIT_SUCCESS);
				}
			}
		}
		break;
			
		case WaitCondition::TimeOut:
		case WaitCondition::Error:
		{
			DetectingBandwidth = false;
// FIXME Is Stop_Main_Loop() safe here?
			extern void Stop_Main_Loop (int);
			Stop_Main_Loop(EXIT_SUCCESS);
		}
		break;

		default:
		DIE;
		break;
	}
}


//----------------------------------------------------------------------------------
void 
cGameSpyAdmin::Join_Server(void) {
	Connect_To_Game_Server();
	IsLaunchFromGamespyRequested = false;
	IsLaunchedFromGamespy = true;
}

//----------------------------------------------------------------------------------
void 
cGameSpyAdmin::Reset
(
	void
)
{
	IsUnderGamespyMenuing			= false;
	IsLaunchFromGamespyRequested	= false;
	IsLaunchedFromGamespy			= false;
	IsServerGamespyListed			= false;
	GameHostIp							= 0;
	GameHostPort						= 0;
	GameSpyQnR.Shutdown();
}

//----------------------------------------------------------------------------------
void 
cGameSpyAdmin::Connect_To_Game_Server
(
	void
)
{
	WWASSERT(GameHostIp > 0);
	WWASSERT(GameHostPort > 0);

	GameInitMgrClass::Initialize_LAN();

	WWASSERT(PTheGameData == NULL);
	PTheGameData = cGameData::Create_Game_Of_Type(cGameData::GAME_TYPE_CNC);
	WWASSERT(PTheGameData != NULL);
	PTheGameData->Set_Ip_Address(GameHostIp);
	PTheGameData->Set_Port(GameHostPort);

	cNetwork::Init_Client();

	//
	//	Display the "connecting" dialog
	//
	DlgMPConnect::DoDialog(-1, 0);
}

//----------------------------------------------------------------------------------
void
cGameSpyAdmin::Set_Game_Host_Ip
(
	ULONG ip
)
{
	WWASSERT(ip > 0);
	GameHostIp = ip;
}

//----------------------------------------------------------------------------------
void
cGameSpyAdmin::Set_Game_Host_Port
(	
	USHORT port
)
{
	WWASSERT(port > 0);
	GameHostPort = port;
}

//----------------------------------------------------------------------------------
bool
cGameSpyAdmin::Is_Gamespy_Game
(
	void
)
{
	return 
		IsUnderGamespyMenuing			||
		IsLaunchFromGamespyRequested	|| 
		IsLaunchedFromGamespy			|| 
		IsServerGamespyListed;
}

//----------------------------------------------------------------------------------
bool
cGameSpyAdmin::Is_Nickname_Collision
(
	WideStringClass & nickname
)
{
	WWASSERT(!nickname.Is_Empty());
	WWASSERT(cNetwork::I_Am_Server());

	bool collides = (cPlayerManager::Find_Player(nickname) != NULL);
	if (cNetwork::I_Am_Only_Server()) 
	{
		collides |= !nickname.Compare_No_Case(cNetInterface::Get_Nickname());
	}

	return collides;
}



