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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/WWOnline/GameResPacket.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 12/11/01 6:23p $
*
******************************************************************************/

#include "GameResPacket.h"
#include <assert.h>
#include <string.h>
#include <winsock.h>
#include <algorithm>

namespace WWOnline {

/**************************************************************************
 * GameResPacket::~GameResPacket -- destroys a packet class be freeing list   *
 *                                                                        *
 * INPUT:      none                                                       *
 *                                                                        *
 * OUTPUT:     none                                                       *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/24/1996 PWG : Created.                                            *
 *========================================================================*/
GameResPacket::~GameResPacket(void)
	{
	GameResField* current;
	GameResField* next;

	// Loop through the entire field list and delete each entry.
	for (current = mHead; current; current = next)
		{
		next = current->mNext;
		delete current;
		}
	}


/**************************************************************************
 * GameResPacket::ADD_FIELD -- Adds a GameResField entry to head of packet li *
 *                                                                        *
 * INPUT:      GameResField * - a properly constructed field class entry.   *
 *                                                                        *
 * OUTPUT:     none                                                       *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/24/1996 PWG : Created.                                            *
 *========================================================================*/
void GameResPacket::Add_Field(GameResField *field)
	{
	field->mNext = mHead;
	mHead = field;
	}


/**************************************************************************
 * GameResPacket::GameResPacket -- Creates a Packet object from a COMMS packe *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/22/1996 PWG : Created.                                            *
 *========================================================================*/
GameResPacket::GameResPacket(unsigned char* curbuf)
	{
	// Pull the size and packet ID out of the linear packet stream.
	mSize = ntohl(*(unsigned long*)curbuf);
	curbuf += sizeof(mSize);

	mID = ntohs(*(unsigned short*)curbuf);
	curbuf += sizeof(mID);
	
	mReserved = ntohs(*(unsigned short*)curbuf);
	curbuf += sizeof(mReserved);

	mHead = NULL;

	// Calculate the remaining size so that we can loop through the
	// packets and extract them.
	unsigned long remaining_size = (mSize - (sizeof(mSize) + sizeof(mID) + sizeof(mReserved)));

	// Loop through the linear packet until we run out of room and
	// create a field for each.
	while (remaining_size > 0)
		{
		GameResField* field = new GameResField;

		// Copy the adjusted header into the buffer and then advance the buffer
		memcpy(field, curbuf, GAMERESFIELD_HEADER_SIZE);
		curbuf += GAMERESFIELD_HEADER_SIZE;
		remaining_size -= GAMERESFIELD_HEADER_SIZE;

		// Copy the data into the buffer
		unsigned short size = ntohs(field->mSize);
		field->mData = new unsigned char[size];
		memcpy(field->mData, curbuf, size);
		curbuf += size;
		remaining_size -= size;

		// Make sure we allow for the pad bytes.
		int pad = (4 - (ntohs(field->mSize) & 3)) & 3;
		curbuf += pad;
		remaining_size -= pad;

		// Convert the field back to the host format
		field->Net_To_Host();

		// Finally add the field to the field list in the packet
		// structure.
		Add_Field(field);
		}
	}


/**************************************************************************
 * CREATE_COMMS_PACKET -- Walks field list creating a packet              *
 *                                                                        *
 * INPUT:      short - the id of the packet so the server can identify it *
 *               unsigned short & - the size of the packet returned here  *
 *                                                                        *
 * OUTPUT:     void * pointer to the linear packet data                   *
 *                                                                        *
 * WARNINGS:    This routine allocates memory that the user is responsible*
 *              for freeing.                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/22/1996 PWG : Created.                                            *
 *========================================================================*/
unsigned char* GameResPacket::Create_Comms_Packet(unsigned long& size, char* sig_name, unsigned long& sig_offset)
	{
	GameResField* current;
	sig_offset = 0;

	// Size starts at 8 because that is the size of the packet header. (size[4] + id[2] + reserved[2])
	size = (sizeof(mSize) + sizeof(mID) + sizeof(mReserved));

	// Take a quick spin through and calculate the size of the packet we
	// are building.
	for (current = mHead; current; current = current->mNext)
		{
		size += (unsigned long)GAMERESFIELD_HEADER_SIZE;			// add in packet header size
		size += current->mSize;				// add in data size
		size += (4 - (current->mSize & 3)) & 3;		// add in pad value to dword align next packet
		}

	// Now that we know the size allocate a buffer big enough to hold the
	// packet.
	unsigned char* bufferStart = new unsigned char[size];
	unsigned char* curbuf = bufferStart;

	// write the size into the packet header
	*(unsigned long*)curbuf = htonl(size);
	curbuf += sizeof(unsigned long);

	*(unsigned short*)curbuf = htons(mID);
	curbuf += sizeof(unsigned short);

	*(unsigned short*)curbuf = htons(mReserved);
	curbuf += sizeof(unsigned short);

	// Ok now that the actual header information has been written we need to write out
	// field information.
	for (current = mHead; current; current = current->mNext)
		{
		#ifdef _DEBUG
		current->DebugDump();
		#endif

		unsigned short fieldSize = current->mSize;

		// Temporarily convert the packet to net format (this saves alot of
		// effort, and seems safe...)
		current->Host_To_Net();

		// Copy the adjusted header into the buffer and then advance the buffer
		memcpy(curbuf, current, GAMERESFIELD_HEADER_SIZE);
		curbuf += GAMERESFIELD_HEADER_SIZE;

		// If this is the sig data then make a note of the offset into the packet.
		if (sig_name && strncmp(sig_name, current->mID, 4) == 0)
			{
			// If sig_offset isn't zero then that would indicate that there were multiple sig entries in the packet.
			assert(sig_offset == 0);
			sig_offset = (curbuf - bufferStart);
			}

		// Copy the data into the buffer and then advance the buffer
		memcpy(curbuf, current->mData, fieldSize);
		curbuf += fieldSize;

		// Finally take care of any pad bytes by setting them to 0
		int pad = ((4 - (fieldSize & 3)) & 3);

		// If there is any pad left over, make sure you memset it
		// to zeros, so it looks like a pad.
		if (pad)
			{
			memset(curbuf, 0, pad);
			curbuf += pad;
			}

		current->Net_To_Host();
		}

	return bufferStart;
	}


/**************************************************************************
 * GameResPacket::FIND_FIELD -- Finds a field if it exists in the packets   *
 *                                                                        *
 * INPUT:      char *  - the id of the field we are looking for.          *
 *                                                                        *
 * OUTPUT:     GameResField * pointer to the field class                    *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/23/1996 PWG : Created.                                            *
 *========================================================================*/
GameResField *GameResPacket::Find_Field(char *id)
	{
	for (GameResField *current = mHead; current; current = current->mNext)
		{
		if (strncmp(id, current->mID, 4) == 0)
			return current;
		}

	return NULL;
	}


/**************************************************************************
 * GET_FIELD -- Find specified name and returns data                      *
 *                                                                        *
 * INPUT:      char *   - the id of the field that holds the data.        *
 *             char &   - the reference to store the data into            *
 *                                                                        *
 * OUTPUT:     true if the field was found, false if it was not.          *
 *                                                                        *
 * WARNINGS:   The data reference is not changed if the field is not      *
 *               found.                                                   *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/23/1996 PWG : Created.                                            *
 *========================================================================*/
bool GameResPacket::Get_Field(char *id, char &data)
	{
	GameResField *field = Find_Field(id);

	if (field)
		{
		data = *((char*)field->mData);
		}

	return((field) ? true : false);
	}


/**************************************************************************
 * GET_FIELD -- Find specified name and returns data                      *
 *                                                                        *
 * INPUT:      char *   - the id of the field that holds the data.        *
 *               unsigned char &   - the reference to store the data into *
 *                                                                        *
 * OUTPUT:      true if the field was found, false if it was not.         *
 *                                                                        *
 * WARNINGS:   The data reference is not changed if the field is not      *
 *               found.                                                   *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/23/1996 PWG : Created.                                            *
 *========================================================================*/
bool GameResPacket::Get_Field(char *id, unsigned char &data)
{
	GameResField *field = Find_Field(id);

	if (field)
		{
		data = *((unsigned char *)field->mData);
		}

	return((field) ? true : false);
	}


/**************************************************************************
 * GET_FIELD -- Find specified name and returns data                      *
 *                                                                        *
 * INPUT:      char *   - the id of the field that holds the data.        *
 *               short &   - the reference to store the data into         *
 *                                                                        *
 * OUTPUT:      true if the field was found, false if it was not.         *
 *                                                                        *
 * WARNINGS:   The data reference is not changed if the field is not      *
 *               found.                                                   *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/23/1996 PWG : Created.                                            *
 *========================================================================*/
bool GameResPacket::Get_Field(char *id, short &data)
	{
	GameResField *field = Find_Field(id);

	if (field)
		{
		data = *((short *)field->mData);
		}

	return((field) ? true : false);
	}


/**************************************************************************
 * GET_FIELD -- Find specified name and returns data                      *
 *                                                                        *
 * INPUT:      char *   - the id of the field that holds the data.        *
 *               unsigned short &   - the reference to store the data into*
 *                                                                        *
 * OUTPUT:      true if the field was found, false if it was not.         *
 *                                                                        *
 * WARNINGS:   The data reference is not changed if the field is not      *
 *               found.                                                   *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/23/1996 PWG : Created.                                            *
 *========================================================================*/
bool GameResPacket::Get_Field(char *id, unsigned short &data)
	{
	GameResField *field = Find_Field(id);

	if (field)
		{
		data = *((unsigned short *)field->mData);
		}

	return((field) ? true : false);
	}


/**************************************************************************
 * GET_FIELD -- Find specified name and returns data                      *
 *                                                                        *
 * INPUT:      char *   - the id of the field that holds the data.        *
 *               long &   - the reference to store the data into          *
 *                                                                        *
 * OUTPUT:      true if the field was found, false if it was not.         *
 *                                                                        *
 * WARNINGS:   The data reference is not changed if the field is not      *
 *               found.                                                   *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/23/1996 PWG : Created.                                            *
 *========================================================================*/
bool GameResPacket::Get_Field(char *id, long &data)
	{
	GameResField *field = Find_Field(id);

	if (field)
		{
		data = *((long *)field->mData);
		}

	return((field) ? true : false);
	}


/**************************************************************************
 * GET_FIELD -- Find specified name and returns data as a string          *
 *                                                                        *
 * INPUT:      char *   - the id of the field that holds the data.        *
 *               char *   - the string to store the data into             *
 *                                                                        *
 * OUTPUT:      true if the field was found, false if it was not.         *
 *                                                                        *
 * WARNINGS:   The string is not changed if the field is not found.  It   *
 *               is assumed that the string variabled specified by the    *
 *               pointer is large enough to hold the data.                *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/23/1996 PWG : Created.                                            *
 *========================================================================*/
bool GameResPacket::Get_Field(char *id, char *data)
	{
	GameResField *field = Find_Field(id);

	if (field)
		{
		strcpy(data, (char *)field->mData);
		}

	return((field) ? true : false);
	}



/**************************************************************************
 * GET_FIELD -- Find specified name and returns data                      *
 *                                                                        *
 * INPUT:    char *   - the id of the field that holds the data           *
 *           unsigned long &   - the reference to store the data into     *
 *                                                                        *
 * OUTPUT:   true if the field was found, false if it was not.            *
 *                                                                        *
 * WARNINGS: The data reference is not changed if the field is not        *
 *           found.                                                       *
 *                                                                        *
 * HISTORY:                                                               *
 *   04/23/1996 PWG : Created.                                            *
 *========================================================================*/
bool GameResPacket::Get_Field(char *id, unsigned long &data)
	{
	GameResField *field = Find_Field(id);

	if (field)
		{
		data = *((unsigned long *)field->mData);
		}

	return((field) ? true : false);
	}


/**************************************************************************
 * GET_FIELD -- Find specified name and returns data                      *
 *                                                                        *
 * INPUT:   char *   - the id of the field that holds the data.           *
 *          void * - the reference to store the data into                 *
 *           int    - the length of the buffer passed in                  *
 *                                                                        *
 * OUTPUT:  true if the field was found, false if it was not.             *
 *                                                                        *
 * WARNINGS:  The data reference is not changed if the field is not       *
 *            found.                                                      *
 *                                                                        *
 * HISTORY:                                                               *
 *   6/4/96 4:46PM ST : Created                                           *
 *========================================================================*/
bool GameResPacket::Get_Field(char *id, void *data, int &length)
	{
	GameResField *field = Find_Field(id);

	if (field)
		{
		memcpy(data, field->mData, std::min((int)field->mSize, length));
		length = (int) field->mSize;
		}

	return((field) ? true : false);
	}

} // namespace WWOnline
