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
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Denzil_l $
*     $Revision: 2 $
*     $Modtime: 3/29/00 1:17p $
*     $Archive: /Commando/Code/Scripts/strtrim.cpp $
*
******************************************************************************/

#include "strtrim.h"
#include <ctype.h>
#include <stddef.h>
#include <string.h>


/******************************************************************************
*
* NAME
*     strtrim
*
* DESCRIPTION
*     Trim leading and trailing white space off of a string.
*
* INPUTS
*     char* buffer
*
* RESULTS
*     char*
*
******************************************************************************/

char* strtrim(char* buffer)
	{
	if (buffer != NULL)
		{
		// Strip leading white space from the string.
		char* source = buffer;
		
		while (isspace(*source))
			source++;

		if (source != buffer)
			strcpy(buffer, source);

		// Clip trailing white space from the string.
		for (size_t index = ::strlen(buffer); index > 0; )
			{
			const size_t char_index = index - 1;
			if (isspace(static_cast<unsigned char>(buffer[char_index])))
				buffer[char_index] = '\0';
			else
				break;
			--index;
			}
		}

	return buffer;
	}
