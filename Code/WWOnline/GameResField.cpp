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
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/WWOnline/GameResField.cpp                    $* 
 *                                                                                             * 
 *                      $Author:: Denzil_l                                                    $*
 *                                                                                             * 
 *                     $Modtime:: 9/20/01 9:43p                                               $*
 *                                                                                             * 
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "GameResField.h"
#include <string.h>
#include <assert.h>
#include <WWDebug\WWDebug.h>

// Get rid of the nameless struct/union warning
#pragma warning(disable: 4201)
#include <winsock.h>
#pragma warning(default: 4201)

namespace WWOnline {

/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

GameResField::GameResField(const char *id, char data)
	{
	strncpy(mID, id, sizeof(mID));
	mDataType = TYPE_CHAR;
	mSize	= sizeof(data);
	mData	= new char[mSize];
	memcpy(mData, &data, mSize);
	mNext = NULL;
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

GameResField::GameResField(const char *id, unsigned char data)
	{
	strncpy(mID, id, sizeof(mID));
	mDataType = TYPE_UNSIGNED_CHAR;
	mSize = sizeof(data);
	mData = new char[mSize];
	memcpy(mData, &data, mSize);
	mNext = NULL;
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

GameResField::GameResField(const char *id, short data)
	{
	strncpy(mID, id, sizeof(mID));
	mDataType = TYPE_SHORT;
	mSize = sizeof(data);
	mData = new char[mSize];
	memcpy(mData, &data, mSize);
	mNext = NULL;
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

GameResField::GameResField(const char *id, unsigned short data)
	{
	strncpy(mID, id, sizeof(mID));
	mDataType = TYPE_UNSIGNED_SHORT;
	mSize = sizeof(data);
	mData = new char[mSize];
	memcpy(mData, &data, mSize);
	mNext = NULL;
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

GameResField::GameResField(const char *id, long data)
	{
	strncpy(mID, id, sizeof(mID));
	mDataType = TYPE_LONG;
	mSize = sizeof(data);
	mData = new char[mSize];
	memcpy(mData, &data, mSize);
	mNext = NULL;
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

GameResField::GameResField(const char *id, unsigned long data)
	{
	strncpy(mID, id, sizeof(mID));
	mDataType = TYPE_UNSIGNED_LONG;
	mSize = sizeof(data);
	mData = new char[mSize];
	memcpy(mData, &data, mSize);
	mNext = NULL;
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

GameResField::GameResField(const char *id, const char *data)
	{
	strncpy(mID, id, sizeof(mID));
	mDataType = TYPE_STRING;
	mSize = (unsigned short)(strlen(data)+1);
	mData = new char[mSize];
	memcpy(mData, data, mSize);
	mNext = NULL;
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

GameResField::GameResField(const char *id, void *data, int length)
	{
	strncpy(mID, id, sizeof(mID));
	mDataType = TYPE_CHUNK;
	mSize = (unsigned short)length;
	mData = new char[mSize];
	memcpy(mData, data, mSize);
	mNext = NULL;
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

GameResField::~GameResField()
	{
  delete[](mData);
	}


/**************************************************************************
 * PACKETCLASS::HOST_TO_NET_FIELD -- Converts host field to net format    *
 *                                                                        *
 * INPUT:      FIELD    * to the data field we need to convert            *
 *                                                                        *
 * OUTPUT:     none                                                       *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/22/1996 PWG : Created.                                            *
 *========================================================================*/
void GameResField::Host_To_Net(void)
	{
	// Before we convert the data type, we should convert the actual data sent.
	switch (mDataType)
		{
		case TYPE_CHAR:
		case TYPE_UNSIGNED_CHAR:
		case TYPE_STRING:
		case TYPE_CHUNK:
			break;

		case TYPE_SHORT:
		case TYPE_UNSIGNED_SHORT:
			*((unsigned short *)mData) = htons(*((unsigned short *)mData));
			break;

		case TYPE_LONG:
		case TYPE_UNSIGNED_LONG:
			*((unsigned long *)mData) = htonl(*((unsigned long *)mData));
			break;

		// Might be good to insert some type of error message here for unknown
		// datatypes.
		default:
			assert(!"GameResField: Unknown data type");
			break;
		}

	// Finally convert over the data type and the size of the packet.
	mDataType = htons(mDataType);
	mSize = htons(mSize);
	}


/**************************************************************************
 * PACKETCLASS::NET_TO_HOST_FIELD -- Converts net field to host format    *
 *                                                                        *
 * INPUT:      FIELD    * to the data field we need to convert            *
 *                                                                        *
 * OUTPUT:     none                                                       *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/22/1996 PWG : Created.                                            *
 *========================================================================*/
void GameResField::Net_To_Host(void)
	{
	// Convert the variables to host order.  This needs to be converted so
	// the switch statement does compares on the data that follows.
	mSize = ntohs(mSize);

	mDataType = ntohs(mDataType);

	// Before we convert the data type, we should convert the actual data sent.
	switch (mDataType)
		{
		case TYPE_CHAR:
		case TYPE_UNSIGNED_CHAR:
		case TYPE_STRING:
		case TYPE_CHUNK:
			break;

		case TYPE_SHORT:
		case TYPE_UNSIGNED_SHORT:
			*((unsigned short *)mData) = ntohs(*((unsigned short *)mData));
			break;

		case TYPE_LONG:
		case TYPE_UNSIGNED_LONG:
			*((unsigned long *)mData) = ntohl(*((unsigned long *)mData));
			break;

		// Might be good to insert some type of error message here for unknown
		// datatypes.
		default:
			assert(!"GameResField: Unknown data type");
			break;
		}
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

#ifdef _DEBUG
void GameResField::DebugDump(void)
	{
	char id[5];
	memcpy(id, mID, 4);
	id[4] = 0;

	switch (mDataType)
		{
		case TYPE_CHAR:
			{
			short data = *((char*)mData);
			WWDEBUG_SAY(("[%4s] %d\n", id, data));
			}
			break;

		case TYPE_UNSIGNED_CHAR:
			{
			short data = *((unsigned char*)mData);
			WWDEBUG_SAY(("[%4s] %ld\n", id, data));
			}
			break;

		case TYPE_SHORT:
			WWDEBUG_SAY(("[%4s] %d\n", id, *((short*)mData)));
			break;

		case TYPE_LONG:
			WWDEBUG_SAY(("[%4s] %ld\n", id, *((long*)mData)));
			break;

		case TYPE_UNSIGNED_SHORT:
			WWDEBUG_SAY(("[%4s] %u\n", id, *((unsigned short*)mData)));
			break;

		case TYPE_UNSIGNED_LONG:
			WWDEBUG_SAY(("[%4s] %lu\n", id, *((unsigned long*)mData)));
			break;

		case TYPE_STRING:
			WWDEBUG_SAY(("[%4s] '%s'\n", id, mData));
			break;

		default:
			WWDEBUG_SAY(("[%4s]\n", id));
			break;
		}
	}
#endif

} // namespace WWOnline
