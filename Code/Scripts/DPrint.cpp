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

void DebugPrint(const char* string, ...)
	{
	static char _buffer[1024];
	static char _filename[512] = "";

	if (string != nullptr)
		{
		va_list	va;

		// Format string
		va_start(va, string);
		vsprintf(&_buffer[0], string, va);
		va_end(va);

		if (Commands != nullptr)
			{
			// Send string to commando executable
			Commands->Debug_Message(_buffer);
			}
		else
			{
			// Send string to debugger
			fprintf(stderr, "%s", _buffer);
			}
		}
	}

#endif // _DEBUG
