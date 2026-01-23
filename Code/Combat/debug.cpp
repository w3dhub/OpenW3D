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
 *                     $Archive:: /Commando/Code/Combat/debug.cpp                             $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/16/02 8:44p                                               $*
 *                                                                                             *
 *                    $Revision:: 90                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "debug.h"
#include "input.h"
#include "ww3dtrig.h"
#include "wwphystrig.h"
#include "timemgr.h"
#include "ww3d.h"
#include "registry.h"
#include <stdio.h>
#include "WWAudio.h"
#include "combat.h"
#include "wwmemlog.h"
#include "FastAllocator.h"

#ifndef 	STEVES_NEW_CATCHER
#define LOG_MEMORY 1		// enable this to turn on memory logging
#endif	//STEVES_NEW_CATCHER

/*
**
*/
int		DebugManager::EnabledDevices;
int		DebugManager::EnabledTypes;
int		DebugManager::EnabledOptions;
bool	DebugManager::EnableFileLogging;
bool	DebugManager::EnableDiagLogging;
bool	DebugManager::LoadDebugScripts;
int	DebugManager::VersionNumber = 0;
bool	DebugManager::IsSlave = false;
bool	DebugManager::AllowCinematicKeys = false;

CriticalSectionClass DebugManager::CriticalSection;

DebugDisplayHandlerClass *	DebugManager::DisplayHandler		= NULL;
#define DEFAULT_LOGFILE_NAME "_logfile.txt"
const char *DebugManager::LOGFILE = DEFAULT_LOGFILE_NAME;
char DebugManager::LogfileNameBuffer[256];

/*
** local prototypes
*/
void wwdebug_message_handler(DebugType type, const char * message);
void wwdebug_assert_handler(const char * message);
bool wwdebug_trigger_handler(int trigger_num);
void wwdebug_profile_start_handler( const char * title );
void wwdebug_profile_stop_handler( const char * title );

char DefaultRegistryModifier[1024] = {""};

/*
**
*/
void	DebugManager::Init( void )
{
	// Install message handler functions for the WWDebug messages
	// and assertion failures.
	WWDebug_Install_Message_Handler(wwdebug_message_handler);
	WWDebug_Install_Assert_Handler(wwdebug_assert_handler);
	WWDebug_Install_Trigger_Handler(wwdebug_trigger_handler);
	WWDebug_Install_Profile_Start_Handler(wwdebug_profile_start_handler);
	WWDebug_Install_Profile_Stop_Handler(wwdebug_profile_stop_handler);

	// Clear all of the debug devices, types, and options
	EnabledDevices = -1;
	EnabledTypes	= -1;
	EnabledOptions	= 0;

	EnableFileLogging = false;
	EnableDiagLogging = false;
	LoadDebugScripts = false;

	//
	// By default let us turn these off
	//
	Disable_Device(DEBUG_DEVICE_SCREEN);
	Disable_Type(DEBUG_TYPE_NETWORK_PROLIFIC);

   //
	// Clear the logfile
	//
   Init_Logfile();

	Debug_Say(( "\n" ));
}

/*
**
*/
void	DebugManager::Shutdown( void )
{
	// Remove message handler functions for the WWDebug messages
	// and assertion failures.
	WWDebug_Install_Message_Handler(NULL);
	WWDebug_Install_Assert_Handler(NULL);
	WWDebug_Install_Trigger_Handler(NULL);
	WWDebug_Install_Profile_Start_Handler(NULL);
	WWDebug_Install_Profile_Stop_Handler(NULL);
}

/*
**
*/
void	DebugManager::Update( void )
{
	// Tell the profiler that a frame has passed

	if ( Input::Get_State( INPUT_FUNCTION_MAKE_SCREEN_SHOT) ) {
		WW3D::Make_Screen_Shot();
	}

	if ( Input::Get_State( INPUT_FUNCTION_TOGGLE_MOVIE_CAPTURE ) ) {
#ifdef WWDEBUG
		WW3D::Toggle_Movie_Capture();
#endif
	}

#if 0
	// Single Step code
	if (WWDEBUG_TRIGGER(0x53 /*S*/)) {
		while (!WWDEBUG_TRIGGER(0x20/*SPACE*/))		Input::Update();
		while (WWDEBUG_TRIGGER(0x20/*SPACE*/))			Input::Update();
	}
#endif
}

void	DebugManager::Load_Registry_Settings( const char * sub_key )
{
	RegistryClass registry( sub_key );
	if ( registry.Is_Valid() ) {
		EnabledDevices = registry.Get_Int( "EnabledDevices",	EnabledDevices );
		EnabledTypes	= registry.Get_Int( "EnabledTypes",	EnabledTypes );
		EnabledOptions	= registry.Get_Int( "EnabledOptions", EnabledOptions );
		EnableFileLogging	= registry.Get_Bool( "EnableFileLogging",	EnableFileLogging );
		EnableDiagLogging	= registry.Get_Bool( "EnableDiagLogging",	EnableDiagLogging );
		LoadDebugScripts = registry.Get_Bool( "LoadDebugScripts",	LoadDebugScripts );
		AllowCinematicKeys = registry.Get_Bool( "AllowCinematicKeys",	AllowCinematicKeys );
	}

#ifdef LOG_MEMORY
		Debug_Say(( "*** Memory Logging Enabled ***\n" ));
#endif

}

void	DebugManager::Save_Registry_Settings( const char * sub_key )
{
	RegistryClass registry( sub_key );
	if ( registry.Is_Valid() ) {
		registry.Set_Int( "EnabledDevices",			EnabledDevices );
		registry.Set_Int( "EnabledTypes",		EnabledTypes );
		registry.Set_Int( "EnabledOptions",	EnabledOptions );
		registry.Set_Bool( "EnableFileLogging",	EnableFileLogging );
		registry.Set_Bool( "EnableDiagLogging",	EnableDiagLogging );
		registry.Set_Bool( "LoadDebugScripts",	LoadDebugScripts );
		registry.Set_Bool( "AllowCinematicKeys",	AllowCinematicKeys );
	}
}

/*
**
*/
void	DebugManager::Display( char const *buffer )
{
	CriticalSectionClass::LockClass lock(CriticalSection);

	if ( EnabledDevices & DEBUG_DEVICE_SCREEN ) {
		Display_Text( buffer );
	}

#ifdef WWDEBUG
   if ( EnabledDevices & DEBUG_DEVICE_DBWIN32 ) {
		WWDebug_DBWin32_Message_Handler( buffer );
	}
#endif // WWDEBUG

	if ( EnabledDevices & DEBUG_DEVICE_LOG ) {
      Write_To_File(buffer);
	}

	if ( EnabledDevices & DEBUG_DEVICE_WINDOWS ) {
		OutputDebugStringA( buffer );		// puts it in the MSVC debug window
	}
}

//
//
//
void	DebugManager::Display_Script( char const *text, ... )
{
	if ( !(EnabledTypes & DEBUG_TYPE_SCRIPT) )	return;
	va_list	va;
	char		buffer[256];
	va_start(va, text);
	vsprintf(buffer, text, va);
	buffer[sizeof(buffer)-1] = '\0';
	char		buffer2[256];
	sprintf( buffer2, "SCRIPT:%s", buffer );
	Display( buffer2 );
	va_end(va);
}

/*
**
*/
void DebugManager::Display_Network_Admin(char const *text, ...)
{
   if (!(EnabledTypes & DEBUG_TYPE_NETWORK_ADMIN))	{
      return;
   }

	va_list va;
	char buffer[1024];
	va_start(va, text);
	vsprintf(buffer, text, va);
	buffer[sizeof(buffer)-1] = '\0';
	char buffer2[1024];
	sprintf(buffer2, "NET ADMIN:%s\n", buffer);
	Display(buffer2);
	va_end(va);
}

/*
**
*/
void DebugManager::Display_Network_Basic(char const *text, ...)
{
   if (!(EnabledTypes & DEBUG_TYPE_NETWORK_BASIC))	{
      return;
   }

	va_list va;
	char buffer[1024];
	va_start(va, text);
	vsprintf(buffer, text, va);
	buffer[sizeof(buffer)-1] = '\0';
	char buffer2[1024];
	sprintf(buffer2, "NET BASIC:%s\n", buffer);
	Display(buffer2);
	va_end(va);
}

/*
**
*/
void DebugManager::Display_Network_Prolific(char const *text, ...)
{
   if (!(EnabledTypes & DEBUG_TYPE_NETWORK_PROLIFIC))	{
      return;
   }

	va_list va;
	char buffer[1024];
	va_start(va, text);
	vsprintf(buffer, text, va);
	buffer[sizeof(buffer)-1] = '\0';
	char buffer2[1024];
	sprintf(buffer2, "NET PROLIFIC:%s\n", buffer);
	Display(buffer2);
	va_end(va);
}

/*
**
*/
void	DebugManager::Measure_Frame_Textures( void )
{
//	WW3D::Flush_Texture_Cache();
}

/*
**
*/
void wwdebug_message_handler(DebugType type, const char * message)
{
	/*
	** Hand the message off to the scrolling debug screen
	*/
	if ( !DebugManager::Is_Type_Enabled( (DebugManager::DebugType)(1<<type ) ) )	return;

	if ( type == WWDEBUG_TYPE_ERROR ) {
		DebugManager::Display( "ERROR:" );
	}

	if ( type == WWDEBUG_TYPE_WARNING ) {
		DebugManager::Display( "WARNING:" );
	}

	DebugManager::Display( message );
}

void wwdebug_assert_handler(const char * message)
{
	/*
	** Hand the message off to the scrolling debug screen
	*/
	DebugManager::Display( message );
}

bool wwdebug_trigger_handler([[maybe_unused]] int trigger_num)
{
#ifdef WWDEBUG
	switch( trigger_num ) {
		case ' ':	return 	Input::Get_State( INPUT_FUNCTION_DEBUG_SINGLE_STEP_STEP );
		case 'S':	return 	Input::Get_State( INPUT_FUNCTION_DEBUG_SINGLE_STEP );

		case WWDEBUG_TRIGGER_GENERIC0:				return Input::Get_State( INPUT_FUNCTION_DEBUG_GENERIC0 );
		case WWDEBUG_TRIGGER_GENERIC1:				return Input::Get_State( INPUT_FUNCTION_DEBUG_GENERIC1 );
		case WWPHYS_TRIGGER_COLLISION_DEBUGGING:	return DebugManager::Option_Is_Enabled(DebugManager::DEBUG_COLLISION_MESSAGES); //Input::Get_State( INPUT_FUNCTION_DEBUG_COLLISION_MESSAGES );
		case WWPHYS_TRIGGER_COLLISION_DISPLAY:		return DebugManager::Option_Is_Enabled(DebugManager::DEBUG_COLLISION_DISPLAY); //Input::Get_State( INPUT_FUNCTION_DEBUG_SHOW_COLLISIONS );
		case WWPHYS_TRIGGER_INVERT_VIS:				return DebugManager::Option_Is_Enabled(DebugManager::DEBUG_INVERT_VIS); //Input::Get_State( INPUT_FUNCTION_DEBUG_VIS_INVERT );
		case WWPHYS_TRIGGER_DISABLE_VIS:				return DebugManager::Option_Is_Enabled(DebugManager::DEBUG_DISABLE_VIS); //Input::Get_State( INPUT_FUNCTION_DEBUG_VIS_DISABLE );
		case WW3D_TRIGGER_PROCESS_STATS:				return DebugManager::Option_Is_Enabled(DebugManager::DEBUG_PROCESS_STATS); //Input::Get_State( INPUT_FUNCTION_DEBUG_RENDER_STATS );
		case WW3D_TRIGGER_RENDER_STATS:				return false; //Input::Get_State( INPUT_FUNCTION_DEBUG_RENDER_STATS );
		case WW3D_TRIGGER_SURFACE_CACHE_STATS:		return DebugManager::Option_Is_Enabled(DebugManager::DEBUG_SURFACE_CACHE); //Input::Get_State( INPUT_FUNCTION_DEBUG_SURFACE_CACHE );
		default:												Debug_Say(( "Unhandled Trigger %d %c\n", trigger_num, trigger_num));
	}
#endif
	return false;
}

void wwdebug_profile_start_handler( const char * /* title */ )
{
	// Perhaps switch to the ProfileManager calls...
}

void wwdebug_profile_stop_handler( const char * /* title */ )
{
}

/*
**
*/
void	DebugManager::Display_Text( const char * string, const Vector4 & color )
{
	if (DisplayHandler != NULL) {
		DisplayHandler->Display_Text( string, color );
	}
}

void	DebugManager::Display_Text( const char * string, const Vector3 & color )
{
	if (DisplayHandler != NULL) {
		DisplayHandler->Display_Text( string, Vector4(color[0],color[1],color[2],1.0f) );
	}
}


void	DebugManager::Display_Text( const WideStringClass & string, const Vector4 & color )
{
	if (DisplayHandler != NULL) {
		DisplayHandler->Display_Text( string, color );
	}
}

void	DebugManager::Display_Text( const WideStringClass & string, const Vector3 & color )
{
	if (DisplayHandler != NULL) {
		DisplayHandler->Display_Text( string, Vector4(color[0],color[1],color[2],1.0f) );
	}
}

//---------------------------------------------------------------------------
void DebugManager::Init_Logfile(void)
{
	if (IsSlave) {
		sprintf(LogfileNameBuffer, "%s%s", DefaultRegistryModifier, DEFAULT_LOGFILE_NAME);
		LOGFILE = LogfileNameBuffer;
	}

	//
   // Destroy contents
   //
   FILE * file = fopen(LOGFILE, "wt");
	if ( file ) {
		fclose(file);
	}
}

//---------------------------------------------------------------------------
void DebugManager::Write_To_File(LPCSTR str)
{
	//
   // I open/close for each write so as to maximize integrity of this file.
   //

   FILE * file = fopen(LOGFILE, "at");
   if (file != NULL) {
	   fwrite(str, 1, strlen(str), file);
	   fclose(file);
   }
}

/*****************************************************************************************************
**
** WWMEMLOG support - replacement new and delete operators.  See the wwmemlog modules in WWDEBUG.LIB
** for more info!
**
*****************************************************************************************************/
/*
** Only install the custom new and delete handlers if WWDEBUG is enabled (debug and profile builds)
** AND _CRTDBG_MAP_ALLOC is not defined (this causes link errors).  If you are using _CRTDBG_MAP_ALLOC,
** the memory log stuff cannot be used...
*/
//#ifdef LOG_MEMORY
//#ifdef WWDEBUG
#ifndef _CRTDBG_MAP_ALLOC
#ifndef PARAM_EDITING_ON
#ifndef STEVES_NEW_CATCHER

//extern "C" {
//void *gsimalloc(size_t size)
//{
//	WWMEMLOG(MEM_BINK);
//	return(WWMemoryLogClass::Allocate_Memory(size));
//}
//void gsifree(void *ptr)
//{
//	WWMEMLOG(MEM_BINK);
//	WWMemoryLogClass::Release_Memory(ptr);
//}
//}

#include <new>

void * operator new (size_t size)
{
	void* memory=NULL;
#ifdef LOG_MEMORY
	#ifdef WWDEBUG
		memory=WWMemoryLogClass::Allocate_Memory(size);
	#else
		memory=FastAllocatorGeneral::Get_Allocator()->Alloc(size);
	#endif
#else
	memory=FastAllocatorGeneral::Get_Allocator()->Alloc(size);
#endif
	return memory;
}

void operator delete (void *ptr) noexcept
{
#ifdef LOG_MEMORY
	#ifdef WWDEBUG
		WWMemoryLogClass::Release_Memory(ptr);
	#else
		FastAllocatorGeneral::Get_Allocator()->Free(ptr);
	#endif
#else
	FastAllocatorGeneral::Get_Allocator()->Free(ptr);
#endif
}

void operator delete(void *p, size_t /* size */) noexcept
{
    operator delete(p);
}

#endif //STEVES_NEW_CATCHER
#endif //PARAM_EDITING_ON
#endif //!_CRTDBG_MAP_ALLOC
//#endif //WWDEBUG
//#endif //LOG_MEMORY






/*
**
**
** Added 'new' operator. Walks the stack to help track memory leaks.
**
**
**
**
**
**
**
*/



#include <imagehlp.h>


#define WALK_FRAMES 8

struct NewCallerStruct {
	unsigned long Addresses[WALK_FRAMES];
	char AddressName[768];
	int AddressLine;
};


void* ReturnAddresses[20];
int Stack_Walk(void **return_addresses, int num_addresses, CONTEXT *);
bool Lookup_Symbol(void *code_ptr, char *symbol, int &displacement);
void *NewMutex = NULL;

#ifdef _DEBUG
#ifdef STEVES_NEW_CATCHER

extern _CRTIMP void * __cdecl operator new(size_t, int, const char *, int);

/*
** List of addresses to 'watch'. You can stuff them in here and compile or manually poke them in with the debugger at run time.
*/
#define NUM_WATCH_ADDRESSES 8
unsigned long WatchAddresses[NUM_WATCH_ADDRESSES] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000
};

/*
** This assumes no more than 100000 places in the code that call 'new'
*/
NewCallerStruct NewAddressList[16384];

/***********************************************************************************************
 * operator new -- New operator overload to catch memory leaks.                                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    size of block to allocate                                                         *
 *                                                                                             *
 * OUTPUT:   ptr to allocated memory                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/12/2001 4:31PM ST : Created                                                             *
 *=============================================================================================*/
void* operator new(size_t s)
{
	static char _localstr[32];
	static unsigned long _return_addr;
	static int _address_strings = 0;
	static int _index = -1;
	static int _i, _j;
	static char _temp_str[512];
	static char _compose_str[512];
	static int _displacement;
	static unsigned long _temp_addr;
	static char _spaces[33] = {"                                "};
	static int _wa;
	//static CriticalSectionClass *_new_mutex = NULL;

	/*
	** Length of all locals.
	*/
	static int _locals_size = 0;

	/*
	** Wait for exclusive access.
	*/
	//if (_new_mutex == NULL) {
	//	void *newmem = ::operator new(s, 1, __FILE__, __LINE__);
	//	_new_mutex = new(newmem) CriticalSectionClass;
	//
	//}
	//CriticalSectionClass::LockClass mutex(*_new_mutex);
	if (NewMutex == NULL) {
		NewMutex = CreateMutex(NULL, false, NULL);
		assert(NewMutex != NULL);
	}
	if (NewMutex) {
		WaitForSingleObject(NewMutex, INFINITE);
	}


	/*
	** Extract the return address. This is the default worst case amount of info we can get about the
	** stack.
	*/
	__asm
	{
		mov		eax,_locals_size
		mov		eax,[ebp+eax+4]
		mov		[_return_addr],eax
	}

	/*
	** Try using imagehlp.dll to walk the stack and get real return addresses for multiple calls.
	*/
	memset(ReturnAddresses, 0, WALK_FRAMES * sizeof(ReturnAddresses[0]));
	int num_frames = 0;

	/*
	** If this is an address we are looking for then do the stack walk.
	*/
#ifdef ONLY_WATCH_ADDRESSES
	for (_wa = 0 ; _wa<NUM_WATCH_ADDRESSES ; _wa++) {
		if (_return_addr == WatchAddresses[_wa]) {
			num_frames = Stack_Walk(ReturnAddresses, WALK_FRAMES, NULL);
			break;
		}
	}
#else	//ONLY_WATCH_ADDRESSES
	num_frames = Stack_Walk(ReturnAddresses, WALK_FRAMES, NULL);
#endif //ONLY_WATCH_ADDRESSES

	if (num_frames == 0) {
		ReturnAddresses[0] = _return_addr;
	}

	/*
	** Find or create the address string.
	*/
	_index = -1;
	for (_i=0 ; _i<min(_address_strings, ARRAY_SIZE(NewAddressList)) ; _i++) {
		if (memcmp(NewAddressList[_i].Addresses, ReturnAddresses, WALK_FRAMES * sizeof(ReturnAddresses[0])) == 0) {
		//if (NewAddressList[_i].Address == _return_addr) {
			_index = _i;
			break;
		}
	}

	if (_index == -1) {
		_index = _address_strings++;
	}

	if (_index >= ARRAY_SIZE(NewAddressList)) {
		void *newmem = ::operator new(s, 1, __FILE__, _address_strings);
		if (NewMutex) {
			ReleaseMutex(NewMutex);
		}
		return (newmem);
	}

	/*
	** If we got no good stack info and just have the return address then print that into this
	** entry.
	*/
	memcpy(NewAddressList[_index].Addresses, ReturnAddresses, WALK_FRAMES * sizeof(ReturnAddresses[0]));
	if (num_frames == 0) {
		//NewAddressList[_index].Address = _return_addr;
		NewAddressList[_index].AddressLine = ReturnAddresses[0];
		sprintf(NewAddressList[_index].AddressName, "Addr: %08X ", ReturnAddresses[0]);
	} else {

		/*
		** Otherwise try and get symbol info for each return address.
		*/
		char *ptr = NewAddressList[_index].AddressName;

		bool ok = true;
		for (_i=0 ; _i<WALK_FRAMES ; _i++) {
			if (!ok) {
				break;
			}
			_temp_addr = NewAddressList[_index].Addresses[_i];
			_displacement = 0;
			_temp_str[0] = 0;
			ok = Lookup_Symbol((void*)_temp_addr, _temp_str, _displacement);
			if (_i == 0) {
				sprintf(ptr, "\nAddr: %08X (%s + %d)\n", NewAddressList[_index].Addresses[_i], _temp_str, _displacement);
			} else {
				strcpy(_compose_str, &_spaces[(sizeof(_spaces) -2) -_i]);
				//_compose_str[0] = 0;
				//for (_j=0 ; _j<_i ; _j++) {
				//	strcat(_compose_str, " ");
				//}
				sprintf(_compose_str + strlen(_compose_str), "Called from: %08X (%s + %d)\n", NewAddressList[_index].Addresses[_i], _temp_str, _displacement);
				if (strlen(ptr) + strlen(_compose_str) >= sizeof(NewAddressList[_index].AddressName)) {
					break;
				}
				strcat(ptr, _compose_str);
			}
		}
	}

	assert(strlen(NewAddressList[_index].AddressName) < sizeof(NewAddressList[_index].AddressName));

	/*
	** Call the CRT new to actually allocate the memory. Pass in the string we composed.
	*/
	void *newmem = ::operator new(s, 1, NewAddressList[_index].AddressName, NewAddressList[_index].AddressLine);
	if (NewMutex) {
		ReleaseMutex(NewMutex);
	}
	return(newmem);
	//return (::operator new(s, 1, __FILE__, __LINE__));
}


#endif	//STEVES_NEW_CATCHER
#endif	//_DEBUG


#if (0)
/*
** Definitions to allow run-time linking to the Imagehlp.dll functions.
**
*/
typedef BOOL  (WINAPI *SymCleanupType) (HANDLE hProcess);
typedef BOOL  (WINAPI *SymGetSymFromAddrType) (HANDLE hProcess, DWORD Address, LPDWORD Displacement, PIMAGEHLP_SYMBOL Symbol);
typedef BOOL  (WINAPI *SymInitializeType) (HANDLE hProcess, LPSTR UserSearchPath, BOOL fInvadeProcess);
typedef BOOL  (WINAPI *SymLoadModuleType) (HANDLE hProcess, HANDLE hFile, LPSTR ImageName, LPSTR ModuleName, DWORD BaseOfDll, DWORD SizeOfDll);
typedef DWORD (WINAPI *SymSetOptionsType) (DWORD SymOptions);
typedef BOOL  (WINAPI *SymUnloadModuleType) (HANDLE hProcess, DWORD BaseOfDll);
typedef BOOL  (WINAPI *StackWalkType) (DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME StackFrame, LPVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE TranslateAddress);
typedef LPVOID (WINAPI *SymFunctionTableAccessType) (HANDLE hProcess, DWORD AddrBase);
typedef DWORD (WINAPI *SymGetModuleBaseType) (HANDLE hProcess, DWORD dwAddr);


SymCleanupType						_SymCleanup = NULL;
SymGetSymFromAddrType			_SymGetSymFromAddr = NULL;
SymInitializeType					_SymInitialize = NULL;
SymLoadModuleType					_SymLoadModule = NULL;
SymSetOptionsType					_SymSetOptions = NULL;
SymUnloadModuleType				_SymUnloadModule = NULL;
StackWalkType						_StackWalk = NULL;
SymFunctionTableAccessType		_SymFunctionTableAccess = NULL;
SymGetModuleBaseType				_SymGetModuleBase = NULL;

static char const *ImagehelpFunctionNames[] = {
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


bool SymbolsAvailable = false;
HINSTANCE ImageHelp = (HINSTANCE) -1;



/***********************************************************************************************
 * Load_Image_Helper -- Load imagehlp.dll and retrieve the programs symbols                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/12/2001 4:27PM ST : Created                                                             *
 *=============================================================================================*/
void Load_Image_Helper(void)
{
	/*
	** If this is the first time through then fix up the imagehelp function pointers since imagehlp.dll
	** can't be statically linked.
	*/
	if (ImageHelp == (HINSTANCE)-1) {
		ImageHelp = LoadLibrary("IMAGEHLP.DLL");

		if (ImageHelp != NULL) {
			char const *function_name = NULL;
			unsigned long *fptr = (unsigned long *) &_SymCleanup;
			int count = 0;

			do {
				function_name = ImagehelpFunctionNames[count];
				if (function_name) {
					*fptr = (unsigned long) GetProcAddress(ImageHelp, function_name);
					fptr++;
					count++;
				}
			}
			while (function_name);
		}

		/*
		** Retrieve the programs symbols if they are available. This can be a .pdb or a .dbg file.
		*/
		if (_SymSetOptions != NULL) {
			_SymSetOptions(SYMOPT_DEFERRED_LOADS);
		}

		int symload = 0;

		if (_SymInitialize != NULL && _SymInitialize(GetCurrentProcess(), NULL, false)) {

			if (_SymSetOptions != NULL) {
				_SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME);
			}

			char exe_name[_MAX_PATH];
			GetModuleFileName(NULL, exe_name, sizeof(exe_name));

			if (_SymLoadModule != NULL) {
				symload = _SymLoadModule(GetCurrentProcess(), NULL, exe_name, NULL, 0, 0);
			}

			if (symload) {
				SymbolsAvailable = true;
			} else {
				//assert (_SymLoadModule != NULL);
				//DebugString ("SymLoad failed for module %s with code %d - %s\n", szModuleName, GetLastError(), Last_Error_Text());
			}
		}
	}
}







/***********************************************************************************************
 * Lookup_Symbol -- Get the symbol for a given code address                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address of code to get symbol for                                                 *
 *           Ptr to buffer to return symbol in                                                 *
 *           Reference to int to return displacement                                           *
 *                                                                                             *
 * OUTPUT:   True if symbol found                                                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/12/2001 4:47PM ST : Created                                                             *
 *=============================================================================================*/
bool Lookup_Symbol(void *code_ptr, char *symbol, int &displacement)
{
	/*
	** Locals.
	*/
	char symbol_struct_buf[1024];
	IMAGEHLP_SYMBOL *symbol_struct_ptr = (IMAGEHLP_SYMBOL *)symbol_struct_buf;

	/*
	** Set default values in case of early exit.
	*/
	displacement = 0;
	*symbol = '\0';

	/*
	** Make sure symbols are available.
	*/
	if (!SymbolsAvailable || _SymGetSymFromAddr == NULL) {
		return(false);
	}

	/*
	** If it's a bad code pointer then there is no point in trying to match it with a symbol.
	*/
	if (IsBadCodePtr((FARPROC)code_ptr)) {
		strcpy(symbol, "Bad code pointer");
		return(false);
	}

	/*
	** Set up the parameters for the call to SymGetSymFromAddr
	*/
	memset (symbol_struct_ptr, 0, sizeof (symbol_struct_buf));
	symbol_struct_ptr->SizeOfStruct = sizeof (symbol_struct_buf);
	symbol_struct_ptr->MaxNameLength = sizeof(symbol_struct_buf)-sizeof (IMAGEHLP_SYMBOL);
	symbol_struct_ptr->Size = 0;
	symbol_struct_ptr->Address = (unsigned long)code_ptr;

	/*
	** See if we have the symbol for that address.
	*/
	if (_SymGetSymFromAddr(GetCurrentProcess(), (unsigned long)code_ptr, (unsigned long *)&displacement, symbol_struct_ptr)) {

		/*
		** Copy it back into the buffer provided.
		*/
		strcpy(symbol, symbol_struct_ptr->Name);
		return(true);
	}
	return(false);
}




/***********************************************************************************************
 * Stack_Walk -- Walk the stack and get the last n return addresses                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to return address list                                                        *
 *           Number of return addresses to fetch                                               *
 *           Ptr to optional context. NULL means use current                                   *
 *                                                                                             *
 * OUTPUT:   Number of return addresses found                                                  *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/12/2001 11:57AM ST : Created                                                            *
 *=============================================================================================*/
int Stack_Walk(unsigned long *return_addresses, int num_addresses, CONTEXT *context)
{
	static HINSTANCE _imagehelp = (HINSTANCE) -1;

	/*
	** If this is the first time through then fix up the imagehelp function pointers since imagehlp.dll
	** can't be statically linked.
	*/
	if (ImageHelp == (HINSTANCE)-1) {
		Load_Image_Helper();
	}

	/*
	** If there is no debug support .dll available then we can't walk the stack.
	*/
	if (ImageHelp == NULL) {
		return(0);
	}

	/*
	** Set up the stack frame structure for the start point of the stack walk (i.e. here).
	*/
	STACKFRAME stack_frame;
	memset(&stack_frame, 0, sizeof(stack_frame));

	unsigned long reg_eip, reg_ebp, reg_esp;

	__asm {
here:
		lea	eax,here
		mov	reg_eip,eax
		mov	reg_ebp,ebp
		mov	reg_esp,esp
	}

	stack_frame.AddrPC.Mode = AddrModeFlat;
	stack_frame.AddrPC.Offset = reg_eip;
	stack_frame.AddrStack.Mode = AddrModeFlat;
	stack_frame.AddrStack.Offset = reg_esp;
	stack_frame.AddrFrame.Mode = AddrModeFlat;
	stack_frame.AddrFrame.Offset = reg_ebp;

	/*
	** Use the context struct if it was provided.
	*/
	if (context) {
		stack_frame.AddrPC.Offset = context->Eip;
		stack_frame.AddrStack.Offset = context->Esp;
		stack_frame.AddrFrame.Offset = context->Ebp;
	}

	int pointer_index = 0;

	/*
	** Walk the stack by the requested number of return address iterations.
	*/
	for (int i = 0; i < num_addresses + 1; i++) {
		if (_StackWalk(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &stack_frame, NULL, NULL, _SymFunctionTableAccess, _SymGetModuleBase, NULL)) {

			/*
			** First result will always be the return address we were called from.
			*/
			if (i==0 && context == NULL) {
				continue;
			}
			unsigned long return_address = stack_frame.AddrReturn.Offset;
			return_addresses[pointer_index++] = return_address;
		} else {
			break;
		}
	}

	return(pointer_index);
}

#endif	//(0)
