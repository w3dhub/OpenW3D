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
 *                     $Archive:: /Commando/Code/Combat/apppackettypes.h         $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 2/21/02 3:01p                                               $*
 *                                                                                             *
 *                    $Revision:: 21                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __APPPACKETTYPES_H__
#define __APPPACKETTYPES_H__

//-----------------------------------------------------------------------------
//
// Leaf node net object types
// N.B. If you add to this list you must also add in cAppPacketStats::Interpret_Type.
//
enum
{
	//
	// S->C
	//
	APPPACKETTYPE_UNKNOWN,
	APPPACKETTYPE_SIMPLE,
	APPPACKETTYPE_SOLDIER,
	APPPACKETTYPE_VEHICLE,
	APPPACKETTYPE_TURRET,
	APPPACKETTYPE_BUILDING,
	APPPACKETTYPE_PLAYER,
	APPPACKETTYPE_TEAM,
	APPPACKETTYPE_GAMEOPTIONSEVENT,
	APPPACKETTYPE_PLAYERKILLEVENT,
	APPPACKETTYPE_PURCHASERESPONSEEVENT,
	APPPACKETTYPE_SCTEXTOBJ,
	APPPACKETTYPE_SVRGOODBYEEVENT,
	APPPACKETTYPE_WINEVENT,
	APPPACKETTYPE_POWERUP,
	APPPACKETTYPE_STATIC,
	APPPACKETTYPE_DOOR,
	APPPACKETTYPE_ELEVATOR,
	APPPACKETTYPE_DSAPO,
	APPPACKETTYPE_SERVERFPS,
	APPPACKETTYPE_CONSOLECOMMANDEVENT,
	APPPACKETTYPE_RESETWINSEVENT,
	APPPACKETTYPE_EVICTIONEVENT,
	APPPACKETTYPE_NETWEATHER,
	APPPACKETTYPE_GAMEDATAUPDATEEVENT,
	APPPACKETTYPE_SCPINGRESPONSEEVENT,
	APPPACKETTYPE_BASECONTROLLER,
	APPPACKETTYPE_CINEMATIC,
	APPPACKETTYPE_C4,
	APPPACKETTYPE_BEACON,
	APPPACKETTYPE_SCEXPLOSIONEVENT,
	APPPACKETTYPE_SCOBELISKEVENT,
	APPPACKETTYPE_SCANNOUNCEMENT,
	APPPACKETTYPE_NETBACKGROUND,
	APPPACKETTYPE_GAMESPYSCCHALLENGEEVENT,

	//
	// C->S
	//
	APPPACKETTYPE_CLIENTCONTROL,
	APPPACKETTYPE_CSTEXTOBJ,
	APPPACKETTYPE_SUICIDEEVENT,
	APPPACKETTYPE_CHANGETEAMEVENT,
	APPPACKETTYPE_MONEYEVENT,
	APPPACKETTYPE_WARPEVENT,
	APPPACKETTYPE_PURCHASEREQUESTEVENT,
	APPPACKETTYPE_CLIENTGOODBYEEVENT,
	APPPACKETTYPE_BIOEVENT,
	APPPACKETTYPE_LOADINGEVENT,
	APPPACKETTYPE_GODMODEEVENT,
	APPPACKETTYPE_VIPMODEEVENT,
	APPPACKETTYPE_SCOREEVENT,
	APPPACKETTYPE_CLIENTBBOEVENT,
	APPPACKETTYPE_CLIENTFPS,
	APPPACKETTYPE_CSPINGREQUESTEVENT,
	APPPACKETTYPE_CSDAMAGEEVENT,
	APPPACKETTYPE_REQUESTKILLEVENT,
	APPPACKETTYPE_CSCONSOLECOMMANDEVENT,
	APPPACKETTYPE_CSHINT,
	APPPACKETTYPE_CSANNOUNCEMENT,
	APPPACKETTYPE_DONATEEVENT,
	APPPACKETTYPE_GAMESPYCSCHALLENGERESPONSEEVENT,

	//
	// Summation
	//
	APPPACKETTYPE_ALL,

	APPPACKETTYPE_COUNT,
};

//-----------------------------------------------------------------------------

#endif //__APPPACKETTYPES_H__









	//APPPACKETTYPE_FLAGCAPEVENT,
	//APPPACKETTYPE_STEALTHEVENT,
