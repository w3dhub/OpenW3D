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
 *                     $Archive:: /Commando/Code/Commando/apppacketstats.h                           $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                   $*
 *                                                                                             *
 *                     $Modtime:: 10/15/01 4:18p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __APPPACKETSTATS_H__
#define __APPPACKETSTATS_H__

#include "bittype.h"
#include "apppackettypes.h"
#include "networkobject.h"

//-----------------------------------------------------------------------------
//
// Record and report app packet stats
//
class	cAppPacketStats
{
public:
	static void				Reset(void);
	static void				Dump_Diagnostics(void);
	static LPCSTR			Interpret_Type(BYTE app_packet_type);
	static void				Update_Object_Tally(void);

	static void				Increment_Packets_Sent(BYTE app_packet_type);
	static void				Increment_Bits_Sent(BYTE app_packet_type, DWORD bits);
	static void				Increment_Bits_Sent_Tier(BYTE app_packet_type, PACKET_TIER_ENUM tier, DWORD bits);

	static DWORD			Get_Packets_Sent(BYTE app_packet_type);
	static DWORD			Get_Bits_Sent(BYTE app_packet_type);
	static DWORD			Get_Bits_Sent_Tier(BYTE app_packet_type, PACKET_TIER_ENUM tier);

	static DWORD			Get_Object_Tally(BYTE app_packet_type);

	static StringClass &	Get_Heading(void);
	static StringClass &	Get_Description(BYTE app_packet_type);

private:
	static DWORD			PacketsSent[APPPACKETTYPE_COUNT];
	static DWORD			BitsSent[APPPACKETTYPE_COUNT];
	static DWORD			BitsSentTier[APPPACKETTYPE_COUNT][PACKET_TIER_COUNT];
	static DWORD			ObjectTally[APPPACKETTYPE_COUNT];

	static StringClass	WorkingString;
};

//-----------------------------------------------------------------------------

#endif	// __APPPACKETSTATS_H__
