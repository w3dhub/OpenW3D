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
*     $Archive: /Commando/Code/BinkMovie/subtitlemanager.cpp $
*
* DESCRIPTION
*     Subtitling manager
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Denzil_l $
*     $Modtime: 1/24/02 10:11a $
*     $Revision: 3 $
*
****************************************************************************/

#include "always.h"
#include "subtitlemanager.h"
#include "subtitleparser.h"
#include "subtitle.h"
#include "xstraw.h"
#include "rawfile.h"
#include "assetmgr.h"
#include "ww3d.h"
#include <stdlib.h>


/******************************************************************************
*
* NAME
*     SubTitleManagerClass::Create
*
* DESCRIPTION
*     Instantiate a subtitle manager for the specified movie.
*
* INPUTS
*     Filename  - Name of movie file to create subtitle manager for.
*
* RESULTS
*     NONE
*
******************************************************************************/

SubTitleManagerClass* SubTitleManagerClass::Create(const char* filename, const char* subtitlefilename, FontCharsClass* font)
{
	if ((filename == NULL) || (strlen(filename) == 0)) {
		return NULL;
	}

	if (subtitlefilename && !font) {
		return NULL;
	}

	// Create subtitle manager for the vqa
	SubTitleManagerClass* instance = new SubTitleManagerClass();
	WWASSERT(instance != NULL);

	if (instance != NULL) {
		instance->Set_Font(font);

		// Retrieve moviename
		char fname[_MAX_FNAME];
		_splitpath(filename, NULL, NULL, fname, NULL);
		bool loaded = instance->Load_Sub_Titles(fname, subtitlefilename);

		if (loaded == false) {
			delete instance;
			return NULL;
		}
	}

	return instance;
}


/******************************************************************************
*
* NAME
*     SubTitleManagerClass::SubTitleManagerClass
*
* DESCRIPTION
*
* INPUTS
*
* RESULTS
*     NONE
*
******************************************************************************/

SubTitleManagerClass::SubTitleManagerClass()
	:
	mSubTitles(NULL),
	mSubTitleIndex(0),
	mActiveSubTitle(NULL)
{
}


/******************************************************************************
*
* NAME
*     SubTitleManagerClass::~SubTitleManagerClass
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

SubTitleManagerClass::~SubTitleManagerClass()
{
	// Release subtitle entries
	if (mSubTitles != NULL) {
		for (int index = 0; index < mSubTitles->Count(); index++) {
			delete (*mSubTitles)[index];
		}

		delete mSubTitles;
	}
}


void SubTitleManagerClass::Set_Font(FontCharsClass* font)
{
	if (font) {
		Renderer.Set_Font(font);
	}
}


/******************************************************************************
*
* NAME
*     SubTitleManagerClass::LoadSubTitles
*
* DESCRIPTION
*
* INPUTS
*     Moviename - Pointer to movie name
*
* RESULTS
*     Success - 
*
******************************************************************************/

bool SubTitleManagerClass::Load_Sub_Titles(const char* moviename, const char* subtitlefilename)
{
	if ((moviename == NULL) || (strlen(moviename) == 0)) {
		return false;
	}

	if ((subtitlefilename == NULL) || (strlen(subtitlefilename) == 0)) {
		return false;
	}

	RawFileClass file(subtitlefilename);

	if (!file.Is_Available()) {
		return false;
	}

 	FileStraw input(file);
 	SubTitleParserClass parser(input);
 	
 	mSubTitles = parser.Get_Sub_Titles(moviename);

 	if (mSubTitles == NULL) {
 		return false;
	}

	// TODO: Make sure entries are sorted by time.

	return true;
}


/******************************************************************************
*
* NAME
*     SubTitleManagerClass::Process
*
* DESCRIPTION
*     Handle subtitle processing. This must be called each frame advance.
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

bool SubTitleManagerClass::Process(unsigned long movieTime)
{
	if (mSubTitles == NULL) {
		return false;
	}

	bool update = false;

	for (;;) {
		// Terminate if there aren't more subtitles
		if (mSubTitleIndex >= mSubTitles->Count()) {
			break;
		}

		// Get the next subtitle
		SubTitleClass* subtitle = (*mSubTitles)[mSubTitleIndex];
		WWASSERT(subtitle != NULL);

		// Check the display time against the current movie time. If it is time
		// to display the subtitle then send a subtitle event to the client.
		unsigned long displayTime = subtitle->Get_Display_Time();

		// If its not time then we are done.
		if (displayTime > movieTime) {
			break;
		}	else {
			// Make this subtitle the active one
			mActiveSubTitle = subtitle;

			// Advance to the next subtitle entry
			mSubTitleIndex++;

			Draw_Sub_Title(subtitle);
			update = true;

//			WWDEBUG_SAY(("SubTitle: %04d @ %u\n", mSubTitleIndex, movieTime));
		}
	}

	// If the active subtitles duration has expired then remove it as being active.
	if (mActiveSubTitle != NULL) {
		SubTitleClass* subtitle = mActiveSubTitle;
		unsigned long expireTime = subtitle->Get_Display_Time() + subtitle->Get_Display_Duration();

		if (movieTime >= expireTime) {
			mActiveSubTitle = NULL;

			// Erase subtitle
			Renderer.Reset();
			update = true;
		}
	}

	return update;
}


/******************************************************************************
*
* NAME
*     SubTitleManagerClass::Reset
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

void SubTitleManagerClass::Reset(void)
{
	mSubTitleIndex = 0;
	mActiveSubTitle = NULL;
}


/******************************************************************************
*
* NAME
*     SubTitleManagerClass::DrawSubTitle
*
* DESCRIPTION
*
* INPUTS
*     const SubTitleClass* subtitle
*
* RESULTS
*     NONE
*
******************************************************************************/

void SubTitleManagerClass::Draw_Sub_Title(const SubTitleClass* subtitle)
{
	WWASSERT(subtitle != NULL);

	Renderer.Reset();

	const wchar_t* string = subtitle->Get_Caption();

	int w,h,bits;
	bool windowed;
	WW3D::Get_Device_Resolution(w,h,bits,windowed);
	Vector2 extents=Renderer.Get_Text_Extents( string );

	// Assume left justification
	int xPos = 0;
	int yPos = subtitle->Get_Line_Position() * (h/16);
	int xSize=extents[0];

	SubTitleClass::Alignment align = subtitle->Get_Alignment();

	if (align == SubTitleClass::Center)	{
		xPos = ((w - xSize) / 2);
	}
	else if (align == SubTitleClass::Right) {
		xPos = (w - xSize);
	}

	Renderer.Set_Location(Vector2(xPos,yPos));
	Renderer.Build_Sentence(string);

	// Set font color
	unsigned long rgbColor = subtitle->Get_RGB_Color()|0xff000000;

	Renderer.Draw_Sentence(rgbColor);
}


void SubTitleManagerClass::Render()
{
	Renderer.Render();
}
