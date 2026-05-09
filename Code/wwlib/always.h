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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/always.h                               $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/28/01 3:21p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef ALWAYS_H
#define ALWAYS_H

/*
** Define for debug memory allocation to include __FILE__ and __LINE__ for every memory allocation.
** This helps find leaks.
*/
//#define STEVES_NEW_CATCHER
#ifdef _DEBUG
#ifdef STEVES_NEW_CATCHER

#ifdef _MSC_VER
#include	<crtdbg.h>
#include <stdlib.h>
#include <malloc.h>

#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   calloc(c, s)      _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   _expand(p, s)     _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   free(p)           _free_dbg(p, _NORMAL_BLOCK)
#define   _msize(p)         _msize_dbg(p, _NORMAL_BLOCK)
#endif	//_MSC_VER

#include <new>

void* operator new(size_t s);

void operator delete(void *ptr) noexcept;

void operator delete(void *p, size_t size) noexcept;

#endif	//STEVES_NEW_CATCHER
#endif	//_DEBUG

// Provide a portable definition for __forceinline when not compiling with MSVC.
#if !defined(_MSC_VER) && !defined(__forceinline)
	#if defined(_WIN32) && (defined(__clang__) || defined(__GNUC__))
		#define __forceinline inline [[msvc::forceinline]]
	#elif defined(__GNUC__) || defined(__clang__)
		#define __forceinline inline __attribute__((always_inline))
	#else
		#define __forceinline inline
	#endif
#endif

// Jani: MSVC doesn't necessarily inline code with inline keyword. Using __forceinline results better inlining
// and also prints out a warning if inlining wasn't possible. __forceinline is MSVC specific.
#if defined(_MSC_VER)
#define WWINLINE __forceinline
#else
#define WWINLINE inline
#endif

/*
** Define the MIN and MAX macros.
** NOTE: Joe used to #include <minmax.h> in the various compiler header files.  This
** header defines 'min' and 'max' macros which conflict with the surrender code so
** I'm relpacing all occurances of 'min' and 'max with 'MIN' and 'MAX'.  For code which
** is out of our domain (e.g. Max sdk) I'm declaring template functions for 'min' and 'max'
*/

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#if defined(_MSC_VER)
/*
**	Turn off some unneeded warnings.
*/
// "conversion from 'double' to 'float', possible loss of data" Yea, so what?
#pragma warning(disable : 4244)
#endif

#ifndef	NULL
	#define	NULL		0
#endif

/**********************************************************************
**	This macro serves as a general way to determine the number of elements
**	within an array.
*/
#ifndef ARRAY_SIZE
#define	ARRAY_SIZE(x)		int(sizeof(x)/sizeof(x[0]))
#endif

#ifndef size_of
#define size_of(typ,id) sizeof(((typ*)0)->id)
#endif


#if !defined(_WIN32)
#include <bit>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <strings.h>
#include <alloca.h>

#ifndef ZeroMemory
#define ZeroMemory(Destination, Length) std::memset((Destination), 0, (Length))
#endif

#ifndef CopyMemory
#define CopyMemory(Destination, Source, Length) std::memcpy((Destination), (Source), (Length))
#endif

#ifndef _rotl
__forceinline unsigned long _rotl(unsigned long value, int shift)
{
	return std::rotl(value, shift);
}
#endif

#ifndef _byteswap_ulong
__forceinline unsigned int _byteswap_ulong(unsigned int value)
{
	return __builtin_bswap32(value);
}
#endif

#ifndef _alloca
#define _alloca(size) alloca(size)
#endif

#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif

#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif

#ifndef _MAX_DRIVE
#define _MAX_DRIVE 4
#endif

#ifndef _MAX_DIR
#define _MAX_DIR 1024
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

#ifndef _splitpath
inline void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
	if (drive != NULL) {
		drive[0] = '\0';
	}
	if (dir != NULL) {
		dir[0] = '\0';
	}
	if (fname != NULL) {
		fname[0] = '\0';
	}
	if (ext != NULL) {
		ext[0] = '\0';
	}

	if (path == NULL || path[0] == '\0') {
		return;
	}

	const char *last_forward_slash = std::strrchr(path, '/');
	const char *last_back_slash = std::strrchr(path, '\\');
	const char *last_separator = last_forward_slash;
	if (last_separator == NULL || (last_back_slash != NULL && last_back_slash > last_separator)) {
		last_separator = last_back_slash;
	}

	const char *basename = (last_separator != NULL) ? (last_separator + 1) : path;
	const char *dot = std::strrchr(basename, '.');
	if (dot == basename) {
		dot = NULL;
	}

	if (dir != NULL && last_separator != NULL) {
		size_t dir_len = static_cast<size_t>((last_separator - path) + 1);
		std::memcpy(dir, path, dir_len);
		dir[dir_len] = '\0';
	}

	if (fname != NULL) {
		size_t fname_len = (dot != NULL) ? static_cast<size_t>(dot - basename) : std::strlen(basename);
		std::memcpy(fname, basename, fname_len);
		fname[fname_len] = '\0';
	}

	if (ext != NULL && dot != NULL) {
		std::strcpy(ext, dot);
	}
}
#endif

#ifndef strcmpi
#define strcmpi strcasecmp
#endif

#ifndef _strcmpi
#define _strcmpi strcasecmp
#endif

#ifndef lstrcmpi
#define lstrcmpi strcasecmp
#endif

#ifndef SYSTEMTIME
typedef struct _SYSTEMTIME {
	unsigned short wYear;
	unsigned short wMonth;
	unsigned short wDayOfWeek;
	unsigned short wDay;
	unsigned short wHour;
	unsigned short wMinute;
	unsigned short wSecond;
	unsigned short wMilliseconds;
} SYSTEMTIME;
#endif

#ifndef GetSystemTime
inline void GetSystemTime(SYSTEMTIME *system_time)
{
	if (system_time == NULL) {
		return;
	}

	timespec current_time = { 0, 0 };
	clock_gettime(CLOCK_REALTIME, &current_time);
	tm utc_time = { 0 };
	gmtime_r(&current_time.tv_sec, &utc_time);

	system_time->wYear = static_cast<unsigned short>(utc_time.tm_year + 1900);
	system_time->wMonth = static_cast<unsigned short>(utc_time.tm_mon + 1);
	system_time->wDayOfWeek = static_cast<unsigned short>(utc_time.tm_wday);
	system_time->wDay = static_cast<unsigned short>(utc_time.tm_mday);
	system_time->wHour = static_cast<unsigned short>(utc_time.tm_hour);
	system_time->wMinute = static_cast<unsigned short>(utc_time.tm_min);
	system_time->wSecond = static_cast<unsigned short>(utc_time.tm_sec);
	system_time->wMilliseconds = static_cast<unsigned short>(current_time.tv_nsec / 1000000);
}
#endif

#ifndef OutputDebugStringA
inline void OutputDebugStringA(const char *message)
{
	if (message != NULL) {
		std::fputs(message, stderr);
	}
}
#endif

#ifndef DeleteFileA
inline int DeleteFileA(const char *filename)
{
	return (filename != NULL && std::remove(filename) == 0) ? 1 : 0;
}
#endif

#ifndef MoveFileA
inline int MoveFileA(const char *existing_filename, const char *new_filename)
{
	return (existing_filename != NULL && new_filename != NULL && std::rename(existing_filename, new_filename) == 0) ? 1 : 0;
}
#endif

//---------------------------------------------------------------------------
// Registry API stubs (needed by WWOnline/WOLProduct.cpp)
//---------------------------------------------------------------------------
#ifndef HKEY_CURRENT_USER
#define HKEY_CURRENT_USER reinterpret_cast<void*>(static_cast<uintptr_t>(0x80000001))
#endif

#ifndef KEY_READ
#define KEY_READ 0x20019
#endif

#ifndef KEY_WRITE
#define KEY_WRITE 0x20006
#endif

#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0
#endif

#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND 2
#endif

#ifndef RegOpenKeyExA
inline int RegOpenKeyExA(
        void* /*hKey*/,
        const char* /*lpSubKey*/,
        uint32_t /*ulOptions*/,
        uint32_t /*samDesired*/,
        void** /*phkResult*/)
{
	return ERROR_FILE_NOT_FOUND; // Registry not available on non-Windows
}
#endif

#ifndef RegCloseKey
inline int RegCloseKey(void* /*hKey*/)
{
	return ERROR_SUCCESS;
}
#endif

#ifndef RegQueryValueExA
inline int RegQueryValueExA(
        void* /*hKey*/,
        const char* /*lpValueName*/,
        uint32_t* /*lpReserved*/,
        uint32_t* /*lpType*/,
        uint8_t* /*lpData*/,
        uint32_t* /*lpcbData*/)
{
	return ERROR_FILE_NOT_FOUND;
}
#endif

#ifndef RegSetValueExA
inline int RegSetValueExA(
        void* /*hKey*/,
        const char* /*lpValueName*/,
        uint32_t /*Reserved*/,
        uint32_t /*dwType*/,
        const uint8_t* /*lpData*/,
        uint32_t /*cbData*/)
{
	return ERROR_SUCCESS;
}
#endif

#ifndef ERROR_NO_MORE_ITEMS
#define ERROR_NO_MORE_ITEMS 259
#endif

#ifndef RegEnumValueA
inline int RegEnumValueA(
        void* /*hKey*/,
        uint32_t /*dwIndex*/,
        char* /*lpValueName*/,
        uint32_t* /*lpcchValueName*/,
        uint32_t* /*lpReserved*/,
        uint32_t* /*lpType*/,
        uint8_t* /*lpData*/,
        uint32_t* /*lpcbData*/)
{
	return ERROR_NO_MORE_ITEMS;
}
#endif

#endif // !_WIN32

#endif
