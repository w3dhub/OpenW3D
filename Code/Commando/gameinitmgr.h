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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/gameinitmgr.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/12/01 3:15p                                              $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __GAME_INIT_MGR_H
#define __GAME_INIT_MGR_H

#include "renegadedialogmgr.h"


////////////////////////////////////////////////////////////////
//
//	GameInitMgrClass
//
////////////////////////////////////////////////////////////////
class GameInitMgrClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Level init
	//
	static void		Start_Game (const char *map_name, int teamChoice, unsigned int clanID);
	static void		End_Game (void);
	static void		Continue_Game (void);
	static void		Display_End_Game_Menu (void);
	static bool		Is_Game_In_Progress(void);
	//
	//	Client/server control
	//
	static void		Set_Is_Client_Required (bool onoff)	{ IsClientRequired = onoff; }
	static void		Set_Is_Server_Required (bool onoff)	{ IsServerRequired = onoff; }

	//
	//	Interface type init
	//
	//		LAN	= Local Area Network
	//		WOL	= Westwood Online
	//		SP		= Single Player
	//
	static void		Initialize_LAN (void);
	static void		Initialize_WOL (void);
	static void		Initialize_SP (void);
	static void		Initialize_Skirmish (void);

	static bool		Is_LAN_Initialized (void)			{ return Mode == MODE_LAN; }
	static bool		Is_WOL_Initialized (void)			{ return Mode == MODE_WOL; }
	static bool		Is_SP_Initialized (void)			{ return Mode == MODE_SP; }
	static bool		Is_Skirmish_Initialized (void)	{ return Mode == MODE_SKIRMISH; }

	static void		Shutdown (void);
	static void		Shutdown_LAN (void);
	static void		Shutdown_WOL (void);
	static void		Shutdown_SP (void);
	static void		Shutdown_Skirmish (void);

	static void		End_Client_Server (void);

	//
	//	Thinking support (for safely triggering game ends)
	//
	static void		Think (void);
	static void		Set_Needs_Game_Exit			(bool onoff)	{ NeedsGameExit = onoff; }
	static void		Set_Needs_Game_Exit_All		(bool onoff)	{ NeedsGameExitAll = onoff; }

	//
	//	WOL specific
	//
	static void		Set_WOL_Return_Dialog (RenegadeDialogMgrClass::LOCATION location)	{ WOLReturnDialog = location; }

private:

	////////////////////////////////////////////////////////////////
	//	Private constants
	////////////////////////////////////////////////////////////////
	enum
	{
		MODE_UNKNOWN	= 0,
		MODE_SP,
		MODE_SKIRMISH,
		MODE_LAN,
		MODE_WOL
	};

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	static void		Start_Client_Server (void);
	static void		Transmit_Player_Data (int teamChoice, unsigned int clanID);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	static bool		IsClientRequired;
	static bool		IsServerRequired;
	static bool		RestoreSFX;
	static bool		RestoreMusic;
	static int		Mode;
	static int		WOLReturnDialog;

	static bool		NeedsGameExit;
	static bool		NeedsGameExitAll;
};


#endif //__GAME_INIT_MGR_H