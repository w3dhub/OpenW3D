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

/****************************************************************************
*
* FILE
*     Main.cpp
*
* DESCRIPTION
*     Main entry point for console application
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Denzil_l $
*     $Revision: 3 $
*     $Modtime: 6/19/00 2:37p $
*     $Archive: /Commando/Code/Tests/BitPackTest/Code/Main.cpp $
*
****************************************************************************/

#include "typeencoder.h"
#include "bitpacker.h"
#include <stdio.h>
#include <assert.h>

#define BUFFER_SIZE 2048

void main(int, void**)
{
	void* buffer = new unsigned char[BUFFER_SIZE];

	if (buffer == NULL) {
		printf("Failed to allocate buffer\n");
	}

	TypeEncoder::SetTypePrecision(TypeEncoder::X_POSITION, 0.0f, 1000.0f, 0.1f);

	TypeEncoder encoder(buffer, BUFFER_SIZE);

	encoder.PutBool(true);
	encoder.PutBool(true);
	encoder.PutBool(false);
	encoder.PutInt(5, 3);
	encoder.PutInt(1000, 12);
	encoder.PutInt(-67, 7);
	encoder.PutBool(true);
	encoder.PutType(TypeEncoder::X_POSITION, 99.9f);
	encoder.Flush();


	printf("Finished : %d\n", encoder.GetPackedSize());

	bool b;
	int i;
	float f;

	encoder.Reset();
	b = encoder.GetBool();
	b = encoder.GetBool();
	b = encoder.GetBool();
	i = encoder.GetInt(3);
	i = encoder.GetInt(12);
	i = encoder.GetInt(7);
	b = encoder.GetBool();
	f = encoder.GetType(TypeEncoder::X_POSITION);

	delete[] buffer;
}
