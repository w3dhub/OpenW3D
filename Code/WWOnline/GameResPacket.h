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
*     $Archive: /Commando/Code/WWOnline/GameResPacket.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 5 $
*     $Modtime: 1/25/02 11:53a $
*
******************************************************************************/

#pragma once

#ifndef __GAMERESPACKET_H__
#define __GAMERESPACKET_H__

#pragma warning(disable : 4711)

#include "GameResField.h"

namespace WWOnline {

class GameResPacket {
	public:
		GameResPacket(short id = 0) :
				mSize(0),
				mID(id),
				mReserved(0),
				mHead(0)
			{}

		GameResPacket(unsigned char *cur_buf);
		~GameResPacket(void);

		// This function allows us to add a field to the start of the list.  As the field is just
		//   a big linked list it makes no difference which end we add a member to.
		void Add_Field(GameResField *field);

		//
		// These conveniance functions allow us to add a field directly to the list without
		// having to worry about newing one first.
		//
		void Add_Field(const char *field, char data) {Add_Field(new GameResField(field, data));};
		void Add_Field(const char *field, unsigned char data) {Add_Field(new GameResField(field, data));};
		void Add_Field(const char *field, short data) {Add_Field(new GameResField(field, data));};
		void Add_Field(const char *field, unsigned short data) {Add_Field(new GameResField(field, data));};
		void Add_Field(const char *field, long data) {Add_Field(new GameResField(field, data));};
		void Add_Field(const char *field, unsigned long data) {Add_Field(new GameResField(field, data));};
		void Add_Field(const char *field, const char *data) {Add_Field(new GameResField(field, data));};
		void Add_Field(const char *field, void *data, int length) {Add_Field(new GameResField(field, data, length));};

		//
		// These functions search for a field of a given name in the list and
		// return the data via a reference value.
		//
		GameResField *Find_Field(char *id);
		bool Get_Field(char *id, char &data);
		bool Get_Field(char *id, unsigned char &data);
		bool Get_Field(char *id, short &data);
		bool Get_Field(char *id, unsigned short &data);
		bool Get_Field(char *id, long &data);
		bool Get_Field(char *id, unsigned long &data);
		bool Get_Field(char *id, char *data);
		bool Get_Field(char *id, void *data, int &length);

		unsigned char* Create_Comms_Packet(unsigned long& size, char* sig_name, unsigned long& sig_offset);

	private:
		unsigned long mSize;
		unsigned short mID;
		unsigned short mReserved;

		GameResField* mHead;
		GameResField* mCurrent;
};

} // namespace WWOnline

#endif