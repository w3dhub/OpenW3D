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
 *                     $Archive:: /Commando/Code/Commando/mpsettingsmgr.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/06/02 2:39p                                               $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_MP_SETTINGS_MGR_H
#define __DLG_MP_SETTINGS_MGR_H

#include "wwstring.h"
#include "vector.h"
#include "widestring.h"
#include <vector>
#include <map>

////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
//
//	MPSettingsMgrClass
//
////////////////////////////////////////////////////////////////
class MPSettingsMgrClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		OPTION_ALLOW_PAGES			= 1,
		OPTION_LANGUAGE_FILTER		= 2,
		OPTION_ALLOW_FIND				= 4,
		OPTION_DISPLAY_ASIAN			= 8,
		OPTION_DISPLAY_NONASIAN		= 16,
		OPTION_BUDDY_CHAT_ONLY		= 32,
		OPTION_CLAN_CHAT_ONLY			= 64,

		OPTION_DEFAULTS				= OPTION_ALLOW_PAGES | OPTION_ALLOW_FIND |
												OPTION_DISPLAY_ASIAN | OPTION_DISPLAY_NONASIAN,

		OPTION_DEFAULTS_LATIN = OPTION_ALLOW_PAGES|OPTION_ALLOW_FIND|OPTION_DISPLAY_NONASIAN,

	} OPTION;

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Save/load settings
	//
	static void				Load_Settings (void);

	//
	//	Login methods
	//
	static void				Set_Last_Login (const char *name);
	static const char *	Get_Last_Login (void);

	static void				Set_Auto_Login (const char *);
	static const char *	Get_Auto_Login (void);

	static void				Set_Auto_Password (const char *);
	static const char *	Get_Auto_Password (void);

#ifdef OBSOLETE
	//
	// Quickmatch preferences
	//
	static void		Set_QuickMatch_Mode_Preference (const char *mode, int preference);
	static int		Get_QuickMatch_Mode_Preference (const char *mode);
	static void		Save_QuickMatch_Mode_Preferences (void);
#endif

	//
	//	Sidebar help
	//
	static bool	Is_Sidebar_Help_Displayed (void)					{ return DisplaySidebarHelp; }
	static void	Set_Is_Sidebar_Help_Displayed (bool onoff);

	//
	//	Auto login prompt
	//
	static bool	Is_Auto_Login_Prompt_Enabled (void)		{ return IsAutoLoginPromptEnabled; }
	static void	Enable_Auto_Login_Prompt (bool onoff);

	//
	//	Message of the day support
	//
	static bool	Has_MOTD_Been_Viewed (void)	{ return HasMOTDBeenViewed; }
	static void Set_MOTD_Viewed (bool onoff)	{ HasMOTDBeenViewed = onoff; }

	//
	//	Options
	//
	static void	Set_Option_Flag (OPTION flag, bool onoff);
	static bool	Get_Option_Flag (OPTION flag)						{ return (OptionFlags & flag) == flag; }

	//
	//	Alternate skins unlocking
	//
	static bool	Are_Alternate_Skins_Unlocked (void);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////

	static StringClass					SelectedLogin;
	static StringClass					LastLogin;
	static StringClass					AutoLogin;
	static StringClass					AutoPassword;
	static bool								DisplaySidebarHelp;
	static bool								IsAutoLoginPromptEnabled;
	static bool								IsIGR;
	static bool								HasMOTDBeenViewed;
	static bool								AreSkinsUnlocked;
	static int								OptionFlags;

#ifdef OBSOLETE
	typedef std::map<const char*, int> GameModeMap;
	static GameModeMap _mModePrefs;
#endif
};

#endif //__DLG_MP_SETTINGS_MGR_H