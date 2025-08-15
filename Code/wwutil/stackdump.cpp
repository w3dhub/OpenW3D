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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/wwutil/stackdump.cpp                 $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 9/29/01 1:13p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 1                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "stackdump.h"

#include	<windows.h>
#include	<imagehlp.h>
#include	<conio.h>
#include	<imagehlp.h>
#include <crtdbg.h>
#include	<stdio.h>
#include <stdlib.h>
#include <assert.h>

#include	"wwdebug.h"

typedef BOOL		(WINAPI *SymCleanupType)					(HANDLE hProcess);
typedef BOOL		(WINAPI *SymGetSymFromAddrType)			(HANDLE hProcess, DWORD Address, LPDWORD Displacement, PIMAGEHLP_SYMBOL Symbol);
typedef BOOL		(WINAPI *SymInitializeType)				(HANDLE hProcess, LPSTR UserSearchPath, BOOL fInvadeProcess);
typedef BOOL		(WINAPI *SymLoadModuleType)				(HANDLE hProcess, HANDLE hFile, LPSTR ImageName, LPSTR ModuleName, DWORD BaseOfDll, DWORD SizeOfDll);
typedef DWORD		(WINAPI *SymSetOptionsType)				(DWORD SymOptions);
typedef BOOL		(WINAPI *SymUnloadModuleType)				(HANDLE hProcess, DWORD BaseOfDll);
typedef BOOL		(WINAPI *StackWalkType)						(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME StackFrame, LPVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE TranslateAddress);
typedef LPVOID		(WINAPI *SymFunctionTableAccessType)	(HANDLE hProcess, DWORD AddrBase);
typedef DWORD		(WINAPI *SymGetModuleBaseType)			(HANDLE hProcess, DWORD dwAddr);


static SymCleanupType					_SymCleanup						= NULL;
static SymGetSymFromAddrType			_SymGetSymFromAddr			= NULL;
static SymInitializeType				_SymInitialize					= NULL;
static SymLoadModuleType				_SymLoadModule					= NULL;
static SymSetOptionsType				_SymSetOptions					= NULL;
static SymUnloadModuleType				_SymUnloadModule				= NULL;
static StackWalkType						_StackWalk						= NULL;
static SymFunctionTableAccessType	_SymFunctionTableAccess		= NULL;
static SymGetModuleBaseType			_SymGetModuleBase				= NULL;

static char const * ImagehelpFunctionNames[] =
{
	"SymCleanup",
	"SymGetSymFromAddr",
	"SymInitialize",
	"SymLoadModule",
	"SymSetOptions",
	"SymUnloadModule",
	"StackWalk",
	"SymFunctionTableAccess",
	"SymGetModuleBaseType",
	NULL
};

//
// Class statics
//

//-----------------------------------------------------------------------------
static char const * 
Last_Error_Text
(
	void
)
{
	static char message_buffer[256];
	::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &message_buffer[0], 256, NULL);
	return (message_buffer);
}

//-----------------------------------------------------------------------------
void
cStackDump::Print_Call_Stack
(
	void
)
{
	WWDEBUG_SAY(("cStackDump::Print_Call_Stack:\n"));

	HINSTANCE imagehelp = ::LoadLibraryA("IMAGEHLP.DLL");

	if (imagehelp == NULL) 
	{
		WWDEBUG_SAY(("  Unable to load IMAGEHLP.DLL.\n"));
	}
	else 
	{
		WWDEBUG_SAY(("  Found IMAGEHLP.DLL - linking to required functions.\n"));

		char const * function_name = NULL;
		ULONG * fptr = (ULONG *) &_SymCleanup;
		int count = 0;

		do 
		{
			function_name = ImagehelpFunctionNames[count];
			if (function_name != NULL) 
			{
				*fptr = (ULONG) ::GetProcAddress(imagehelp, function_name);
				fptr++;
				count++;
			}
		} while (function_name != NULL);
	}

	extern int Stack_Walk(ULONG * return_addresses, int num_addresses, CONTEXT * context);
	ULONG return_addresses[256];
	int num_addresses = Stack_Walk(return_addresses, 256, NULL);
	unsigned char symbol[256];
	IMAGEHLP_SYMBOL * symptr = (IMAGEHLP_SYMBOL*) &symbol;
	int symload = 0;
	bool symbols_available = false;

	if (_SymInitialize != NULL && _SymInitialize(::GetCurrentProcess(), NULL, false))	
	{
		WWDEBUG_SAY(("  Symbols are available.\n"));
		symbols_available = true;

		if (_SymSetOptions != NULL) 
		{
			_SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME);
		}

		char module_name[_MAX_PATH];
		::GetModuleFileNameA(NULL, module_name, sizeof(module_name));

		if (_SymLoadModule != NULL) 
		{
			symload = _SymLoadModule(::GetCurrentProcess(), NULL, module_name, NULL, 0, 0);
		}

		if (symload == 0 && GetLastError() != 0)
		{
			assert(_SymLoadModule != NULL);
			WWDEBUG_SAY(("  SymLoad failed for module %s with code %d - %s.\n", 
				module_name, GetLastError(), Last_Error_Text()));
		}
	} 
	else 
	{
		WWDEBUG_SAY(("  Symbols are NOT available.\n"));
		WWDEBUG_SAY(("  SymInitialize failed with code %d - %s.\n", GetLastError(), Last_Error_Text()));
	}

	if (num_addresses > 0) 
	{
		WWDEBUG_SAY(("  Stack:\n"));
		for (int s = 0 ; s < num_addresses; s++) 
		{
			ULONG temp_addr = return_addresses[s];

			for (int space = 0 ; space <= s ; space++) 
			{
				WWDEBUG_SAY(("  "));
			}

			if (symbols_available) 
			{
				symptr->SizeOfStruct = sizeof(symbol);
				symptr->MaxNameLength = 128;
				symptr->Size = 0;
				symptr->Address = temp_addr;

				ULONG displacement = 0;
				if (_SymGetSymFromAddr != NULL && _SymGetSymFromAddr(GetCurrentProcess(), temp_addr, &displacement, symptr)) 
				{
					char symbuf[256];
					//::sprintf(symbuf, "%s + %08X\n", symptr->Name, displacement);
					if (s == 0)
					{
						::sprintf(symbuf, "%s + %08X", symptr->Name, displacement);
					}
					else
					{
						::sprintf(symbuf, "%s", symptr->Name);
					}
					WWDEBUG_SAY((symbuf));
				}
			} 
			else 
			{
				char symbuf[256];
				::sprintf(symbuf, "%08x", temp_addr);
				WWDEBUG_SAY((symbuf));
			}

			WWDEBUG_SAY(("\n"));
		}
	} 
	else 
	{
		WWDEBUG_SAY(("  Stack walk failed!\n"));
	}

	//
	// Unload the symbols.
	//
	if (symbols_available) 
	{
		if (_SymCleanup != NULL) 
		{
			_SymCleanup(GetCurrentProcess());
		}

		if (symload && _SymUnloadModule != NULL) 
		{
			_SymUnloadModule(GetCurrentProcess(), NULL);
		}
	}

	if (imagehelp) 
	{
		::FreeLibrary(imagehelp);
	}
}



















	/*
	//
	//	Determine the path to the executable
	//
	char path[MAX_PATH] = "";
	DWORD gmf = ::GetModuleFileNameA(NULL, path, sizeof(path));
	
	if (gmf != 0)
	{
		//
		//	Strip off the filename
		//
		char * filename = ::strrchr(path, '\\');
		if (filename != NULL) 
		{
			filename[0] = 0;
		}

		::SetCurrentDirectoryA(path);
	}
	*/

