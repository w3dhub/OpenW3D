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

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef DEFFONT_H
#define DEFFONT_H

#include "wwfont.h"

// Default 1 color fonts.
extern WWFontClass				DefaultFont;


enum FontStyleEnum {
	// Note: Space 6 means that a standard letter uses 6 points - the font is larger then that
	//			because of overhangs and rises.
	//			The 4 Color means there are four colors used in this not including transperancy.
	SPACE_6_PNT_2_COLOR,
	SPACE_8_PNT_3_COLOR,
	SPACE_11_PNT_4_COLOR,

	NUM_FONT_STYLES,
};

enum FontRemapEnum  {
	// A temp buffer that programmer can change before each use.
	USER_256_COLORS,

	DARK_GREEN_3_COLOR,
	LIGHT_GREEN_3_COLOR,
	DARK_GREEN_4_COLOR,
	LIGHT_GREEN_4_COLOR,
	DARK_GREEN_2_COLOR,
	LIGHT_GREEN_2_COLOR,
	DARK_BLUE_2_COLOR,
	LIGHT_BLUE_2_COLOR,
	DARK_RED_4_COLOR,

	// This is a RAMP palette.  See/Use Build_Font_Palette_From_Ramp().
	RAMP_GREEN_TO_RED_2_COLOR,	   // For use with 2 color fonts where color 2 is changes.  Color 1 is preset to 0.

	NUM_FONT_REMAPS,
};

// Two spacy looking fonts that use 8 colors each.
extern void 						*FontData[NUM_FONT_STYLES];

// For now we have green cause that is all that is needed.  Add more as needed.
extern unsigned char				*FontRemapTables[NUM_FONT_REMAPS];

// Number of colors (allocated chars) in FontRemapTable.  Useful for some of the RAMPS.
extern int							FontRemapTablesSize[NUM_FONT_REMAPS];

// Converter class for use with Font*Data and *FontRemapTable.
// You can create a font like this:
//		NEW WWFontClass(SpaceFont8Data, true, 1, FontConverter, GreenFontRemapTable);
extern ConvertClass				*FontConverter;

// Load font data for fonts listed above.
void Load_Default_Font_Data();
void Release_Default_Font_Data();

// A routine that will load font data from a *.fnt file make from FontMake.exe
void *Load_Font_Data(char *fname);

// Creates a palette from a ramp palette given min and max values.
unsigned char *Build_Font_Palette_From_Ramp(float val_norm, FontRemapEnum remap, unsigned char *ret_remap = &FontRemapTables[USER_256_COLORS][0]);

#endif
