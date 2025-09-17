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
*     $Archive: /Commando/Code/BinkMovie/subtitleparser.cpp $
*
* DESCRIPTION
*     Subtitling control file parser
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Denzil_l $
*     $Modtime: 1/12/02 9:27p $
*     $Revision: 2 $
*
****************************************************************************/

#include "subtitleparser.h"
#include "subtitle.h"
#include "straw.h"
#include "readline.h"
#include "trim.h"
#include <wchar.h>
#include <stdlib.h>
#include "wwlib/widestring.h"

// Subtitle control file parsing tokens
#define BEGINMOVIE_TOKEN L"BeginMovie"
#define ENDMOVIE_TOKEN   L"EndMovie"
#define TIMEBIAS_TOKEN   L"TimeBias"
#define TIME_TOKEN       L"Time"
#define DURATION_TOKEN   L"Duration"
#define POSITION_TOKEN   L"Position"
#define COLOR_TOKEN      L"Color"
#define TEXT_TOKEN       L"Text"

unsigned long DecodeTimeString(wchar_t* string);
void Parse_Time(wchar_t* string, SubTitleClass* subTitle);
void Parse_Duration(wchar_t* string, SubTitleClass* subTitle);
void Parse_Position(wchar_t* string, SubTitleClass* subTitle);
void Parse_Color(wchar_t* string, SubTitleClass* subTitle);
void Parse_Text(wchar_t* string, SubTitleClass* subTitle);

SubTitleParserClass::TokenHook SubTitleParserClass::mTokenHooks[] =
{
	{TIME_TOKEN, Parse_Time},
	{DURATION_TOKEN, Parse_Duration},
	{POSITION_TOKEN, Parse_Position},
	{COLOR_TOKEN, Parse_Color},
	{TEXT_TOKEN, Parse_Text},
	{NULL, NULL}
};

/******************************************************************************
*
* NAME
*     SubTitleParserClass::SubTitleParserClass
*
* DESCRIPTION
*
* INPUTS
*     Input - Control file input stream.
*
* RESULTS
*     NONE
*
******************************************************************************/

SubTitleParserClass::SubTitleParserClass(Straw& input)
	:
	mInput(input),
	mLineNumber(0)
{
	// Check for Unicode byte-order mark.
	// All Unicode plaintext files are prefixed with the byte-order mark U+FEFF
	// or its mirror U+FFFE. This mark is  used to indicate the byte order of a
	// text stream.
	wchar_t byteOrderMark = 0;
	mInput.Get(&byteOrderMark, sizeof(wchar_t));
	WWASSERT(byteOrderMark == 0xFEFF);

	if (byteOrderMark != 0xFEFF) {
		WWDEBUG_SAY(("Error: Subtitle control file is not unicode!\n"));
	}
}


/******************************************************************************
*
* NAME
*     SubTitleParserClass::~SubTitleParserClass
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

SubTitleParserClass::~SubTitleParserClass()
{
}


/******************************************************************************
*
* NAME
*     SubTitleParserClass::GetSubTitles
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULTS
*
******************************************************************************/

DynamicVectorClass<SubTitleClass*>* SubTitleParserClass::Get_Sub_Titles(const char* moviename)
{
	DynamicVectorClass<SubTitleClass*>* subTitleCollection = NULL;
	
	// Find the movie marker
	if (Find_Movie_Entry(moviename) == true)	{
		// Allocate container to hold subtitles
		subTitleCollection = new DynamicVectorClass<SubTitleClass*>;
		WWASSERT(subTitleCollection != NULL);

		if (subTitleCollection != NULL) {
			for (;;) {
				// Retrieve a line from the control file
				wchar_t* string = Get_Next_Line();

				if ((string != NULL) && (wcslen(string) > 0)) {
					// Check for subtitle entry markers
					if ((string[0] == L'<') && (string[wcslen(string) - 1] == L'>')) {
						// Trim off markers
						string++;
						string[wcslen(string) - 1] = 0;
						wcstrim(string);

						// Ignore empty caption
						if (wcslen(string) == 0) {
							continue;
						}

						// Create a new SubTitleClass
						SubTitleClass* subTitle = new SubTitleClass();
						WWASSERT(subTitle != NULL);

						if (subTitle == NULL) {
							WWDEBUG_SAY(("***** Failed to create SubTitleClass!\n"));
							break;
						}

						if (Parse_Sub_Title(string, subTitle) == true)	{
							subTitleCollection->Add(subTitle);
						}
						else {
							delete subTitle;
						}

						continue;
					}

					// Terminate if end movie token encountered.
                    if (openw3d::widestring_len_compare_case(string, ENDMOVIE_TOKEN, wcslen(ENDMOVIE_TOKEN)) == 0) {
						break;
					}
				}
			}

			if (subTitleCollection->Count() == 0) {
				delete subTitleCollection;
				subTitleCollection = NULL;
			}
		}
	}

	return subTitleCollection;
}


/******************************************************************************
*
* NAME
*     SubTitleParserClass::FindMovieEntry
*
* DESCRIPTION
*     No description provided,
*
* INPUTS
*     Moviename - Pointer to name of movie to find subtitles for.
*
* RESULTS
*     Success - True if movie entry found; False if unable to find movie entry.
*
******************************************************************************/

bool SubTitleParserClass::Find_Movie_Entry(const char* moviename)
{
	// Convert the moviename into Unicode
	WWASSERT(moviename != NULL);
	wchar_t wideName[32];
	mbstowcs(wideName, moviename, 32);

	do {
		// Retrieve line of text
		wchar_t* string = Get_Next_Line();

		// Terminate if no string read.
		if (string == NULL) {
			break;
		}

		// Look for begin movie token
        if (openw3d::widestring_len_compare_case(string, BEGINMOVIE_TOKEN, wcslen(BEGINMOVIE_TOKEN)) == 0) {
			// Get moviename following the token
			wchar_t* ptr = wcschr(string, L' ');

			// Check for matching moviename
			if (ptr != NULL) {
				wcstrim(ptr);

                if (openw3d::widestring_compare_case(ptr, wideName) == 0) {
					WWDEBUG_SAY(("Found movie entry %s\n", moviename));
					return true;
				}
			}
		}
	} while (true);

	return false;
}


/******************************************************************************
*
* NAME
*     SubTitleParserClass::ParseSubTitle
*
* DESCRIPTION
*
* INPUTS
*     wchar_t* string
*     SubTitleClass* subTitle
*
* RESULTS
*     bool
*
******************************************************************************/

bool SubTitleParserClass::Parse_Sub_Title(wchar_t* string, SubTitleClass* subTitle)
{
	// OpenW3D @fix cfehunter 07/03/2025
	// Empty char to assign an empty string to the mutable pointer
	wchar_t empty = L'\0';

	// Parameter check
	WWASSERT(string != NULL);
	WWASSERT(subTitle != NULL);

	for (;;) {
		// Find token separator
		wchar_t* separator = wcschr(string, L'=');

		if (separator == NULL) {
			WWDEBUG_SAY(("Error on line %d: syntax error\n", Get_Line_Number()));
			return false;
		}

		// NULL terminate token part
		*separator++ = 0;

		// Tokens are to the left of the separator
		wchar_t* token = string;
		wcstrim(token);

		// Parameters are to the right of the separator
		wchar_t* param = separator;
		wcstrim(param);

		// Quoted parameters are treated as literals (ignore contents)
		if (param[0] == L'"') {
			// Skip leading quote
			param++;

			// Use next quote to mark end of parameter
			separator = wcschr(param, L'"');

			if (separator == NULL) {
				WWDEBUG_SAY(("Error on line %d: mismatched quotes\n", Get_Line_Number()));
				return false;
			}

			// NULL terminate parameter
			*separator++ = 0;

			// Skip any comma following a literal string since we used the trailing
			// quote to terminate the tokens parameters
			wcstrim(separator);
			
			if (*separator == L',') {
				separator++;
			}

			// Advance string past quoted parameter
			string = separator;
		}
		else {
			// Look for separator to next token
			separator = wcspbrk(param, L", ");

			if (separator != NULL) {
				*separator++ = 0;
				string = separator;
			}
			else {
				string = &empty;
			}
		}

		// Error on empty tokens
		if (wcslen(token) == 0) {
			WWDEBUG_SAY(("Error on line %d: missing token\n", Get_Line_Number()));
			return false;
		}

		// Parse current token
		Parse_Token(token, param, subTitle);

		// Prepare for next token
		wcstrim(string);

		if (wcslen(string) == 0) {
			break;
		}
	}

	return true;
}


/******************************************************************************
*
* NAME
*     SubTitleParserClass::ParseToken
*
* DESCRIPTION
*
* INPUTS
*     wchar_t* token
*     wchar_t* param
*     SubTitleClass* subTitle
*
* RESULTS
*     NONE
*
******************************************************************************/

void SubTitleParserClass::Parse_Token(wchar_t* token, wchar_t* param, SubTitleClass* subTitle)
{
	// Parameter check
	WWASSERT(token != NULL);
	WWASSERT(subTitle != NULL);

	if (token != NULL) {
		int index = 0;

		while (mTokenHooks[index].Token != NULL) {
			TokenHook& hook = mTokenHooks[index];

            if (openw3d::widestring_compare_case(hook.Token, token) == 0) {
				WWASSERT(subTitle != NULL);
				hook.Handler(param, subTitle);
				return;
			}

			index++;
		}
	}
}


/******************************************************************************
*
* NAME
*     SubTitleParserClass::GetNextLine
*
* DESCRIPTION
*     Retrieve the next line of text from the control file.
*
* INPUTS
*     NONE
*
* RESULTS
*     String - Pointer to next line of text. NULL if error or EOF.
*
******************************************************************************/

wchar_t* SubTitleParserClass::Get_Next_Line(void)
{
	bool eof = false;

	while (eof == false) {
		// Read in a line of text
		Read_Line(mInput, mBuffer, LINE_MAX, eof);
		mLineNumber++;

		// Remove whitespace
		wchar_t* string = wcstrim(mBuffer);

		// Skip comments and blank lines
		if ((wcslen(string) > 0) && (string[0] != L';')) {
			return string;
		}
	}

	return NULL;
}


// Convert a time string in the format hh:mm:ss:tt into 1/60 second ticks.
unsigned long Decode_Time_String(wchar_t* string)
{
	#define TICKS_PER_SECOND 60
	#define TICKS_PER_MINUTE (60 * TICKS_PER_SECOND)
	#define TICKS_PER_HOUR   (60 * TICKS_PER_MINUTE)

	WWASSERT(string != NULL);

	wchar_t buffer[12];
	wcsncpy(buffer, string, 12);
	buffer[11] = 0;

	wchar_t* ptr = &buffer[0];

	// Isolate hours part
	wchar_t* separator = wcschr(ptr, L':');
	WWASSERT(separator != NULL);
	*separator++ = 0;
	unsigned long hours = wcstoul(ptr, NULL, 10);

	// Isolate minutes part
	ptr = separator;
	separator = wcschr(ptr, L':');
	WWASSERT(separator != NULL);
	*separator++ = 0;
	unsigned long minutes = wcstoul(ptr, NULL, 10);

	// Isolate seconds part
	ptr = separator;
	separator = wcschr(ptr, L':');
	WWASSERT(separator != NULL);
	*separator++ = 0;
	unsigned long seconds = wcstoul(ptr, NULL, 10);

	// Isolate hundredth part (1/100th of a second)
	ptr = separator;
	unsigned long hundredth = wcstoul(ptr, NULL, 10);

	unsigned long time = (hours * TICKS_PER_HOUR);
	time += (minutes * TICKS_PER_MINUTE);
	time += (seconds * TICKS_PER_SECOND);
	time += ((hundredth * TICKS_PER_SECOND) / 100);

	return time;
}


void Parse_Time(wchar_t* param, SubTitleClass* subTitle)
{
	WWASSERT(param != NULL);
	WWASSERT(subTitle != NULL);
	unsigned long time = Decode_Time_String(param);
	subTitle->Set_Display_Time(time);
}


void Parse_Duration(wchar_t* param, SubTitleClass* subTitle)
{
	WWASSERT(param != NULL);
	WWASSERT(subTitle != NULL);
	unsigned long time = Decode_Time_String(param);

	if (time > 0) {
		subTitle->Set_Display_Duration(time);
	}
}


void Parse_Position(wchar_t* param, SubTitleClass* subTitle)
{
	static struct
	{
		const wchar_t* Name;
		SubTitleClass::Alignment Align;
		} _alignLookup[] = {
			{L"Left", SubTitleClass::Left},
			{L"Right", SubTitleClass::Right},
			{L"Center", SubTitleClass::Center},
			{NULL, SubTitleClass::Center}
	};

	WWASSERT(subTitle != NULL);
	WWASSERT(param != NULL);

	wchar_t* ptr = param;

	// Line position
	wchar_t* separator = wcschr(ptr, L':');

	if (separator != NULL) {
		*separator++ = 0;
		int linePos = wcstol(ptr, NULL, 0);
		subTitle->Set_Line_Position(linePos);
		ptr = separator;
	}

	// Justification
	SubTitleClass::Alignment align = SubTitleClass::Center;
	int index = 0;

	while (_alignLookup[index].Name != NULL) {
        if (openw3d::widestring_compare_case(ptr, _alignLookup[index].Name) == 0) {
			align = _alignLookup[index].Align;
			break;
		}

		index++;
	}

	subTitle->Set_Alignment(align);
}


void Parse_Color(wchar_t* param, SubTitleClass* subTitle)
{
	WWASSERT(param != NULL);
	WWASSERT(subTitle != NULL);

	wchar_t* ptr = param;

	wchar_t* separator = wcschr(ptr, L':');
	*separator++ = 0;
	unsigned char red = (unsigned char)wcstoul(ptr, NULL, 10);
	
	ptr = separator;
	separator = wcschr(ptr, L':');
	*separator++ = 0;
	unsigned char green = (unsigned char)wcstoul(ptr, NULL, 10);

	ptr = separator;
	unsigned char blue = (unsigned char)wcstoul(ptr, NULL, 10);

	subTitle->Set_RGB_Color(red, green, blue);
}


void Parse_Text(wchar_t* param, SubTitleClass* subTitle)
{
	WWASSERT(param != NULL);
	WWASSERT(subTitle != NULL);

	subTitle->Set_Caption(param);
}
