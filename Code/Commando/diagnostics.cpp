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
// Filename:     diagnostics.cpp
// Project:      Network.lib, for Commando
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:
//

#include "diagnostics.h"

#include <stdio.h>

#include "wwdebug.h"
#include "assets.h"
#include "font3d.h"
#include "render2d.h"

//
// Add any includes for exposed interfaces that you are diagnosing
//

#include "devoptions.h"
#include "cnetwork.h"
#include "gamedata.h"
#include "networkobjectmgr.h"
#include "serverfps.h"
#include "sbbomanager.h"
#include "singlepl.h"
#include "gameobjmanager.h"
#include "gamemode.h"
#include "humanphys.h"
#include "playermanager.h"
#include "useroptions.h"
#include "packetmgr.h"
#include "apppacketstats.h"
#include "connect.h"
#include "wwprofile.h"
#include "vehicle.h"
#include "csdamageevent.h"
#include "specialbuilds.h"
#include "ConsoleMode.h"
#include "gametype.h"
#include <algorithm>

static int RendererFps;
static int RendererSFps;
static int RendererPing;
static int RendererBandwidthBps;
static bool RendererGodStatus;
static bool RendererVipStatus;

//
// Class statics
//
Render2DTextClass *		cDiagnostics::PRenderer		= nullptr;
Font3DInstanceClass *	cDiagnostics::PFont			= nullptr;
float							cDiagnostics::DiagnosticX	= 0;
float							cDiagnostics::DiagnosticY	= 0;

//-----------------------------------------------------------------------------
void cDiagnostics::Init(void)
{
	if (!ConsoleBox.Is_Exclusive()) {
		WWASSERT(WW3DAssetManager::Get_Instance() != nullptr);
   	PFont = WW3DAssetManager::Get_Instance()->Get_Font3DInstance("FONT6x8.TGA");
   	WWASSERT(PFont != nullptr);
		SET_REF_OWNER(PFont);
		PFont->Set_Mono_Spaced();

		PRenderer = new Render2DTextClass(PFont);
		WWASSERT(PRenderer != nullptr);

		RectClass rect = Render2DClass::Get_Screen_Resolution();
		PRenderer->Set_Coordinate_Range(rect);

		DiagnosticX = 10;
		//DiagnosticY = 10;

		RendererFps=0;
		RendererSFps=0;
		RendererPing = 0;
		RendererBandwidthBps = 0;
		RendererGodStatus=false;
		RendererVipStatus=false;
	}

}

//-----------------------------------------------------------------------------
void cDiagnostics::Close(void)
{
	if (PFont != nullptr) {
		PFont->Release_Ref();
		PFont = nullptr;
	}

	if (PRenderer != nullptr) {
		delete PRenderer;
		PRenderer = nullptr;
	}
}

//-----------------------------------------------------------------------------
void cDiagnostics::Show_Object_Tally(void)
{
	cAppPacketStats::Update_Object_Tally();

	Add_Diagnostic("");
	Add_Diagnostic("Object stats (reset with APTR command):");
	Add_Diagnostic("%s", cAppPacketStats::Get_Heading().Peek_Buffer());

	for (BYTE i = 0; i < APPPACKETTYPE_COUNT; i++)
	{
		Add_Diagnostic("%s", cAppPacketStats::Get_Description(i).Peek_Buffer());
	}
}

//-----------------------------------------------------------------------------
void cDiagnostics::Add_Diagnostic(const char *format, ...)
{
	if (PRenderer) {
		va_list va;
		char buffer[1024];

		va_start(va, format);
		::vsprintf(buffer, format, va);

		WWASSERT(PRenderer != nullptr);
		PRenderer->Set_Location(Vector2(DiagnosticX, DiagnosticY));
		PRenderer->Draw_Text(buffer);

		WWASSERT(PRenderer->Peek_Font() != nullptr);
		DiagnosticY += (int)(PRenderer->Peek_Font()->Char_Height() * 1.2);
	}
}

//-----------------------------------------------------------------------------
void cDiagnostics::Render(void)
{
	if (PFont == nullptr || PRenderer == nullptr) {
		return;
	}

#ifdef BETACLIENT
	PRenderer->Reset();
	RectClass beta_rect = Render2DClass::Get_Screen_Resolution();
	PRenderer->Set_Location(Vector2(beta_rect.Left + 5, beta_rect.Bottom - 10));
	PRenderer->Draw_Text("BETACLIENT");
	PRenderer->Render();
#endif // BETACLIENT

#ifdef FREEDEDICATEDSERVER
	PRenderer->Reset();
	RectClass fds_rect = Render2DClass::Get_Screen_Resolution();
	PRenderer->Set_Location(Vector2(fds_rect.Left + 5, fds_rect.Bottom - 10));
	PRenderer->Draw_Text("FREEDEDICATEDSERVER");
	PRenderer->Render();
#endif // FREEDEDICATEDSERVER

#ifdef MULTIPLAYERDEMO
	PRenderer->Reset();
	RectClass mpd_rect = Render2DClass::Get_Screen_Resolution();
	PRenderer->Set_Location(Vector2(mpd_rect.Left + 5, mpd_rect.Bottom - 10));
	PRenderer->Draw_Text("MULTIPLAYER DEMO");
	PRenderer->Render();
#endif // MULTIPLAYERDEMO


	bool changed=false;

	if (cDevOptions::ShowFps.Is_True()) {
		int fps=cNetwork::Get_Fps();
		if (fps!=RendererFps) {
			RendererFps=fps;
			changed=true;
		}

		if (cNetwork::I_Am_Only_Client())
		{
			fps=cServerFps::Get_Instance()->Get_Fps();
			if (fps!=RendererSFps) {
				RendererSFps=fps;
				changed=true;
			}

			if (cNetwork::PClientConnection) {
				cRemoteHost *server = cNetwork::PClientConnection->Get_Remote_Host(0);
				if (server) {
					int ping = server->Get_Average_Internal_Pingtime_Ms();
					if (ping && ping != RendererPing) {
						RendererPing = ping;
						changed = true;
					}
					int bps = PacketManager.Get_Compressed_Bandwidth_In(&server->Get_Address());
					if (bps && bps != RendererBandwidthBps) {
						RendererBandwidthBps = bps;
						changed = true;
					}
				}
			}
		} else {
			int bps = PacketManager.Get_Total_Compressed_Bandwidth_Out();
			if (bps && bps != RendererBandwidthBps) {
				RendererBandwidthBps = bps;
				changed = true;
			}
		}
	}
	// If we displayed fps last frame, turn it off now
	else {
		if (RendererFps) {
			changed=true;
		}
		RendererFps=0;
	}
#ifdef WWDEBUG

	bool god_status = cDevOptions::ShowGodStatus.Is_True() && cNetwork::I_Am_Client();
	if (god_status!=RendererGodStatus) {
		RendererGodStatus=god_status;
		changed=true;
	}

	bool vip_status = cNetwork::I_Am_Client();
	if (vip_status != RendererVipStatus) {
		RendererVipStatus = vip_status;
		changed = true;
	}

	// If diagnostics are displayed, changes happen most likely every frame
	if (cDevOptions::ShowDiagnostics.Is_True() || cDevOptions::ShowObjectTally.Is_True()) {
		changed=true;
	}
#endif //WWDEBUG

	if (cDevOptions::ShowFps.Is_True()) {
		// Stop the flicker
		changed = true;
	}
	// Render only if changed!
	if (!changed) {
		PRenderer->Render();
		return;
	}

	PRenderer->Reset();

	DiagnosticY = 75;

	if (cDevOptions::ShowFps.Is_True()) {
		StringClass fps_text;
		fps_text.Format("FPS = %3d", RendererFps);

		if (cNetwork::I_Am_Only_Client())
		{
			StringClass server_fps_text;
			server_fps_text.Format(", SFPS = %3d", RendererSFps);
			fps_text += server_fps_text;

			if (cNetwork::PClientConnection && cNetwork::PClientConnection->Get_Remote_Host(0)) {
				StringClass ping_time;
				ping_time.Format(", PING = %4d", std::min(9999, RendererPing));
				fps_text += ping_time;
			}

		}

		if (!IS_SOLOPLAY) {
			StringClass bps_text;
			bps_text.Format(", KBPS = %4d", RendererBandwidthBps / 1024);
			fps_text += bps_text;
		}

		float width = PFont->String_Width(fps_text);
		RectClass rect = Render2DClass::Get_Screen_Resolution();
		PRenderer->Set_Location(Vector2(rect.Right - width - 5, rect.Top + 2));
		PRenderer->Draw_Text(fps_text);
	}

#ifdef WWDEBUG

	//
	// Show god status
	//
	if (god_status) {
		cPlayer * p_player = cNetwork::Get_My_Player_Object();
		if (p_player != nullptr && p_player->Invulnerable.Is_True()) {

			RectClass rect = Render2DClass::Get_Screen_Resolution();
			PRenderer->Set_Location(Vector2(rect.Left + 10, rect.Bottom - 20));
			PRenderer->Draw_Text("GOD");
		}
	}

	//
	// Show vip status
	//
	if (vip_status) {
		cPlayer * p_player = cNetwork::Get_My_Player_Object();
		if (p_player != nullptr && p_player->Get_Damage_Scale_Factor() < 100) {

			RectClass rect = Render2DClass::Get_Screen_Resolution();
			PRenderer->Set_Location(Vector2(rect.Left + 10, rect.Bottom - 30));
			PRenderer->Draw_Text("VIP");
		}
	}

   /*
	if (cNetwork::I_Am_Server() && cDevOptions::ShowBandwidthBudgetOut.Is_True()) {
		Add_Diagnostic("BBO: %d bps\n", cNetwork::PServerConnection->Get_Bandwidth_Budget_Out());
	}
	*/

	if (cDevOptions::ShowObjectTally.Is_True()) {
		Show_Object_Tally();
	}

	if (cDevOptions::ShowDiagnostics.Is_True()) {

		if (cNetwork::I_Am_Server()) {
			Add_Diagnostic("BBO (server):       %u bps\n", cNetwork::PServerConnection->Get_Bandwidth_Budget_Out());
		}

		if (cNetwork::I_Am_Client()) {
			Add_Diagnostic("BBO (client):       %u bps\n", cNetwork::PClientConnection->Get_Bandwidth_Budget_Out());
			Add_Diagnostic("CSC Last Ping:      %u ms", CombatManager::Get_Last_Round_Trip_Ping_Ms());
			Add_Diagnostic("CSC Avg. Ping:      %u ms", CombatManager::Get_Avg_Round_Trip_Ping_Ms());
		}

		int low		= 0;
		int high		= 0;
		int current	= 0;
		cConnection::Get_Latency(low, high, current);
		Add_Diagnostic("latency sim:        (%d, %d) : %d", low, high, current);

		cConnection::Get_Latency(low, high, current);
		Add_Diagnostic("#netobjects:        %d", NetworkObjectMgrClass::Get_Object_Count());
		Add_Diagnostic("#players:           %d", cPlayerManager::Count());

		if (cNetwork::I_Am_Server()) {
			Add_Diagnostic("NetToCombatRatio:   %-5.2f", cSbboManager::Get_Net_To_Combat_Ratio());
			Add_Diagnostic("ThinkCount:         %d", cNetwork::Get_Think_Count());
		}

		Add_Diagnostic("I_Am_Client:        %d",		cNetwork::I_Am_Client());
		Add_Diagnostic("I_Am_Server:        %d",		cNetwork::I_Am_Server());
		Add_Diagnostic("NetUpdateRate:      %d",		cUserOptions::NetUpdateRate.Get());
		Add_Diagnostic("ClientHintFactor:   %5.2f",	cUserOptions::ClientHintFactor.Get());
		Add_Diagnostic("MaxFacingPenalty:   %5.2f",	cUserOptions::MaxFacingPenalty.Get());

		if (cNetwork::I_Am_Client()) {
			SoldierGameObj * p_my_soldier = GameObjManager::Find_Soldier_Of_Client_ID(cNetwork::Get_My_Id());
			if (p_my_soldier != nullptr) {

				int tally = p_my_soldier->Tally_Vis_Visible_Soldiers();
				if (tally >= 0) {
					Add_Diagnostic("Soldiers VV:        %d", tally);
				} else {
					Add_Diagnostic("Soldiers VV:        NO VIS HERE");
				}

          //Add_Diagnostic("--------------------");
			 //Add_Diagnostic("In elevator:        %d", p_my_soldier->Is_In_Elevator());
			}
		}

		if (PTheGameData != nullptr) {
			Add_Diagnostic("ip addy:            %s:%u",
				cNetUtil::Address_To_String(PTheGameData->Get_Ip_Address()),
				PTheGameData->Get_Port());

			Add_Diagnostic("mapname:            %s", PTheGameData->Get_Map_Name().Peek_Buffer());
			Add_Diagnostic("HostedGameNumber:   %d", cGameData::Get_Hosted_Game_Number());
		}

		Add_Diagnostic("PacketManager:");
		Add_Diagnostic("  FlushFrequency:   %d", PacketManager.Get_Flush_Frequency());
		Add_Diagnostic("  AllowDeltas:      %d", PacketManager.Get_Allow_Deltas());
		Add_Diagnostic("  AllowCombos:      %d", PacketManager.Get_Allow_Combos());

		//Add_Diagnostic("Frames:             %d", WWProfileManager::Get_Frame_Count_Since_Reset());

		//
		// Let's track how many vehicles are in the world, and how many of those
		// have drivers.
		//
		int vehicle_count = 0;
		int vehicle_driven_count = 0;
		SLNode<BaseGameObj> * objnode;
		for (objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
			WWASSERT(objnode->Data() != nullptr);
			PhysicalGameObj * p_phys_obj = objnode->Data()->As_PhysicalGameObj();
			if (p_phys_obj != nullptr && p_phys_obj->As_VehicleGameObj() != nullptr) {
				vehicle_count++;
				if (p_phys_obj->As_VehicleGameObj()->Get_Driver() != nullptr) {
					vehicle_driven_count++;
				}
			}
		}
		Add_Diagnostic("Vehicles:           %d (%d with driver)", vehicle_count, vehicle_driven_count);

		//Add_Diagnostic("AreClientsTrusted:    %d", cCsDamageEvent::Get_Are_Clients_Trusted());
		//Add_Diagnostic("DriverIsAlwaysGunner: %d", VehicleGameObj::Get_Driver_Is_Always_Gunner());
		//Add_Diagnostic("CameraLockedToTurret: %d", VehicleGameObj::Get_Camera_Locked_To_Turret());

		if (PTheGameData != nullptr) {
			Add_Diagnostic("SpawnWeapons:       %d", PTheGameData->SpawnWeapons.Get());

			/*
			StringClass mvp_name;
			PTheGameData->Get_Mvp_Name().Convert_To(mvp_name);
			Add_Diagnostic("MVP:                %s", mvp_name.Peek_Buffer());
			*/

			Add_Diagnostic("Win Type:           %d", (int) PTheGameData->Get_Win_Type());
			Add_Diagnostic("DurationS:          %u", (int) PTheGameData->Get_Game_Duration_S());
		}
	}

#endif // WWDEBUG
	PRenderer->Render();

}
