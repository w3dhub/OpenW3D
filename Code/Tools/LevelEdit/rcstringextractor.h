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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : leveledit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/rcstringextractor.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/29/01 9:56a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __RCSTRINGEXTRACTOR_H
#define __RCSTRINGEXTRACTOR_H


#include "wwstring.h"


//////////////////////////////////////////////////////////////////////
//
//	RCStringExtractorClass
//
//////////////////////////////////////////////////////////////////////
class RCStringExtractorClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	RCStringExtractorClass  (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Configuration
	//
	void			Set_Src_RC_Filename (const char *full_path);
	void			Set_TranslationDB_Prefix (const char *prefix)		{ Prefix = prefix; }
	void			Set_TranslationDB_Category (const char *category)	{ CategoryName = category; }

	//
	//	Extraction
	//
	void			Extract_Strings (void);

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	void			Process_Line (StringClass &line, int &current_index);
	bool			Find_String (StringClass &line, const char *keyword, const char *replacement, StringClass &contents);
	int			Find_Starting_Index (void);

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	StringClass	SrcFilename;
	StringClass	DestFilename;
	StringClass	Prefix;
	StringClass	CategoryName;
	int			CategoryIndex;
};


#endif //__RCSTRINGEXTRACTOR_H
