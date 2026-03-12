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
// Filename:     devoptions.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1999
// Description:
//

#include "devoptions.h" // I WANNA BE FIRST!

#include "_globals.h"
#include "wwdebug.h"
#include "player.h"
#include "registry.h"

//
// Class statics
//
#ifdef WWDEBUG
   cRegistryBool cDevOptions::ShowGodStatus(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowGodStatus",				true);
   cRegistryBool cDevOptions::ShowSoldierData(        APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowSoldierData",          false);
   cRegistryBool cDevOptions::ShowVehicleData(        APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowVehicleData",          false);
   cRegistryBool cDevOptions::ShowDoorData(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowDoorData",					false);
   cRegistryBool cDevOptions::ShowElevatorData(       APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowElevatorData",         false);
   cRegistryBool cDevOptions::ShowDSAPOData(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowDSAPOData",				false);
   cRegistryBool cDevOptions::ShowPowerupData(        APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowPowerupData",          false);
   cRegistryBool cDevOptions::ShowBuildingData(       APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowBuildingData",         false);
   cRegistryBool cDevOptions::ShowSpawnerData(        APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowSpawnerData",          false);
   cRegistryBool cDevOptions::ShowImportStates(       APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowImportStates",         false);
   cRegistryBool cDevOptions::ShowImportStatesSV(		APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowImportStatesSV",			false);
   cRegistryBool cDevOptions::ShowServerRhostData(    APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowServerRhostData",      false);
   cRegistryBool cDevOptions::ShowClientRhostData(    APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowClientRhostData",      false);
   //cRegistryBool cDevOptions::ShowPacketGraphs(       APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowPacketGraphs",         false);

	cRegistryBool cDevOptions::PacketsSentServer(					APPLICATION_SUB_KEY_NAME_NETDEBUG, "PacketsSentServer",					false);
	cRegistryBool cDevOptions::PacketsSentClient(					APPLICATION_SUB_KEY_NAME_NETDEBUG, "PacketsSentClient",					false);
	cRegistryBool cDevOptions::PacketsRecdServer(					APPLICATION_SUB_KEY_NAME_NETDEBUG, "PacketsRecdServer",					false);
	cRegistryBool cDevOptions::PacketsRecdClient(					APPLICATION_SUB_KEY_NAME_NETDEBUG, "PacketsRecdClient",					false);
	cRegistryBool cDevOptions::AvgSizePacketsSentServer(			APPLICATION_SUB_KEY_NAME_NETDEBUG, "AvgSizePacketsSentServer",       false);
	cRegistryBool cDevOptions::AvgSizePacketsSentClient(			APPLICATION_SUB_KEY_NAME_NETDEBUG, "AvgSizePacketsSentClient",       false);
	cRegistryBool cDevOptions::AvgSizePacketsRecdServer(			APPLICATION_SUB_KEY_NAME_NETDEBUG, "AvgSizePacketsRecdServer",       false);
	cRegistryBool cDevOptions::AvgSizePacketsRecdClient(			APPLICATION_SUB_KEY_NAME_NETDEBUG, "AvgSizePacketsRecdClient",       false);
	cRegistryBool cDevOptions::BytesSentServer(						APPLICATION_SUB_KEY_NAME_NETDEBUG, "BytesSentServer",						false);
	cRegistryBool cDevOptions::BytesSentClient(						APPLICATION_SUB_KEY_NAME_NETDEBUG, "BytesSentClient",						false);
	cRegistryBool cDevOptions::BytesRecdServer(						APPLICATION_SUB_KEY_NAME_NETDEBUG, "BytesRecdServer",						false);
	cRegistryBool cDevOptions::BytesRecdClient(						APPLICATION_SUB_KEY_NAME_NETDEBUG, "BytesRecdClient",						false);

	cRegistryBool cDevOptions::WwnetPacketsSentServer(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetPacketsSentServer",         false);
	cRegistryBool cDevOptions::WwnetPacketsSentClient(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetPacketsSentClient",         false);
	cRegistryBool cDevOptions::WwnetPacketsRecdServer(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetPacketsRecdServer",         false);
	cRegistryBool cDevOptions::WwnetPacketsRecdClient(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetPacketsRecdClient",         false);
	cRegistryBool cDevOptions::WwnetAvgSizePacketsSentServer(	APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetAvgSizePacketsSentServer",	false);
	cRegistryBool cDevOptions::WwnetAvgSizePacketsSentClient(   APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetAvgSizePacketsSentClient",  false);
	cRegistryBool cDevOptions::WwnetAvgSizePacketsRecdServer(   APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetAvgSizePacketsRecdServer",  false);
	cRegistryBool cDevOptions::WwnetAvgSizePacketsRecdClient(   APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetAvgSizePacketsRecdClient",  false);
	cRegistryBool cDevOptions::WwnetBytesSentServer(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetBytesSentServer",				 false);
	cRegistryBool cDevOptions::WwnetBytesSentClient(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetBytesSentClient",				false);
	cRegistryBool cDevOptions::WwnetBytesRecdServer(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetBytesRecdServer",				false);
	cRegistryBool cDevOptions::WwnetBytesRecdClient(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "WwnetBytesRecdClient",				false);

   cRegistryBool cDevOptions::ShowPriorities(			APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowPriorities",				false);
   cRegistryBool cDevOptions::ShowBandwidth(          APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowBandwidth",            false);
   cRegistryBool cDevOptions::ShowLatency(            APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowLatency",              false);
   cRegistryBool cDevOptions::ShowLastContact(        APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowLastContact",          false);
   cRegistryBool cDevOptions::ShowListSizes(          APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowListSizes",            false);
   cRegistryBool cDevOptions::ShowListTimes(          APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowListTimes",            false);
   cRegistryBool cDevOptions::ShowListPacketSizes(    APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowListPacketSizes",      false);
   //cRegistryBool cDevOptions::ShowBandwidthBudgetOut( APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowBandwidthBudgetOut",   false);
   cRegistryBool cDevOptions::ShowWatchList(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowWatchList",				false);
   cRegistryBool cDevOptions::ShowWolLocation(        APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowWolLocation",          false);
   cRegistryBool cDevOptions::ShowDiagnostics(			APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowDiagnostics",				false);
   cRegistryBool cDevOptions::ShowMenuStack(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowMenuStack",				false);
   cRegistryBool cDevOptions::ShowIpAddresses(			APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowIpAddresses",				false);
   cRegistryBool cDevOptions::ShowClientFps(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowClientFps",				false);
   cRegistryBool cDevOptions::ShowId(						APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowId",							false);
	cRegistryBool cDevOptions::ShowPing(					APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowPing",                 false);
   cRegistryBool cDevOptions::ShowObjectTally(			APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowObjectTally",				false);
   cRegistryBool cDevOptions::ShowInactivePlayers(		APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowInactivePlayers",		false);
	cRegistryBool cDevOptions::ShowGameSpyAuthState(	APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowGameSpyAuthState",		false);

	cRegistryInt cDevOptions::DesiredFrameSleepMs(				APPLICATION_SUB_KEY_NAME_NETDEBUG, "DesiredFrameSleepMs",				0);
	cRegistryInt cDevOptions::SimulatedPacketLossPc(			APPLICATION_SUB_KEY_NAME_NETDEBUG, "SimulatedPacketLossPc",				0);
	cRegistryInt cDevOptions::SimulatedPacketDuplicationPc(	APPLICATION_SUB_KEY_NAME_NETDEBUG, "SimulatedPacketDuplicationPc",	0);
	cRegistryInt cDevOptions::SimulatedLatencyRangeMsLower(	APPLICATION_SUB_KEY_NAME_NETDEBUG, "SimulatedLatencyRangeMsLower",	0);
	cRegistryInt cDevOptions::SimulatedLatencyRangeMsUpper(	APPLICATION_SUB_KEY_NAME_NETDEBUG, "SimulatedLatencyRangeMsUpper",	0);
	cRegistryInt cDevOptions::SpamCount(							APPLICATION_SUB_KEY_NAME_NETDEBUG, "SpamCount",								0);

	cRegistryBool cDevOptions::SoundEffectOnAssert(				APPLICATION_SUB_KEY_NAME_DEBUG,	"SoundEffectOnAssert",				false);
	cRegistryBool cDevOptions::DisplayLogfileOnAssert(			APPLICATION_SUB_KEY_NAME_DEBUG,	"DisplayLogfileOnAssert",			false);
	cRegistryBool cDevOptions::BreakToDebuggerOnAssert(		APPLICATION_SUB_KEY_NAME_DEBUG,	"BreakToDebuggerOnAssert",			true);
	cRegistryBool cDevOptions::ShutdownInputOnAssert(			APPLICATION_SUB_KEY_NAME_DEBUG,	"ShutdownInputOnAssert",			true);
	cRegistryBool cDevOptions::PreloadAssets(						APPLICATION_SUB_KEY_NAME_DEBUG,	"PreloadAssets",						true);
	cRegistryBool cDevOptions::FilterLevelFiles(					APPLICATION_SUB_KEY_NAME_DEBUG,	"FilterLevelFiles",					true);
	cRegistryBool cDevOptions::IBelieveInGod(						APPLICATION_SUB_KEY_NAME_DEBUG,	"IBelieveInGod",						false);
	cRegistryBool cDevOptions::LogDataSafe(						APPLICATION_SUB_KEY_NAME_DEBUG,	"LogDataSafe",							true);
	cRegistryBool cDevOptions::EnableExceptionHandler(			APPLICATION_SUB_KEY_NAME_DEBUG,	"ExceptionHandler",					true);
	cRegistryBool cDevOptions::ShowThumbnailPreInitDialog(	APPLICATION_SUB_KEY_NAME_DEBUG,	"ShowThumbnailPreInitDialog",		true);
	cRegistryBool cDevOptions::CrtDbgEnabled(						APPLICATION_SUB_KEY_NAME_DEBUG,	"CrtDbgEnabled",						true);
	cRegistryBool cDevOptions::PacketOptimizationsEnabled(	APPLICATION_SUB_KEY_NAME_DEBUG,	"PacketOptimizationsEnabled",		true);
	cRegistryBool cDevOptions::ShowMoney(							APPLICATION_SUB_KEY_NAME_DEBUG,	"ShowMoney",							false);
	cRegistryBool cDevOptions::ExtraNetDebug(						APPLICATION_SUB_KEY_NAME_DEBUG,	"NetDebugLog",							false);
	cRegistryBool cDevOptions::ExtraModemBandwidthThrottling(APPLICATION_SUB_KEY_NAME_DEBUG,	"ExtraModemBWThrottling",			true);

   cBoolean cDevOptions::GoToMainMenu(false);

#endif // WWDEBUG

   cBoolean cDevOptions::QuickFullExit(false);

   cRegistryBool cDevOptions::ExitThreadOnAssert(				APPLICATION_SUB_KEY_NAME_DEBUG,	"ExitThreadOnAssert",				true);
   cRegistryBool cDevOptions::CompareExeVersionOnNetwork(	APPLICATION_SUB_KEY_NAME_DEBUG,	"CompareExeVersionOnNetwork",		true);

	cRegistryBool cDevOptions::UseNewTCADO(						APPLICATION_SUB_KEY_NAME_DEBUG,	"NewTCADO",								true);
   cRegistryBool cDevOptions::ShowFps(								APPLICATION_SUB_KEY_NAME_NETDEBUG, "ShowFps",							false);








	//cRegistryBool cDevOptions::DoThumbnailPreInit(				APPLICATION_SUB_KEY_NAME_DEBUG,	"DoThumbnailPreInit",				true);
