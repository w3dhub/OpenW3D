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
 *                     $Archive:: /Commando/Code/Commando/apppacketstats.cpp                    $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                   $* 
 *                                                                                             * 
 *                     $Modtime:: 2/21/02 3:01p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 24                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "apppacketstats.h"

#include <memory.h>
#include <string.h>

#include "wwdebug.h"
#include "mathutil.h"
#include "networkobjectmgr.h"
#include "wwprofile.h"

//
// Class statics
//
DWORD				cAppPacketStats::PacketsSent[];
DWORD				cAppPacketStats::BitsSent[];
DWORD				cAppPacketStats::BitsSentTier[][PACKET_TIER_COUNT];
DWORD				cAppPacketStats::ObjectTally[];
StringClass		cAppPacketStats::WorkingString;

//-----------------------------------------------------------------------------
void
cAppPacketStats::Reset
(	
	void
)
{
	::memset(&PacketsSent,	0, sizeof(PacketsSent));
	::memset(&BitsSent,		0, sizeof(BitsSent));
	::memset(&BitsSentTier,	0, sizeof(BitsSentTier));
	::memset(&ObjectTally,	0, sizeof(ObjectTally));
}

//-----------------------------------------------------------------------------
void
cAppPacketStats::Dump_Diagnostics
(	
	void
)
{
	WWDEBUG_SAY(("\n"));
	WWDEBUG_SAY(("---------------------------------------------------------\n"));
	WWDEBUG_SAY(("cAppPacketStats::Dump_Diagnostics:\n"));
	WWDEBUG_SAY(("%s\n", Get_Heading()));

	for (BYTE i = 0; i < APPPACKETTYPE_COUNT; i++)
	{
		WWDEBUG_SAY(("%s\n", Get_Description(i)));
	}

	WWDEBUG_SAY(("\n"));
	WWDEBUG_SAY(("---------------------------------------------------------\n"));
}

//-----------------------------------------------------------------------------
void
cAppPacketStats::Increment_Packets_Sent
(	
	BYTE app_packet_type
)
{
	WWASSERT(app_packet_type != APPPACKETTYPE_ALL && app_packet_type < APPPACKETTYPE_COUNT);

	PacketsSent[app_packet_type]++;
	
	PacketsSent[APPPACKETTYPE_ALL]++;
}

//-----------------------------------------------------------------------------
void
cAppPacketStats::Increment_Bits_Sent
(	
	BYTE	app_packet_type,
	DWORD	bits
)
{
	WWASSERT(app_packet_type != APPPACKETTYPE_ALL && app_packet_type < APPPACKETTYPE_COUNT);
	WWASSERT(bits >= 0);

	BitsSent[app_packet_type] += bits;

	BitsSent[APPPACKETTYPE_ALL] += bits;
}

//-----------------------------------------------------------------------------
void
cAppPacketStats::Increment_Bits_Sent_Tier
(	
	BYTE					app_packet_type,
	PACKET_TIER_ENUM	tier,
	DWORD					bits
)
{
	WWASSERT(app_packet_type != APPPACKETTYPE_ALL && app_packet_type < APPPACKETTYPE_COUNT);
	WWASSERT(bits >= 0);

	BitsSentTier[app_packet_type][tier] += bits;

	BitsSentTier[APPPACKETTYPE_ALL][tier] += bits;
}

//-----------------------------------------------------------------------------
DWORD
cAppPacketStats::Get_Packets_Sent
(	
	BYTE app_packet_type
)
{
	WWASSERT(app_packet_type < APPPACKETTYPE_COUNT);

	return PacketsSent[app_packet_type];
}

//-----------------------------------------------------------------------------
DWORD
cAppPacketStats::Get_Bits_Sent
(	
	BYTE	app_packet_type
)
{
	WWASSERT(app_packet_type < APPPACKETTYPE_COUNT);

	return BitsSent[app_packet_type];
}

//-----------------------------------------------------------------------------
DWORD
cAppPacketStats::Get_Bits_Sent_Tier
(	
	BYTE					app_packet_type,
	PACKET_TIER_ENUM	tier
)
{
	WWASSERT(app_packet_type < APPPACKETTYPE_COUNT);

	return BitsSentTier[app_packet_type][tier];
}

//-----------------------------------------------------------------------------
#define ADD_CASE(exp)	case exp: return #exp;

LPCSTR
cAppPacketStats::Interpret_Type
(
	BYTE app_packet_type
)
{
   switch (app_packet_type) 
	{
		//
		// S->S
		//	
		ADD_CASE(APPPACKETTYPE_UNKNOWN);
		ADD_CASE(APPPACKETTYPE_SIMPLE);
		ADD_CASE(APPPACKETTYPE_SOLDIER);
		ADD_CASE(APPPACKETTYPE_VEHICLE);
		ADD_CASE(APPPACKETTYPE_TURRET);
		ADD_CASE(APPPACKETTYPE_BUILDING);
		ADD_CASE(APPPACKETTYPE_PLAYER);
		ADD_CASE(APPPACKETTYPE_TEAM);
		ADD_CASE(APPPACKETTYPE_GAMEOPTIONSEVENT);
		ADD_CASE(APPPACKETTYPE_PLAYERKILLEVENT);
		ADD_CASE(APPPACKETTYPE_PURCHASERESPONSEEVENT);
		ADD_CASE(APPPACKETTYPE_SCTEXTOBJ);
		ADD_CASE(APPPACKETTYPE_SVRGOODBYEEVENT);
		ADD_CASE(APPPACKETTYPE_WINEVENT);
		ADD_CASE(APPPACKETTYPE_POWERUP);
		ADD_CASE(APPPACKETTYPE_STATIC);
		ADD_CASE(APPPACKETTYPE_DOOR);
		ADD_CASE(APPPACKETTYPE_ELEVATOR);
		ADD_CASE(APPPACKETTYPE_DSAPO);
		ADD_CASE(APPPACKETTYPE_SERVERFPS);
		ADD_CASE(APPPACKETTYPE_CONSOLECOMMANDEVENT);
		ADD_CASE(APPPACKETTYPE_RESETWINSEVENT);
		ADD_CASE(APPPACKETTYPE_EVICTIONEVENT);
		ADD_CASE(APPPACKETTYPE_NETWEATHER);
		ADD_CASE(APPPACKETTYPE_GAMEDATAUPDATEEVENT);
		ADD_CASE(APPPACKETTYPE_SCPINGRESPONSEEVENT);
		ADD_CASE(APPPACKETTYPE_BASECONTROLLER);
		ADD_CASE(APPPACKETTYPE_CINEMATIC);
		ADD_CASE(APPPACKETTYPE_C4);
		ADD_CASE(APPPACKETTYPE_BEACON);
		ADD_CASE(APPPACKETTYPE_SCEXPLOSIONEVENT);
		ADD_CASE(APPPACKETTYPE_SCOBELISKEVENT);
		ADD_CASE(APPPACKETTYPE_SCANNOUNCEMENT);
		ADD_CASE(APPPACKETTYPE_NETBACKGROUND);
		ADD_CASE(APPPACKETTYPE_GAMESPYSCCHALLENGEEVENT);


		//
		// C->S
		//	
		ADD_CASE(APPPACKETTYPE_CLIENTCONTROL);
		ADD_CASE(APPPACKETTYPE_CSTEXTOBJ);
		ADD_CASE(APPPACKETTYPE_SUICIDEEVENT);
		ADD_CASE(APPPACKETTYPE_CHANGETEAMEVENT);
		ADD_CASE(APPPACKETTYPE_MONEYEVENT);
		ADD_CASE(APPPACKETTYPE_WARPEVENT);
		ADD_CASE(APPPACKETTYPE_PURCHASEREQUESTEVENT);
		ADD_CASE(APPPACKETTYPE_CLIENTGOODBYEEVENT);
		ADD_CASE(APPPACKETTYPE_BIOEVENT);
		ADD_CASE(APPPACKETTYPE_LOADINGEVENT);
		ADD_CASE(APPPACKETTYPE_GODMODEEVENT);
		ADD_CASE(APPPACKETTYPE_VIPMODEEVENT);
		ADD_CASE(APPPACKETTYPE_SCOREEVENT);
		ADD_CASE(APPPACKETTYPE_CLIENTBBOEVENT);
		ADD_CASE(APPPACKETTYPE_CLIENTFPS);
		ADD_CASE(APPPACKETTYPE_CSPINGREQUESTEVENT);
		ADD_CASE(APPPACKETTYPE_CSDAMAGEEVENT);
		ADD_CASE(APPPACKETTYPE_REQUESTKILLEVENT);
		ADD_CASE(APPPACKETTYPE_CSCONSOLECOMMANDEVENT);
		ADD_CASE(APPPACKETTYPE_CSHINT);
		ADD_CASE(APPPACKETTYPE_CSANNOUNCEMENT);
		ADD_CASE(APPPACKETTYPE_DONATEEVENT);
		ADD_CASE(APPPACKETTYPE_GAMESPYCSCHALLENGERESPONSEEVENT);

		//
		// Summation
		//
		ADD_CASE(APPPACKETTYPE_ALL);

		default:
         break;
   }

	return "ERROR";
	DIE;
}

//-----------------------------------------------------------------------------
void
cAppPacketStats::Update_Object_Tally
(	
	void
)
{
	::memset(&ObjectTally, 0, sizeof(ObjectTally));

	int count = NetworkObjectMgrClass::Get_Object_Count();

	for (int index = 0; index < count; index ++) 
	{
		NetworkObjectClass * p_object = NetworkObjectMgrClass::Get_Object(index);

		if (p_object != NULL) 
		{
			BYTE type = p_object->Get_App_Packet_Type();
			WWASSERT(type < APPPACKETTYPE_ALL);

			ObjectTally[type]++;
			ObjectTally[APPPACKETTYPE_ALL]++;

			/*
			if (type == APPPACKETTYPE_UNKNOWN)
			{
				WWDEBUG_SAY(("WTF is this?\n"));
			}
			*/
		}
	}
}

//-----------------------------------------------------------------------------
DWORD
cAppPacketStats::Get_Object_Tally
(	
	BYTE app_packet_type
)
{
	WWASSERT(app_packet_type < APPPACKETTYPE_COUNT);

	return ObjectTally[app_packet_type];
}

//-----------------------------------------------------------------------------
StringClass &
cAppPacketStats::Get_Heading
(	
	void
)
{
	//StringClass description;

	WorkingString.Format(
		"%-30s %-8s %-10s %-10s %-7s %-7s %-7s %-7s %-7s %-7s", 
		"Type",
		"Tally",
		"Packets",
		"Bytes",
		"PC",
		"Avg.",
		"PC TC",
		"PC TR",
		"PC TO",
		"PC TF"
		);

	return WorkingString;
}

//-----------------------------------------------------------------------------
StringClass &
cAppPacketStats::Get_Description
(	
	BYTE type
)
{
	WWASSERT(type < APPPACKETTYPE_COUNT);

	float num_bytes = BitsSent[type] / 8.0f;
	DWORD average_bytes = 0;
	if (PacketsSent[type] > 0)
	{
		average_bytes = cMathUtil::Round(num_bytes / (float) PacketsSent[type]);
	}

	float percentage = 0;
	if (BitsSent[APPPACKETTYPE_ALL] > 0)
	{
		percentage = 100 * (BitsSent[type] / (float) BitsSent[APPPACKETTYPE_ALL]);
	}

	//
	// Strip the leading "APPPACKETTYPE_"
	//
	WWASSERT(::strlen(Interpret_Type(type)) > 14);
	char name[200] = "";
	::strcpy(name, &Interpret_Type(type)[14]);

	//
	// Tier percentages
	//
	DWORD t0 = 0;
	DWORD t1 = 0;
	DWORD t2 = 0;
	DWORD t3 = 0;
	if (BitsSent[type] > 0)
	{
		float bits = BitsSent[type];

		t0 = cMathUtil::Round(100 * (BitsSentTier[type][0] / bits));
		t1 = cMathUtil::Round(100 * (BitsSentTier[type][1] / bits));
		t2 = cMathUtil::Round(100 * (BitsSentTier[type][2] / bits));
		t3 = cMathUtil::Round(100 * (BitsSentTier[type][3] / bits));
	}

	//StringClass description;
	WorkingString.Format(
		"%-30s %-8d %-10d %-10d %-7.1f %-7d %-7d %-7d %-7d %-7d", 
		name, 
		ObjectTally[type],
		PacketsSent[type],
		cMathUtil::Round(num_bytes), 
		percentage,
		average_bytes,
		t0, 
		t1, 
		t2, 
		t3
		);

	/**/
	//
	// Replace all solo zero's with a space.
	//
	char last = ' ';
	char next = ' ';
	char * p = WorkingString.Peek_Buffer();
	for (size_t i = 0; i < WorkingString.Get_Length(); i++)
	{
		if (i == 0)
		{
			last = ' ';
		}
		else 
		{
			last = p[i - 1];
		}

		if (i == WorkingString.Get_Length() - 1)
		{
			next = ' ';
		}
		else 
		{
			next = p[i + 1];
		}

		if (last == ' ' && next == ' ' && p[i] == '0') 
		{
			p[i] = ' ';
		}
	}
	/**/

	return WorkingString;
}






















		//ADD_CASE(APPPACKETTYPE_FLAGCAPEVENT);
		//ADD_CASE(APPPACKETTYPE_STEALTHEVENT);
