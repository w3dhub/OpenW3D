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
*     $Archive: /Commando/Code/BinkMovie/subtitleparser.h $
*
* DESCRIPTION
*     Subtitling control file parser
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Ian_l $
*     $Modtime: 8/24/01 2:36p $
*     $Revision: 1 $
*
****************************************************************************/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef _SUBTITLEPARSER_H_
#define _SUBTITLEPARSER_H_

#include "always.h"
#include "unichar.h"
#include <wwlib/vector.h>
#include <stddef.h>

class Straw;
class SubTitleClass;

class SubTitleParserClass
{
public:
	SubTitleParserClass(Straw& input);
	~SubTitleParserClass();

	DynamicVectorClass<SubTitleClass*>* Get_Sub_Titles(const char* moviename);

private:
	enum {MAXIMUM_LINE_LENGTH = 1024};

	typedef struct tagTokenHook
	{
		const unichar_t* Token;
		void (*Handler)(unichar_t* param, SubTitleClass* subTitle);
	} TokenHook;

	// Prevent copy construction
	SubTitleParserClass(const SubTitleParserClass&);
	const SubTitleParserClass operator=(const SubTitleParserClass&);

	bool Find_Movie_Entry(const char* moviename);
	bool Parse_Sub_Title(unichar_t* string, SubTitleClass* subTitle);
	void Parse_Token(unichar_t* token, unichar_t* param, SubTitleClass* subTitle);
	unichar_t* Get_Next_Line(void);
	unsigned int Get_Line_Number(void) const	{return mLineNumber;}

	static TokenHook mTokenHooks[];
	Straw& mInput;
	unichar_t mBuffer[MAXIMUM_LINE_LENGTH];
	unsigned int mLineNumber;
};

#endif // _SUBTITLEPARSER_H_
