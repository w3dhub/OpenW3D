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

//
// Filename:     miscutil.cpp
// Project:      wwutil
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:  
//
//-----------------------------------------------------------------------------
#include "miscutil.h" // I WANNA BE FIRST!

#include <time.h>
#include <cstdio>

#include "rawfile.h"
#include "wwdebug.h"
#include "ffactory.h"

//
// cMiscUtil statics 
//

//---------------------------------------------------------------------------
const char * cMiscUtil::Get_Text_Time(void)
{
   //
   // Returns a pointer to an internal statically allocated buffer...
   // Subsequent time operations will destroy the contents of that buffer.
   // Note: BoundsChecker reports 2 memory leaks in ctime here.
	//

	time_t time_now = ::time(NULL);
   char * time_str = ::ctime(&time_now);
   time_str[::strlen(time_str) - 1] = 0; // remove \n
   return time_str; 
}

//---------------------------------------------------------------------------
void cMiscUtil::Seconds_To_Hms(float seconds, int & h, int & m, int & s)
{
   WWASSERT(seconds >= 0);

   h = (int) (seconds / 3600);
   seconds -= h * 3600;
   m = (int) (seconds / 60);
   seconds -= m * 60;
   s = (int) seconds;

   WWASSERT(h >= 0);
   WWASSERT(m >= 0 && m < 60);
   WWASSERT(s >= 0 && s < 60);

   //WWASSERT(fabs((h * 3600 + m * 60 + s) / 60) - mins < WWMATH_EPSILON);
}

//-----------------------------------------------------------------------------
bool cMiscUtil::Is_String_Same(const char * str1, const char * str2)
{
   WWASSERT(str1 != NULL);
   WWASSERT(str2 != NULL);

   return(::stricmp(str1, str2) == 0);
}

//-----------------------------------------------------------------------------
bool cMiscUtil::Is_String_Different(const char * str1, const char * str2)
{
   WWASSERT(str1 != NULL);
   WWASSERT(str2 != NULL);

   return(::stricmp(str1, str2) != 0);
}

//-----------------------------------------------------------------------------
bool cMiscUtil::File_Exists(const char * filename)
{
#if 0
   WWASSERT(filename != NULL);

	WIN32_FIND_DATAA find_info;
   HANDLE file_handle = ::FindFirstFileA(filename, &find_info);
	
	if (file_handle != INVALID_HANDLE_VALUE) {
		::FindClose(file_handle);
		return true;
	} else {
		return false;
	}
#else
	FileClass * file = _TheFileFactory->Get_File( filename );
	if ( file && file->Is_Available() ) {
		return true;
	}
	_TheFileFactory->Return_File( file );
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool cMiscUtil::Is_Alphabetic(char c)
{
   return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

//-----------------------------------------------------------------------------
bool cMiscUtil::Is_Numeric(char c)
{
   return (c >= '0' && c <= '9');
}

//-----------------------------------------------------------------------------
bool cMiscUtil::Is_Alphanumeric(char c)
{
   return Is_Alphabetic(c) || Is_Numeric(c);
}

//-----------------------------------------------------------------------------
bool cMiscUtil::Is_Whitespace(char c)
{
   return c == ' ' || c == '\t';
}

//-----------------------------------------------------------------------------
void cMiscUtil::Trim_Trailing_Whitespace(char * text)
{	
   WWASSERT(text != NULL);

	int length = static_cast<int>(::strlen(text));
	while (length > 0 && Is_Whitespace(text[length - 1])) {
		text[--length] = 0;
	}
}

//-----------------------------------------------------------------------------
void cMiscUtil::Remove_File(const char * filename)
{
   WWASSERT(filename != NULL);

    ::remove(filename);
}

















/*
#define SIZE_OF_NT_SIGNATURE   sizeof(DWORD)
#define PEFHDROFFSET(a) ((LPVOID)((BYTE *)a +  \
    ((PIMAGE_DOS_HEADER)a)->e_lfanew + SIZE_OF_NT_SIGNATURE))
*/

/*
int cMiscUtil::Get_Exe_Key(void)
{
   //
   // Get exe name
   //
	char filename[500];
   int succeeded;
	succeeded = ::GetModuleFileNameA(NULL, filename, sizeof(filename));
	::strupr(filename);
	WWASSERT(succeeded);
      
   //
   // Get size
   //
   RawFileClass file(filename);
   int filesize = file.Size();
	WWASSERT(filesize > 0);
   file.Close();

   //
   // Strip path off filename
   //
   char * p_start = &filename[strlen(filename)];
   int num_chars = 1;
   while (*(p_start - 1) != '\\') {
      p_start--;
      num_chars++;
   }
   ::memmove(filename, p_start, num_chars);

	//
	// Pull a time/date stamp out of the exe header
	//
	PIMAGE_FILE_HEADER p_header = (PIMAGE_FILE_HEADER) PEFHDROFFSET(ProgramInstance);
	WWASSERT(p_header != NULL);
	int time_date_stamp = p_header->TimeDateStamp;

	//
	// Put all this data into a string
	//
	char id_string[500];
	::sprintf(id_string, "%s %d %d", filename, filesize, time_date_stamp);
	WWDEBUG_SAY(("File id string: %s\n", id_string));

	//
	// return the crc of that string as the key
	//
	return CRCEngine()(id_string, strlen(id_string));
}
*/

//#include <stdio.h>
//#include "verchk.h"

/*
//-----------------------------------------------------------------------------
int cMiscUtil::Get_Exe_Key(void)
{
   //
   // Get exe name
   //
	char filename[500];
   int succeeded;
	succeeded = ::GetModuleFileNameA(NULL, filename, sizeof(filename));
	::strupr(filename);
	WWASSERT(succeeded);
      
	StringClass string;
	Get_File_Id_String(filename, string);

	//
	// return the crc of that string as the key
	//
	return CRCEngine()(string, strlen(string));
}
*/

//#include "crc.h"
