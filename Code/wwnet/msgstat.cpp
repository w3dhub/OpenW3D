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
// Filename:     msgstat.cpp
// Project:      
// Author:       Tom Spencer-Smith
// Date:         
// Description:  
//
//------------------------------------------------------------------------------------
#include "msgstat.h" // I WANNA BE FIRST!

#include <string.h>

#include "mathutil.h"
#include "wwdebug.h"


//
// Class statics
//

//------------------------------------------------------------------------------------
cMsgStat::cMsgStat(void) :
	NumMsgSent(0),
	NumByteSent(0),
	NumMsgRecd(0),
	NumByteRecd(0)
{
	::strcpy(Name, "UNNAMED");
}

//---------------- --------------------------------------------------------------------
cMsgStat::~cMsgStat(void)
{
}

//---------------- --------------------------------------------------------------------
void cMsgStat::Increment_Num_Msg_Sent(int increment)	
{
	WWASSERT(increment > 0); 

	NumMsgSent += increment;
}

//---------------- --------------------------------------------------------------------
void cMsgStat::Increment_Num_Byte_Sent(int increment)
{
	WWASSERT(increment > 0); 
	
	NumByteSent += increment;
}

//---------------- --------------------------------------------------------------------
void cMsgStat::Increment_Num_Msg_Recd(int increment)
{
	WWASSERT(increment > 0); 
	
	NumMsgRecd += increment;
}

//---------------- --------------------------------------------------------------------
void cMsgStat::Increment_Num_Byte_Recd(int increment)
{
	WWASSERT(increment > 0); 
	
	NumByteRecd += increment;
}

//---------------- --------------------------------------------------------------------
unsigned int	cMsgStat::Compute_Avg_Num_Byte_Sent(void) const
{
	unsigned int avg = 0;
	if (NumMsgSent > 0) {
		avg = (unsigned int) cMathUtil::Round(NumByteSent / (float) NumMsgSent);
	}

	return avg;
}

//---------------- --------------------------------------------------------------------
unsigned int cMsgStat::Compute_Avg_Num_Byte_Recd(void) const
{
	unsigned int avg = 0;
	if (NumMsgRecd > 0) {
		avg = (unsigned int) cMathUtil::Round(NumByteRecd / (float) NumMsgRecd);
	}

	return avg;
}

//---------------- --------------------------------------------------------------------
void cMsgStat::Set_Name(const char *name)
{
	WWASSERT(name != NULL);
	WWASSERT(::strlen(name) < sizeof(Name));

	::strcpy(Name, name);
}





