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
// Filename:     encoderlist.cpp
// Project:      wwbitpack.lib
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:
//

#include "encoderlist.h"

#include "wwdebug.h"

//
// Class statics
//
bool cEncoderList::IsCompressionEnabled = true;
cEncoderTypeEntry cEncoderList::EncoderTypes[];

//-----------------------------------------------------------------------------
void cEncoderList::Clear_Entries()
{
	WWDEBUG_SAY(("cEncoderList::Clear_Entries\n"));

	for (int i = 0; i < MAX_ENCODERTYPES; i++) {
		EncoderTypes[i].Invalidate();
	}
}

//-----------------------------------------------------------------------------
cEncoderTypeEntry & cEncoderList::Get_Encoder_Type_Entry(int index)
{
	WWASSERT(index >= 0 && index < MAX_ENCODERTYPES);
	return EncoderTypes[index];
}
