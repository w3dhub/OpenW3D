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
// Filename:     packettype.h
// Project:      wwnet
// Author:       Tom Spencer-Smith
// Date:
// Description:
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef PACKETTYPE_H
#define PACKETTYPE_H

//
// Don't use these at application level
//
enum
{
   PACKETTYPE_FIRST,

	PACKETTYPE_UNRELIABLE = PACKETTYPE_FIRST,
   PACKETTYPE_RELIABLE,
   PACKETTYPE_ACK,
   PACKETTYPE_KEEPALIVE,
   PACKETTYPE_CONNECT_CS,
   PACKETTYPE_ACCEPT_SC,
   PACKETTYPE_REFUSAL_SC,
	PACKETTYPE_FIREWALL_PROBE,

   PACKETTYPE_LAST = PACKETTYPE_FIREWALL_PROBE,

	PACKETTYPE_COUNT
};

//-----------------------------------------------------------------------------

#endif // PACKETTYPE_H
