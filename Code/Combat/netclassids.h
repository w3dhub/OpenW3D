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
 *                     $Archive:: /Commando/Code/Combat/netclassids.h         $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 2/21/02 3:00p                                               $*
 *                                                                                             *
 *                    $Revision:: 27                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __NET_CLASSIDS_H
#define __NET_CLASSIDS_H


////////////////////////////////////////////////////////////////
//	Network class IDs
////////////////////////////////////////////////////////////////
enum
{
	//
	// S-created objects
	//
	NETCLASSID_GAMEOBJ						= 1000,
	NETCLASSID_SCTEXTOBJ,
	NETCLASSID_PLAYERKILL,
	NETCLASSID_WIN,
	//NETCLASSID_FLAGCAPEVENT,
	NETCLASSID_PURCHASERESPONSEEVENT,
	NETCLASSID_CONSOLECOMMANDEVENT,
	NETCLASSID_RESETWINSEVENT,
	NETCLASSID_SVRGOODBYEEVENT,
	NETCLASSID_GAMEOPTIONSEVENT,
	NETCLASSID_EVICTIONEVENT,
	NETCLASSID_TEAM,
	NETCLASSID_PLAYER,
	NETCLASSID_GAMEDATAUPDATEEVENT,
	NETCLASSID_SCPINGRESPONSEEVENT,
	NETCLASSID_SCEXPLOSIONEVENT,
	NETCLASSID_SCOBELISKEVENT,
	NETCLASSID_SCANNOUNCEMENT,
	NETCLASSID_GAMESPYSCCHALLENGEEVENT,

	//
	// C-created objects
	//
	NETCLASSID_CLIENTCONTROL,
	NETCLASSID_CSTEXTOBJ,
	NETCLASSID_SUICIDEEVENT,
	NETCLASSID_CHANGETEAMEVENT,
	NETCLASSID_MONEYEVENT,
	NETCLASSID_WARPEVENT,
	NETCLASSID_PURCHASEREQUESTEVENT,
	NETCLASSID_CLIENTGOODBYEEVENT,
	NETCLASSID_BIOEVENT,
	NETCLASSID_LOADINGEVENT,
	NETCLASSID_GODMODEEVENT,
	NETCLASSID_VIPMODEEVENT,
	//NETCLASSID_STEALTHEVENT,
	NETCLASSID_SCOREEVENT,
	NETCLASSID_CLIENTBBOEVENT,
	NETCLASSID_CLIENTFPS,
	NETCLASSID_CSPINGREQUESTEVENT,
	NETCLASSID_CSDAMAGEEVENT,
	NETCLASSID_REQUESTKILLEVENT,
	NETCLASSID_CSCONSOLECOMMANDEVENT,
	NETCLASSID_CSHINT,
	NETCLASSID_CSANNOUNCEMENT,
	NETCLASSID_DONATEEVENT,
	NETCLASSID_GAMESPYCSCHALLENGERESPONSEEVENT,
};


#endif //__NET_CLASSIDS_H















	//NETCLASSID_MOBIUSCAPEVENT,
	//NETCLASSID_LMSPLAYEROUTEVENT,
	//NETCLASSID_MUTATIONEVENT,
	//NETCLASSID_LMSREAPEVENT,
	//NETCLASSID_SPECTATINGEVENT,
