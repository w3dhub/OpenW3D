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

#include <string.h>
#include <stdlib.h>
#include <wwdebug.h>
#include "viseme.h"
#include <ctype.h>
#include "wwlib/wwstring.h"

#define IS_VOWEL(x)			( x && (x=='a' || x=='e' || x=='i' || x=='o' || x=='u') )
#define IS_CONSONANT(x)		( x && !IS_VOWEL(x) )

struct VisemeTableItem
{
	const char	*LetterCombination;
	char	Visemes[2];
};

static VisemeTableItem gsVisemeTable[] =
{
	{"air",		VISEME_IF, -1},
	{"ar",		VISEME_OX, -1},
	{"ch",		VISEME_CHURCH, -1},
	{"ea",		VISEME_EAT, -1},
	{"ee",		VISEME_EAT, -1},
	{"er",		VISEME_EARTH, -1},
	{"oa",		VISEME_OX, -1},
	{"oo",		VISEME_WET, -1},
	{"ou",		VISEME_OX, VISEME_WET},
	{"ow",		VISEME_WET},
	{"qu",		VISEME_WET, -1},
	{"sch",		VISEME_SIZE,	VISEME_CAGE},
	{"sh",		VISEME_CHURCH, -1},
	{"tch",		VISEME_CHURCH, -1},
	{"th",		VISEME_THOUGH, -1},
	{"gn",		VISEME_NEW, -1},
	{"kn",		VISEME_NEW, -1},
	{"eye",		VISEME_IF, -1},
	{"uy",		VISEME_IF, -1},
	{"ar",		VISEME_OX, -1},
	{"kw",		VISEME_WET, -1},
	{"and",		VISEME_NEW, VISEME_TOLD},
	{"ze",		VISEME_ROAR, VISEME_EAT},		// numbers: zero to nine
	{"ro",		VISEME_WET, VISEME_OX},
	{"one",		VISEME_WET, VISEME_CAT},		
	{"two",		VISEME_EAT, VISEME_WET},
	{"thr",		VISEME_THOUGH, VISEME_ROAR},
	{"fou",		VISEME_FAVE, VISEME_OX},
	{"ive",		VISEME_IF, VISEME_FAVE},
	{"six",		VISEME_UP, VISEME_IF},
	{"se",		VISEME_UP, VISEME_EAT},
	{"ven",		VISEME_FAVE, VISEME_EAT},
	{"eight",	VISEME_CAT, VISEME_EAT},
	{"ni",		VISEME_THOUGH, VISEME_IF},
	{"ne",		VISEME_THOUGH, -1},				// end numbers
	
};

struct VisemeTableReferenceItem
{
	int			StartIndex;
	int			Count;
};

#define NUM_VISEME_REFERENCES	26								// 'a' - 'z'
static VisemeTableReferenceItem VisemeReferenceTable[NUM_VISEME_REFERENCES];

//======================================================================================
static int CompareVisemeTableItems( const void *arg1, const void *arg2 )
{
	VisemeTableItem *p1 = (VisemeTableItem *)arg1;
	VisemeTableItem *p2 = (VisemeTableItem *)arg2;

	return strcmp(p1->LetterCombination, p2->LetterCombination);
}

//======================================================================================
VisemeManager::VisemeManager(void)
{
	// sort viseme lookup table
	int numVisemeTableItems = sizeof(gsVisemeTable) / sizeof(VisemeTableItem);
	qsort(gsVisemeTable, numVisemeTableItems, sizeof(VisemeTableItem), CompareVisemeTableItems);

	// build viseme lookup reference table
	memset(VisemeReferenceTable, 0, sizeof(VisemeReferenceTable));
	VisemeTableItem *pItem = gsVisemeTable;

	for (int i=0; i<numVisemeTableItems; i++,pItem++) {
		int index = (int)tolower(pItem->LetterCombination[0]) - 'a';

		if ( index >= 0 && index < NUM_VISEME_REFERENCES ) {
			VisemeTableReferenceItem *pR = &VisemeReferenceTable[index];
			if ( pR->Count == 0 ) {
				pR->StartIndex = i;
			}
			pR->Count++;
		}
	}
}

//======================================================================================
// GetVisemes(const char *word, int *visemelist, int maxvisemes)
// Input:	word		- word to parse
//			maxvisemes	- max. number of visemes on the output list visemelist
// Output:	visemelist	- contains viseme ID's
// Remark:	this routine examines word and identifies visemes and place them 
//			on the given output list(visemelist)
// Return:	number of visemes on the output list
//
int VisemeManager::Get_Visemes(const char *word, int *visemelist, int maxvisemes) const
{
	char prevchar = 0;
	int last_viseme = -1;
	int viseme[2];
	int i;
	int offset;
	int num_visemes = 0;
	char local_buf[128];

	// make a local copy of the word in lower case
	strncpy(local_buf, word, sizeof(local_buf)-1);
	local_buf[sizeof(local_buf)-1] = 0;
    openw3d::string_to_lower(local_buf);

	const char *pchar = local_buf;
	while ( *pchar ) {
		// check for tabled viseme combinations
		offset = Lookup(pchar, word, viseme);
		
		if ( offset == 0 ) {
			// analyse the current letter

			// init default viseme ID
			viseme[0] = VISEME_TOLD;
			viseme[1] = -1;
			offset = 1;

			switch (*pchar) {
			case 'a':
				offset = Do_Letter_a(pchar, prevchar, viseme);
				break;
			case 'b':
				viseme[0] = VISEME_BUMP;
				break;
			case 'c':
				viseme[0] = VISEME_CAGE;
				break;
			case 'd':
				viseme[0] = VISEME_TOLD;
				break;
			case 'e':
				offset = Do_Letter_e(pchar, prevchar, viseme);
				break;
			case 'f':
				viseme[0] = VISEME_FAVE;
				break;
			case 'g':
				viseme[0] = VISEME_CAGE;
				break;
			case 'h':
				// no sound
				break;
			case 'i':
				offset = Do_Letter_i(pchar, prevchar, viseme);
				break;
			case 'j':
				viseme[0] = VISEME_CHURCH;
				break;
			case 'k':
				viseme[0] = VISEME_CAGE;
				break;
			case 'l':
				viseme[0] = VISEME_THOUGH;
				break;
			case 'm':
				viseme[0] = VISEME_BUMP;
				break;
			case 'n':
				viseme[0] = VISEME_NEW;
				break;
			case 'o':
				offset = Do_Letter_o(pchar, prevchar, viseme);
				break;
			case 'p':
				viseme[0] = VISEME_BUMP;
				break;
			case 'q':
				viseme[0] = VISEME_CAGE;
				break;
			case 'r':
				viseme[0] = VISEME_ROAR;
				break;
			case 's':
				offset = Do_Letter_s(pchar, prevchar, viseme);
				break;
			case 't':
				offset = Do_Letter_t(pchar, prevchar, viseme);
				break;
			case 'u':
				offset = Do_Letter_u(pchar, prevchar, viseme);
				break;
			case 'v':
				viseme[0] = VISEME_FAVE;
				break;
			case 'w':
				viseme[0] = VISEME_WET;
				break;
			case 'x':
				viseme[0] = VISEME_CAGE;
				viseme[1] = VISEME_SIZE;
				break;
			case 'y':
				// geo-aug22/00 no viseme reference
				//viseme[0] = VISEME_WET;
				break;
			case 'z':
				viseme[0] = VISEME_SIZE;
				break;
			}
		}

		// add the newly found visemes to the list
		for (i=0; i<2; i++) {
			if ( viseme[i] >= 0 && viseme[i] != last_viseme ) {
				*visemelist++ = viseme[i];
				last_viseme = viseme[i];

				// check for list limitation
				if ( ++num_visemes >= maxvisemes ) {
					// limit reached
					return(num_visemes);
				}
			}
		}

		// next letter
		pchar += offset;
		prevchar = *(pchar - 1);
	}

	return(num_visemes);
}

//======================================================================================
// return: number of bytes to jump over when done
//
int VisemeManager::Lookup(const char *pchar, const char * /*word*/, int viseme[]) const
{
	int length = 0;

	char ch = (char)tolower(*pchar);
	int index = ch - 'a';

	if ( index < 0 || index >= NUM_VISEME_REFERENCES )	{
		// out of range
		return 0;
	}

	VisemeTableReferenceItem *pR = &VisemeReferenceTable[index];
	// we search backwards so that we can find the max. match first
	VisemeTableItem *pI = &gsVisemeTable[pR->StartIndex + pR->Count - 1];
	for (int i=0; i<pR->Count; i++,pI--) {
		length = strlen(pI->LetterCombination);
        if ( openw3d::strnicmp(pchar, pI->LetterCombination, length) == 0 )
		{
			// found!
			viseme[0] = pI->Visemes[0];
			viseme[1] = pI->Visemes[1];
			return length;
		}
	}

	return 0;
}

//======================================================================================
// return: number of bytes to jump over when done
//
int VisemeManager::Do_Letter_a(const char *pchar, char prevchar, int *viseme) const
{
	int offset = 1;								// default

	// default as in "a" or "about"
	viseme[0] = VISEME_TOLD;
	viseme[1] = -1;

	if ( IS_VOWEL(pchar[1]) )	{
		offset++;
		switch (pchar[1])	{
		case 'i':	// ai
			viseme[1] = VISEME_EAT;			// default: e.g. plaid, aim, maize, train
			if ( pchar[2] == 's' ) {
				// as in "aisle"
				viseme[0] = VISEME_CAT;
			}
			break;
		case 'o':	// ao
			// as in "gaol"
			viseme[1] = VISEME_EAT;
			break;
		case 'u':	// au
			switch (prevchar)	{
			case 'g':	// as in "gauge"
				viseme[1] = VISEME_EAT;
				break;
			case 'l':	// as in "laugh"
				viseme[0] = VISEME_CAT;
				break;
			case 's':	// as in "sauerkraut"
				viseme[0] = VISEME_CAT;
				viseme[1] = VISEME_WET;
				break;
			default:	// as in "daughter"
				viseme[0] = VISEME_CAT;
			}
			break;
		}
	}
	else	{
		if ( IS_VOWEL(pchar[2]) ) {
			// e.g. "ate", 12+2
			viseme[1] = VISEME_EAT;
		}
		else {
			// e.g. "add", 1
			viseme[0] = VISEME_CAT;
		}
	}

	return(offset);
}

//======================================================================================
// return: number of bytes to jump over when done
//
int VisemeManager::Do_Letter_e(const char *pchar, char prevchar, int *viseme) const
{
	int offset = 1;

	// default as in "e" or "be"
	viseme[0] = VISEME_EAT;						// default, as in "beach"
	viseme[1] = -1;

	if ( IS_VOWEL(pchar[1]) ) {
		offset++;
		//viseme[0] = VISEME_EAT;					// default, as in "beach"

		switch ( pchar[1] ) {
		case 'a':	// ea
			if ( pchar[2] == 'u' ) {
				// as in "beauty"
				viseme[0] = VISEME_EAT;
				viseme[1] = VISEME_WET;
				offset++;
			}
			break;
		case 'i':	// ei
			if ( prevchar == 'h' ) {
				// as in "height"
				viseme[0] = VISEME_CAT;
				viseme[1] = VISEME_EAT;
			}
			else if ( pchar[2] == 'g' ) {
				// as in "eight", "reign"
				viseme[0] = VISEME_TOLD;
				viseme[1] = VISEME_EAT;
			}
			else if ( pchar[2] == 'z' ) {
				// as in "seize"
				viseme[0] = VISEME_EAT;
			}
			else {// plain old "ei"
				// default:
				viseme[0] = VISEME_IF;
			}
			break;
		case 'o':	// eo
			if ( prevchar == 'p' ) {
				// as in "people"
				viseme[0] = VISEME_EAT;
			}
			else {
				// default:
				viseme[0] = VISEME_TOLD;
			}
			break;
		case 'u':	// eu
			// as in "maneuver", "eulogy", "queue"
			viseme[0] = VISEME_WET;
			break;
		}
	}
	else {
		if ( prevchar == 'b' || pchar[1] == 0 ) {
			// as in "be"
			viseme[0] = VISEME_EAT;
		}
		else {
			// default as in "pet",
			viseme[0] = VISEME_WET;
		}
	}

	return(offset);
}

//======================================================================================
// return: number of bytes to jump over when done
//
int VisemeManager::Do_Letter_i(const char *pchar, char prevchar, int *viseme) const
{
	int offset = 1;

	// default as in "I"
	viseme[0] = VISEME_IF;
	viseme[1] = -1;

	if ( IS_VOWEL(pchar[1]) ) {
		offset++;
		viseme[0] = VISEME_TOLD;					// default,
		viseme[1] = -1;

		switch ( pchar[1] ) {
		case 'e':	// ie
			if ( pchar[2] == 0 && prevchar == 'l' ) {
				// as in "lie"
				viseme[0] = VISEME_CAT;
				viseme[1] = VISEME_EAT;
			}
			else if ( pchar[2] == 'u' || pchar[2] == 'w' ) {
				viseme[0] = VISEME_WET;			// as in lieutenant or view
				offset++;
			}
			else {
				viseme[0] = VISEME_EAT;			// as in grief
			}
			break;
		case 'o':	// io
			viseme[0] = VISEME_EAT;				// as in onion
			break;
		}
	}
	else {
		viseme[0] = VISEME_IF;					// as in "pig"
		viseme[1] = -1;
	}

	return(offset);
}

//======================================================================================
// return: number of bytes to jump over when done
//
int VisemeManager::Do_Letter_o(const char *pchar, char /*prevchar*/, int *viseme) const
{
	int offset = 1;

	// default as in "no"
	viseme[0] = VISEME_OX;
	viseme[1] = -1;
	// geo-modified aug21/00
	//viseme[1] = VISEME_WET;

	if ( IS_VOWEL(pchar[1]) ) {
		offset++;
		viseme[0] = VISEME_UP;					// default, as in "took"
		viseme[1] = -1;

		switch ( pchar[1] ) {
		case 'a':	// oa
			if ( pchar[-2] == 'b' && pchar[-1] == 'r' ) {
				viseme[0] = VISEME_UP;				// as in "broad"
			}
			else {
				viseme[0] = VISEME_OX;				// as in "loan"
				viseme[1] = VISEME_WET;
			}
			break;
		case 'e':	// oe
			viseme[0] = VISEME_WET;					// default as in "canoe", "foe"
			if ( pchar[-1] == 'd' && pchar[2] == 's' ) {
				viseme[0] = VISEME_UP;				// as in "does"
			}
			else if ( pchar[-2] == 'p' && pchar[-1] == 'h' ) {
				viseme[0] = VISEME_EAT;				// as in "phoenix"
			}
			break;
		case 'i':	// oi
			viseme[0] = VISEME_UP;					// as in "noise"
			viseme[1] = VISEME_EAT;
			break;
		case 'o':	// oo
			// GEO-modified aug21/00
			viseme[0] = VISEME_WET;					// default as in "took", "look", "book"
			break;
		case 'u':	// ou
			viseme[0] = VISEME_WET;					// default as in "through", "though","croup"

			if ( pchar[-2] == 't' && pchar[-1] == 'r' ) {
				// as in "trouble", "trough"
				viseme[0] = VISEME_UP;
			}
			else if ( pchar[-2] == 't' && pchar[-1] == 'h' ) {
				if ( strncmp(&pchar[2], "ght", 3) == 0 ) {
					viseme[0] = VISEME_UP;			// as in "thought"
					offset += 3;
				}
			}
			else if ( pchar[2] == 'r' ) {
				viseme[0] = VISEME_TOLD;				// as in "journey"
				offset++;
			}
			break;
		}
	}
	else if ( pchar[1] ) {
		if ( IS_VOWEL(pchar[2]) ) {
			viseme[0] = VISEME_WET;					// as in "move"
			viseme[1] = -1;
		}
		else {
			viseme[0] = VISEME_OX;					// as in "comment"
			viseme[1] = -1;
		}
	}

	return(offset);
}

//======================================================================================
// return: number of bytes to jump over when done
//
int VisemeManager::Do_Letter_s(const char *pchar, char /*prevchar*/, int *viseme) const
{
	int offset = 1;

	// default as in "Say", "yourS"
	//viseme[0] = VISEME_TOLD;
	viseme[0] = VISEME_SIZE;
	viseme[1] = -1;

	// special cases
	if ( pchar[1] == 'h' || pchar[1] == 'u' )	{
		viseme[0] = VISEME_CHURCH;			// e.g. "ship", "sure", "shoot"
	}
	else if ( pchar[1] == 'c' ) {
		if ( pchar[2] == 'h' ) {
			viseme[0] = VISEME_CHURCH;		// e.g. "school"
			viseme[1] = VISEME_TOLD;
			offset = 3;
		}
		else {
			viseme[0] = VISEME_TOLD;		// e.g. "scent"
			offset = 2;
		}
	}
	else if ( strncmp(&pchar[1], "eou", 3) == 0 ) {
		viseme[0] = VISEME_CHURCH;			// e.g. "nauseous"
		offset = 4;
	}

	return(offset);
}

//======================================================================================
// return: number of bytes to jump over when done
//
int VisemeManager::Do_Letter_t(const char *pchar, char /*prevchar*/, int *viseme) const
{
	int offset = 1;

	// default as in "t"
	viseme[0] = VISEME_TOLD;

	// special cases
	if ( pchar[1] == 'c' ) {
		if ( pchar[2] == 'h' ) {
			viseme[0] = VISEME_CHURCH;		// e.g. "catch"
			offset = 3;
		}
	}
	else if ( pchar[1] == 'h' ) {
		viseme[0] = VISEME_THOUGH;		// e.g. "this","thin", "through", "then" 
		offset = 2;						// except for "thomas"
	}

	return(offset);
}

//======================================================================================
// return: number of bytes to jump over when done
//
int VisemeManager::Do_Letter_u(const char *pchar, char /*prevchar*/, int *viseme) const
{
	int offset = 1;

	// default as in "u" or "use"
	//viseme[0] = VISEME_EAT;
	//viseme[1] = VISEME_WET;
	// geo-modified aug21/00
	viseme[0] = VISEME_UP;

	if ( IS_VOWEL(pchar[1]) ) {
		offset++;

		switch ( pchar[1] ) {
		case 'e':	// ue
		case 'i':	// ui
			viseme[0] = VISEME_WET;			// as in "blue" or "fruit"
			viseme[1] = -1;
			break;
		}
	}
	else {
		if ( IS_CONSONANT(pchar[2]) )	{
			viseme[0] = VISEME_UP;			// as in "utter"
			viseme[1] = -1;
		}
	}

	return(offset);
}

//======================================================================================
//======================================================================================
//======================================================================================
