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
*     $Archive: /Commando/Code/BinkMovie/subtitle.h $
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

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef _SUBTITLE_H_
#define _SUBTITLE_H_

#include "always.h"
#include "wwdebug.h"
#include "unichar.h"

class SubTitleClass
{
public:
	SubTitleClass();
	~SubTitleClass();

	// Set the time (in milliseconds) at which the subtitle is to be displayed.
	void Set_Display_Time(unsigned int time) { mTimeStamp = time; }
	
	// Retrieve the time in ticks (1/60 seconds) this subtitle is to be displayed.
	unsigned int Get_Display_Time(void) const { return mTimeStamp; }

	// Set the time duration in ticks (1/60 seconds) for the subtitle to remain displayed.
	void Set_Display_Duration(unsigned int duration) { mDuration = duration; }
	
	// Retrieve the duration time in ticks (1/60 seconds) for the subtitle.
	unsigned int Get_Display_Duration(void) const { return mDuration; }

	// Set the color the subtitle caption should be displayed in.
	void Set_RGB_Color(unsigned char red, unsigned char green, unsigned char blue);

	// Retrieve the color of the subtitle
	unsigned int Get_RGB_Color(void) const { return mRGBColor; }

	// Set the line position the subtitle should be displayed at.
	void Set_Line_Position(int linePos)
	{
		WWASSERT((linePos >= 1) && (linePos <= 15));
		mLinePosition = linePos;
	}
	
	// Retrieve the line position to display the subtitle at.
	int Get_Line_Position(void) const { return mLinePosition; }

	// Caption justifications
	typedef enum
	{
		Left,
		Right,
		Center,
	} Alignment;

	// Set the alignment of the subtitle caption
	void Set_Alignment(Alignment align)
		{mAlignment = align;}

	// Retrieve the caption justification
	Alignment Get_Alignment(void) const { return mAlignment; }

	// Set the caption text
	void Set_Caption(unichar_t* string);

	// Retrieve the caption text
	const unichar_t* Get_Caption(void) const { return mCaption; }

private:
	unsigned int mTimeStamp;
	unsigned int mDuration;
	unsigned int mRGBColor;
	int mLinePosition;
	Alignment mAlignment;
	unichar_t* mCaption;
};

#endif // _SUBTITLE_H_
