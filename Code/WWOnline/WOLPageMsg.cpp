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
*     $Archive: /Commando/Code/WWOnline/WOLPageMsg.cpp $
*
* DESCRIPTION
*     Page message
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 6 $
*     $Modtime: 12/19/01 2:12p $
*
******************************************************************************/

#include "WOLPageMsg.h"

namespace WOL
{
#include <wolapi/chatdefs.h>
}

namespace WWOnline {

/******************************************************************************
*
* NAME
*     PageMessage::PageMessage
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

PageMessage::PageMessage(const char* pagersName, const char* message) :
		mPager(pagersName),
		mMessage(message)
	{
	}


/******************************************************************************
*
* NAME
*     PageMessage::PageMessage
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

PageMessage::PageMessage(const unichar_t* pagersName, const unichar_t* message) :
		mPager(pagersName),
		mMessage(message)
	{
	}


/******************************************************************************
*
* NAME
*     PageMessage::PageMessage
*
* DESCRIPTION
*     Copy constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

PageMessage::PageMessage(const PageMessage& page)
	{
	*this = page;
	}


/******************************************************************************
*
* NAME
*     PageMessage::~PageMessage
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

PageMessage::~PageMessage()
	{
	}


/******************************************************************************
*
* NAME
*     PageMessage::operator=
*
* DESCRIPTION
*     Assignment operator
*
* INPUTS
*
* RESULT
*
******************************************************************************/

const PageMessage& PageMessage::operator=(const PageMessage& page)
	{
	mPager = page.mPager;
	mMessage = page.mMessage;
	return (*this);
	}

} // namespace WWOnline
