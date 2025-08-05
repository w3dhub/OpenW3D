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
*     $Archive: /Commando/Code/WWOnline/GameResField.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 2 $
*     $Modtime: 8/15/01 5:49p $
*
******************************************************************************/

#pragma once

#ifndef __GAMERESFIELD_H__
#define __GAMERESFIELD_H__

namespace WWOnline {

class GameResPacket;

#define GAMERESFIELD_HEADER_SIZE (sizeof(GameResField) - (sizeof(void *) * 2))

class GameResField {
	public:
		enum Type {
			TYPE_CHAR = 1,
			TYPE_UNSIGNED_CHAR,
			TYPE_SHORT,
			TYPE_UNSIGNED_SHORT,
			TYPE_LONG,
			TYPE_UNSIGNED_LONG,
			TYPE_STRING,
			TYPE_CHUNK = 20
			};

		friend class GameResPacket;

		// Define constructors to be able to create all the different kinds
		// of fields.
		GameResField(void) {};
		GameResField(const char *id, char data);
		GameResField(const char *id, unsigned char data);
		GameResField(const char *id, short data);
		GameResField(const char *id, unsigned short data);
		GameResField(const char *id, long data);
		GameResField(const char *id, unsigned long data);
		GameResField(const char *id, const char *data);
		GameResField(const char *id, void *data, int length);

		~GameResField();

		void Host_To_Net(void);
		void Net_To_Host(void);

		#ifdef _DEBUG
		void DebugDump(void);
		#endif

	private:
		char mID[4];
		unsigned short mDataType;
		unsigned short mSize;
		void* mData;
		GameResField* mNext;
};

} // namespace WWOnline

#endif __GAMERESFIELD_H__
