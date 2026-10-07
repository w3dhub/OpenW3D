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
 *                     $Archive:: /Commando/Code/Commando/_globals.h                           $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/06/02 5:36p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "specialbuilds.h"

/*
#ifdef FREEDEDICATEDSERVER
#define APP_SUB_KEY "Software\\Westwood\\RenegadeFDS"
#else  //FREEDEDICATEDSERVER

#ifdef MULTIPLAYERDEMO
#define APP_SUB_KEY "Software\\Westwood\\RenegadeMPDemo"
#else // MULTIPLAYERDEMO
#define APP_SUB_KEY "Software\\Westwood\\Renegade"
#endif //MULTIPLAYERDEMO

#endif //FREEDEDICATEDSERVER
*/

#if	defined(FREEDEDICATEDSERVER)
#define APP_SUB_KEY "Software\\Westwood\\RenegadeFDS"
#elif defined(MULTIPLAYERDEMO)
#define APP_SUB_KEY "Software\\Westwood\\RenegadeMPDemo"
#elif defined(BETACLIENT)
#define APP_SUB_KEY "Software\\Westwood\\RenegadeBeta"
#elif defined(BETASERVER)
#define APP_SUB_KEY "Software\\Westwood\\RenegadeBeta"
#else
#define APP_SUB_KEY "Software\\Westwood\\Renegade"
#endif


extern char *Build_Registry_Location_String(const char *base, const char *modifier, const char *sub);

#define	APPLICATION_SUB_KEY_NAME						"Application"

#define	APPLICATION_SUB_KEY_NAME_RENDER					"RenderDevice"
#define	APPLICATION_SUB_KEY_NAME_OPTIONS				"Options"
#define	APPLICATION_SUB_KEY_NAME_DEBUG					"Debug"
#define	APPLICATION_SUB_KEY_NAME_SYSTEM_SETTINGS		"System"
#define	APPLICATION_SUB_KEY_NAME_CONTROLS				"Controls"
#define	APPLICATION_SUB_KEY_NAME_SOUND					"Sound"
#define	APPLICATION_SUB_KEY_NAME_MOVIES					"Movies"
#define	APPLICATION_SUB_KEY_NAME_WOLSETTINGS			"WOLSettings"
#define	APPLICATION_SUB_KEY_NAME_MISSION_RANKS			"Ranks"
#define	APPLICATION_SUB_KEY_NAME_INPUT					"Input"
#define	APPLICATION_SUB_KEY_NAME_GAMESPY				"GameSpy"
#define	APPLICATION_SUB_KEY_NAME_WOLSETTINGS			"WOLSettings"
#define	APPLICATION_SUB_KEY_NAME_URL					"WOLSettings/URL"
#define	APPLICATION_SUB_KEY_NAME_LOGINS					"WOLSettings/Logins"
#define	APPLICATION_SUB_KEY_NAME_QUICKMATCH				"WOLSettings/QuickMatch"
#define	APPLICATION_SUB_KEY_NAME_IGNORE_LIST			"WOLSettings/Ignore List"
#define	APPLICATION_SUB_KEY_NAME_SERVER_LIST			"WOLSettings/Servers"
#define	APPLICATION_SUB_KEY_NAME_SKIN_LIST				"MP Settings/Skins"

#define	APPLICATION_SUB_KEY_NAME_NETOPTIONS				"Networking/Options"
#define	APPLICATION_SUB_KEY_NAME_NETDEBUG				"Networking/Debug"
#define	APPLICATION_SUB_KEY_NAME_NET_FIREWALL			"Networking/Firewall"
#define	APPLICATION_SUB_KEY_NAME_NET_SLAVE				"Networking/Slave"
#define	APPLICATION_SUB_KEY_NAME_NET_SERVER_CONTROL		"Networking/ServerControl"

#define	COMBAT_SUB_KEY_NAME_DEBUG						APPLICATION_SUB_KEY_NAME_DEBUG

#define	APPLICATION_SUB_KEY_NAME_BANDTEST				"Bandtest"

#define RENEGADE_BASE_SKU								3072
#define	RENEGADE_FDS_SKU								12288
#define	RENEGADE_DEMO_SKU								13056


#endif
