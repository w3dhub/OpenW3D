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
#include <cstring>
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

#ifndef OutputDebugStringA
#define OutputDebugStringA(s) ((void)(s))
#endif

// Windows BOOL type
#ifndef BOOL
typedef int BOOL;
#endif

// Windows path constant equivalents
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif
#ifndef _MAX_DIR
#define _MAX_DIR 260
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif

// _splitpath: split a path into drive/dir/fname/ext components
static inline void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
	if (drive) drive[0] = '\0'; // no drive letters on Linux
	const char *last_slash = strrchr(path, '/');
	const char *name_start = last_slash ? last_slash + 1 : path;
	const char *last_dot = strrchr(name_start, '.');
	if (dir) {
		if (last_slash) {
			size_t n = (size_t)(last_slash - path) + 1;
			std::memcpy(dir, path, n);
			dir[n] = '\0';
		} else {
			dir[0] = '\0';
		}
	}
	if (fname) {
		size_t n = last_dot ? (size_t)(last_dot - name_start) : std::strlen(name_start);
		std::memcpy(fname, name_start, n);
		fname[n] = '\0';
	}
	if (ext) {
		if (last_dot) {
			std::strcpy(ext, last_dot);
		} else {
			ext[0] = '\0';
		}
	}
}

#endif // !_WIN32

#endif
