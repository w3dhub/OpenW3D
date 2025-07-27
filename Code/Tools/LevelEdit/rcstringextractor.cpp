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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/rcstringextractor.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/29/01 11:08a                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "stdafx.h"
#include "rcstringextractor.h"
#include "translatedb.h"
#include "stringsmgr.h"
#include "textfile.h"


//////////////////////////////////////////////////////////////////////
//
//	RCStringExtractorClass
//
//////////////////////////////////////////////////////////////////////
RCStringExtractorClass::RCStringExtractorClass (void)	:
	CategoryIndex (0)
{
	return; 
}


//////////////////////////////////////////////////////////////////////
//
//	Find_String
//
//////////////////////////////////////////////////////////////////////
bool
RCStringExtractorClass::Find_String
(
	StringClass &		line,
	const char *		keyword,
	const char *		replacement,
	StringClass &		contents
)
{
	bool retval = false;

	StringClass new_line = line;

	//
	//	Did we find the keyword?
	//
	char *buffer = const_cast<char *>(::strstr (new_line, keyword));
	if (buffer != NULL) {
		buffer += ::lstrlen (keyword);

		//
		//	Skip to the start of the string
		//
		bool found_start = false;
		while (buffer[0] != 0) {
			if (buffer[0] == '\"') {
				buffer ++;
				found_start = true;
				break;
			}

			buffer ++;
		}

		if (found_start) {

			//
			//	Skip past any format specifiers
			//
			if (buffer[0] == '%') {
				buffer += 2;
			}

			//
			//	Record the start and end of the string
			//
			bool found_end = false;
			char *start = buffer;
			while (buffer[0] != 0) {
				if (buffer[0] == '\"') {
					found_end = true;
					break;
				}

				buffer ++;
			}

			if (found_end) {
				char *end = buffer;

				//
				//	Return the contents to the caller
				//	
				end[0] = 0;
				contents = start;

				if (contents.Get_Length () > 0) {

					//
					//	Check to ensure this isn't already a translation string
					//
					if (::strstr (contents, "IDS_") == NULL) {

						//
						//	Now fill in the replacement
						//
						start[0] = 0;
						line = new_line.Peek_Buffer ();
						line += replacement;
						line += "\"";
						line += end + 1;
						retval = true;
					}
				}
			}
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Process_Line
//
//////////////////////////////////////////////////////////////////////
void
RCStringExtractorClass::Process_Line (StringClass &line, int &current_index)
{
	StringClass contents;

	const char *KEYWORDS[] =
	{
			"CAPTION",
			"LTEXT",
			"PUSHBUTTON",
			"CONTROL",
			"RTEXT",
			"CTEXT"
	};

	const int KEYWORD_COUNT = sizeof (KEYWORDS) / sizeof (const char *);

	//
	//	Make a string identifier from the prefix and the current index
	//
	StringClass string_id;
	string_id.Format ("%s%.3d", Prefix.Peek_Buffer (), current_index);

	//
	//	Check to see if this line is one of the ones we need to process
	//
	for (int index = 0; index < KEYWORD_COUNT; index ++) {
		if (Find_String (line, KEYWORDS[index], string_id, contents)) {
			
			//
			//	Create a new translation DB entry for this string
			//
			TDBObjClass *new_obj = new TDBObjClass;
			new_obj->Set_English_String (contents);
			new_obj->Set_ID_Desc (string_id);
			new_obj->Set_Category_ID (CategoryIndex);
			
			//
			//	Add the new object to the database
			//
			TranslateDBClass::Add_Object (new_obj);
			current_index ++;
			break;
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  Set_Src_RC_Filename
//
//////////////////////////////////////////////////////////////////////////
void
RCStringExtractorClass::Set_Src_RC_Filename (const char *full_path)
{
	SrcFilename		= full_path;
	DestFilename	= SrcFilename + "_extracted";
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  Extract_Strings
//
//////////////////////////////////////////////////////////////////////////
void
RCStringExtractorClass::Extract_Strings (void)
{
	//
	//	Check out the strings database
	//
	if (StringsMgrClass::Check_Out ()) {

		CategoryIndex = 0;

		//
		//	Try to find the category these strings are to be imported into
		//
		TDBCategoryClass *category = TranslateDBClass::Find_Category (CategoryName);
		if (category != NULL) {
			CategoryIndex = category->Get_ID ();
		}

		//
		//	Try to open the .RC file
		//
		TextFileClass file (SrcFilename);
		TextFileClass output_file (DestFilename);
		if (file.Open (RawFileClass::READ) && output_file.Open (RawFileClass::WRITE)) {
			
			//
			//	Determine where to start our numbering scheme
			//
			int current_index = Find_Starting_Index ();

			//
			//	Read each line from the file
			//
			StringClass curr_line;
			while (file.Read_Line (curr_line)) {
				
				Process_Line (curr_line, current_index);

				curr_line += "\r\n";
				output_file.Write (curr_line, ::lstrlen (curr_line));
			}

			//
			//	Close the files
			//
			file.Close ();
			output_file.Close ();
		}

		//
		//	Save the translation database and check it back into VSS
		//
		StringsMgrClass::Save_Translation_Database ();
		StringsMgrClass::Check_In ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  Find_Starting_Index
//
//////////////////////////////////////////////////////////////////////////
int
RCStringExtractorClass::Find_Starting_Index (void)
{
	int max_index = 0;

	int prefix_len = Prefix.Get_Length ();

	//
	//	Loop over all the objects in the translation database
	//
	int count = TranslateDBClass::Get_Object_Count ();
	for (int index = 0; index < count; index ++) {
		TDBObjClass *obj = TranslateDBClass::Get_Object (index);
		if (obj != NULL) {
			
			//
			//	Is this one of the strings we care about?
			//
			const StringClass &string_desc = obj->Get_ID_Desc ();
			if (::strnicmp (Prefix, string_desc, prefix_len) == 0) {
				
				//
				//	Check to see if this is the largest index we've found yet
				//
				const char *suffix	= (string_desc.Peek_Buffer () + prefix_len);
				int number				= ::atoi (suffix);
				max_index				= max (number, max_index);
			}
		}
	}

	return max_index + 1;
}
