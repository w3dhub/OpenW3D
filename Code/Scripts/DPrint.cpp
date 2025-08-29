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
*     DPrint.cpp
*
* DESCRIPTION
*     Debug printing mechanism
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Byon_g $
*     $Revision: 2 $
*     $Modtime: 2/13/01 11:02a $
*     $Archive: /Commando/Code/Scripts/DPrint.cpp $
*
****************************************************************************/

#ifdef _DEBUG

#include "DPrint.h"
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "scriptcommands.h"
extern ScriptCommands* Commands;

#define LOGFILE_NAME "ScriptLog"

/****************************************************************************
*
* NAME
*     DPrint(String, ArgList...)
*
* DESCRIPTION
*     Ouput debug print messages to the debugger and log file.
*
* INPUTS
*     String  - String to output.
*     ArgList - Argument list
*
* RESULT
*     NONE
*
****************************************************************************/

void __cdecl DebugPrint(const char* string, ...)
	{
	static char _buffer[1024];
	static char _filename[512] = "";

	if (string != NULL)
		{
		va_list	va;

		// Format string
		va_start(va, string);
		vsprintf(&_buffer[0], string, va);
		va_end(va);

		if (Commands != NULL)
			{
			// Send string to commando executable
			Commands->Debug_Message(_buffer);
			}
		else
			{
			// Send string to debugger
			OutputDebugStringA(_buffer);
			}

#if 0
		HANDLE file = INVALID_HANDLE_VALUE;

		// Open log file
		if (strlen(_filename) == 0)
			{
			char path[_MAX_PATH];
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];

			GetModuleFileNameA(GetModuleHandleA(NULL), &path[0], sizeof(path));
			_splitpath(path, drive, dir, NULL, NULL);
			_makepath(_filename, drive, dir, LOGFILE_NAME, "txt");

			file = CreateFileA(_filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			}
		else
			{
			file = CreateFileA(_filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			}
		
		// Insert carriage return after newlines
		int i = 0;

		while (_buffer[i] != '\0')
			{
			if (_buffer[i] == '\n')
				{
				int end = strlen(_buffer);
				assert((end + 1) <= sizeof(_buffer));

				while (end >= i)
					{
					_buffer[end + 1] = _buffer[end];
					end--;
					}

				_buffer[i] = '\r';
				i++;
				}

			i++;
			}

		// Send string to log file
		if (file != INVALID_HANDLE_VALUE)
			{
			DWORD written;

			SetFilePointer(file, 0, NULL, FILE_END);
			WriteFile(file, &_buffer[0], strlen(_buffer), &written, NULL);
			CloseHandle(file);
			}
#endif
		}
	}

#endif // _DEBUG
