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
// Filename:     useroptions.h
// Author:       Tom Spencer-Smith
// Date:         Dec 1999
// Description:
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef USEROPTIONS_H
#define USEROPTIONS_H

#include "regbool.h"
#include "regint.h"
#include "regfloat.h"
#include "boolean.h"
#include "regstring.h"
#include "bandwidth.h"


//-----------------------------------------------------------------------------
//
// Various options that the player chooses
//
class cUserOptions
{
	public:
		enum ParseResult {
			SUCCESS,
			FAILURE,
			PRINT_HELP,
		};

		static ParseResult Parse_Command_Line(int argc, char *argv[]);
		static void Print_Command_Line_Help(bool error);

		static void Set_GameDir(const char *gamedir);

		static void Set_Server_INI_File(const char *ini_file);

		static void Set_Bandwidth_Type(BANDWIDTH_TYPE_ENUM bandwidth_type);
		static BANDWIDTH_TYPE_ENUM Get_Bandwidth_Type(void);
		static void Set_Bandwidth_Bps(int bandwidth_bbs);

		static void Reread(void);

		static cRegistryString GameDir;

		static cRegistryBool ShowNamesOnSoldier;
		static cRegistryBool SkipQuitConfirmDialog;
		static cRegistryBool SkipIngameQuitConfirmDialog;
		static cRegistryBool CameraLockedToTurret;
		static cRegistryBool PermitDiagLogging;

		static cRegistryInt Sku;

		static cRegistryInt BandwidthBps;
		static cRegistryInt BandwidthType;

		static cRegistryInt		GameSpyBandwidthType;
		static cRegistryInt		PreferredGameSpyNic;
		static cRegistryString	GameSpyNickname;
		static cRegistryInt		GameSpyGamePort;
		static cRegistryInt		GameSpyQueryPort;
		static cRegistryInt		SplashCount;
		static cRegistryBool	DoneClientBandwidthTest;

		static cRegistryInt PreferredLanNic;

		static cRegistryInt NetUpdateRate;
		static cRegistryFloat ClientHintFactor;
		static cRegistryFloat MaxFacingPenalty;
		static cRegistryFloat IrrelevancePenalty;

		static cRegistryInt ResultsLogNumber;

	private:
};

//-----------------------------------------------------------------------------
#endif // USEROPTIONS_H










/*
		static cRegistryInt	GameListFilterMaxPing;
		static cRegistryInt	GameListFilterMinPlayersPresent;
		static cRegistryInt	GameListFilterMaxPlayersPresent;
		static cRegistryInt	GameListFilterMaxPlayersPermitted;
		static cRegistryBool	GameListFilterShowPrivateGames;
		static cRegistryBool	GameListFilterShowOnlyDedicatedGames;
		static cRegistryBool	GameListFilterShowOnlyGamesIRankFor;
*/