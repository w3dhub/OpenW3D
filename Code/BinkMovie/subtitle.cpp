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
*     $Archive: /Commando/Code/BinkMovie/subtitle.cpp $
*
* DESCRIPTION
*     Subtitling support.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Denzil_l $
*     $Modtime: 1/15/02 8:48p $
*     $Revision: 2 $
*
****************************************************************************/

#include "always.h"
#include "subtitle.h"


/******************************************************************************
*
* NAME
*     SubTitleClass::SubTitleClass
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

SubTitleClass::SubTitleClass()
	:
	mTimeStamp(0),
	mDuration(20 * 60),
	mRGBColor(0x00FFFFFF),
	mLinePosition(15),
	mAlignment(Center),
	mCaption(NULL)
{
}


/******************************************************************************
*
* NAME
*     SubTitleClass::~SubTitleClass
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

SubTitleClass::~SubTitleClass()
{
	if (mCaption != NULL) {
		delete[] mCaption;
	}
}


/******************************************************************************
*
* NAME
*     SubTitleClass::SetRGBColor
*
* DESCRIPTION
*     Set the color the subtitle caption should be displayed in.
*
* INPUTS
*     unsigned char red
*     unsigned char green
*     unsigned char blue
*
* RESULTS
*     NONE
*
******************************************************************************/

void SubTitleClass::Set_RGB_Color(
	unsigned char red, 
	unsigned char green,
	unsigned char blue)
{
	// Combine components as 8:8:8
	mRGBColor = (
		((unsigned int)red << 16) |
		((unsigned int)green << 8) |
		(unsigned int)blue);
}


/******************************************************************************
*
* NAME
*     SubTitleClass::SetCaption
*
* DESCRIPTION
*     Set the caption text
*
* INPUTS
*     Caption - Caption string
*
* RESULTS
*     NONE
*
******************************************************************************/

void SubTitleClass::Set_Caption(unichar_t* string)
{
	// Release existing caption
	if (mCaption != NULL) {
		delete[] mCaption;
		mCaption = NULL;
	}

	// Make a copy of caption
	if (string != NULL) {
		size_t length = u_strlen(string);
		mCaption = new unichar_t[length + 1];
		WWASSERT(mCaption != NULL);

		if (mCaption != NULL) {
			u_strcpy(mCaption, string);
		}
	}
}
