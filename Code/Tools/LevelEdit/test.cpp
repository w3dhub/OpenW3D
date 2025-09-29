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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/test.cpp                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/23/99 7:23p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//#define _CATCH_MEM_LEAKS
#if (defined(_CATCH_MEM_LEAKS) && defined (_DEBUG))

#include "Windows.H"
#include "Winnt.H"
#include "ImageHlp.H"

extern void *_ptr_array[10000];
extern int g_allocs;
extern int g_frees;


typedef struct {
	char function_name[80];
} MY_DEBUG_INFO;

const int DATA_LEVELS = 5;
const DATA_SIZE = sizeof (MY_DEBUG_INFO) * DATA_LEVELS;


void *_ptr_array[10000] = { 0 };
int g_allocs = 0;
int g_frees = 0;
bool _g_binit = false;


class ReportDebugClass
{
	public:
		ReportDebugClass (void) {}
		~ReportDebugClass (void)
		{
			int counter = 0;
			for (int index = 0; index < 10000; index ++) {
				if (_ptr_array[index] != 0) {
					MY_DEBUG_INFO debug_info[DATA_LEVELS] = { 0 };
					::memcpy (debug_info, ((char*)(_ptr_array[index]))-DATA_SIZE, DATA_SIZE);
					counter ++;

					::OutputDebugString ("Possible memory leak for stack:\n");
					for (int ilevel = 0; ilevel < DATA_LEVELS; ilevel ++) {
						::OutputDebugString ("\t");
						::OutputDebugString (debug_info[ilevel].function_name);
						::OutputDebugString ("\n");
					}
					::OutputDebugString ("\n");
				}
			}
			return ;
		}
};

ReportDebugClass _debug_class;


///////////////////////////////////////////////////////////////////////////
//
//	::operator new
//
void *
::operator new (size_t size)
{

	if (_g_binit == false) {	
		_g_binit = true;
		::SymInitialize (::GetCurrentProcess (), NULL,  TRUE);	
	}


	CONTEXT context = { 0 };
	HANDLE hthread = ::GetCurrentThread ();
	context.ContextFlags = CONTEXT_FULL;
	::GetThreadContext (hthread, &context);

	HANDLE hthread2 = NULL;
	DuplicateHandle (::GetCurrentProcess (), hthread, ::GetCurrentProcess (), &hthread2, THREAD_ALL_ACCESS | THREAD_GET_CONTEXT, FALSE, 0);
	context.ContextFlags = CONTEXT_FULL;
	::GetThreadContext (hthread2, &context);
	::CloseHandle (hthread2);

	MY_DEBUG_INFO debug_info[DATA_LEVELS] = { 0 };	

	// Initialize the STACKFRAME structure for the first call.  This is only
	// necessary for Intel CPUs, and isn't mentioned in the documentation.
	STACKFRAME sf = { 0 };
	sf.AddrPC.Offset       = context.Eip;
	sf.AddrPC.Mode         = AddrModeFlat;
	sf.AddrStack.Offset    = context.Esp;
	sf.AddrStack.Mode      = AddrModeFlat;
	sf.AddrFrame.Offset    = context.Ebp;
	sf.AddrFrame.Mode      = AddrModeFlat;

	
	// Walk the stack up to DATA_LEVELS calls
	for (int iframe = 0; iframe < DATA_LEVELS+1; iframe ++)
	{
		// Get a stack trace for this frame
		if (!::StackWalk (IMAGE_FILE_MACHINE_I386,
							   GetCurrentProcess (),
								GetCurrentThread (),
								&sf,
								&context,
								0,
								SymFunctionTableAccess,
								SymGetModuleBase,
								0)) {
			break;
		}

		// Basic sanity check to make sure
		if (sf.AddrFrame.Offset == 0) {
			break;
		}

		// IMAGEHLP is wacky, and requires you to pass in a pointer to an
		// IMAGEHLP_SYMBOL structure.  The problem is that this structure is
		// variable length.  That is, you determine how big the structure is
		// at runtime.  This means that you can't use sizeof(struct).
		// So...make a buffer that's big enough, and make a pointer
		// to the buffer.  We also need to initialize not one, but TWO
		// members of the structure before it can be used.

		BYTE symbol_buffer [sizeof(IMAGEHLP_SYMBOL) + 512];
		PIMAGEHLP_SYMBOL psymbol = (PIMAGEHLP_SYMBOL)symbol_buffer;
		::memset (symbol_buffer, 0, sizeof (symbol_buffer));
		psymbol->SizeOfStruct = sizeof(symbol_buffer);
		psymbol->MaxNameLength = 512;

		DWORD sym_displacement = 0;
		if (::SymGetSymFromAddr (::GetCurrentProcess (),
										 sf.AddrPC.Offset,
										 &sym_displacement,
										 psymbol))
		{
			// We don't care about the first stack frame (its inside the GetThreadContext call)
			if (iframe >=1) {
				::lstrcpyn (debug_info[iframe-1].function_name, psymbol->Name, sizeof (MY_DEBUG_INFO)-1);
				debug_info[iframe-1].function_name[sizeof (debug_info[iframe-1].function_name)-1] = 0;
			}
		} else {
			DWORD dwerror = ::GetLastError ();
			int itest = 0;
		}

	}

	// Allocate the buffer + our debug information
	void *ptr = ::malloc (size + DATA_SIZE);
	::memcpy (ptr, debug_info, DATA_SIZE);				

	// Stick the new buffer into our array (should be moved to linked list)
	if (ptr) {
		for (int index = 0; index < 10000; index ++) {
			if (_ptr_array[index] == 0) {
				_ptr_array[index] = (void *)(((char *)ptr) + DATA_SIZE);
				break;
			}
		}
	}

	g_allocs ++;

	// Return the pointer
	return (void*)(((char *)ptr) + DATA_SIZE);
}


///////////////////////////////////////////////////////////////////////////
//
//	:::operator delete
//
void
operator delete (void *ptr) noexcept
{
	if (ptr) {

		// Attempt to find the buffer in our array
		bool bfound = false;
		for (int index = 0; index < 10000; index ++) {
			if (_ptr_array[index] == ptr) {
				::free ((void*)(((char *)ptr) - DATA_SIZE));
				_ptr_array[index] = 0;
				bfound = true;
				break;
			}
		}

		// If we didn't find it in our array, just free it
		if (!bfound) {
			//::free (ptr);
		}
	}

	g_frees ++;
	return ;
}

#endif //defined(_CATCH_MEM_LEAKS) && defined (_DEBUG)
