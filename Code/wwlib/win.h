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
 *                     $Archive:: /Commando/Code/wwlib/win.h                                  $*
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/16/01 2:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef WIN_H
#define WIN_H

/*
**	This header file includes the Windows headers. If there are any special pragmas that need
**	to occur around this process, they are performed here. Typically, certain warnings will need
**	to be disabled since the Windows headers are repleat with illegal and dangerous constructs.
**
**	Within the windows headers themselves, Microsoft has disabled the warnings 4290, 4514,
**	4069, 4200, 4237, 4103, 4001, 4035, 4164. Makes you wonder, eh?
*/

#ifdef _WIN32
#include	<windows.h>
#endif
//#include <mmsystem.h>
//#include	<windowsx.h>
//#include	<winnt.h>
//#include	<winuser.h>

#ifdef _WIN32
extern HINSTANCE	ProgramInstance;
extern HWND			MainWindow;
extern bool GameInFocus;

#ifdef _DEBUG

void __cdecl Print_Win32Error(unsigned int win32Error);

#else // _DEBUG

#define Print_Win32Error

#endif // _DEBUG

#else // _WIN32
// Stub types and functions for non-Windows builds
#include "dxvk_wrapper/windows.h"
#include <cstring>

extern bool GameInFocus;
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif
#ifndef _MAX_DIR
#define _MAX_DIR 256
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif
#ifndef WAIT_TIMEOUT
#define WAIT_TIMEOUT 258
#endif
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((void*)(-1))
#endif
#ifndef HANDLE
typedef void *HANDLE;
#endif
#ifndef HWND
typedef void *HWND;
#endif
#ifndef HINSTANCE
typedef void *HINSTANCE;
#endif
#ifndef HDC
typedef void *HDC;
#endif
#ifndef HMODULE
typedef void *HMODULE;
#endif
#ifndef UINT
typedef unsigned int UINT;
#endif
#ifndef WPARAM
typedef long WPARAM;
#endif
#ifndef LPARAM
typedef long LPARAM;
#endif
#ifndef LRESULT
typedef long LRESULT;
#endif
typedef void *LPLOGFONT;
typedef void *HKL;
typedef void *LPSTR;
typedef const char *LPCSTR;
typedef void *GUID; // stub, incomplete
struct WIN32_FIND_DATAA {
    unsigned long dwFileAttributes;
    unsigned long ftCreationTime_lo;
    unsigned long ftCreationTime_hi;
    unsigned long ftLastAccessTime_lo;
    unsigned long ftLastAccessTime_hi;
    unsigned long ftLastWriteTime_lo;
    unsigned long ftLastWriteTime_hi;
    unsigned long nFileSize_lo;
    unsigned long nFileSize_hi;
    unsigned long dwReserved0;
    unsigned long dwReserved1;
    char cFileName[260];
    char cAlternateFileName[14];
};
#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#endif
static inline HANDLE FindFirstFileA(const char *, WIN32_FIND_DATAA *) { return nullptr; }
static inline int FindNextFileA(HANDLE, WIN32_FIND_DATAA *) { return 0; }
static inline int FindClose(HANDLE) { return 1; }
static inline int CloseHandle(HANDLE) { return 1; }
static inline int GetLastError() { return 0; }
static inline HANDLE CreateMutexA(void *, int, const char *) { return nullptr; }
static inline unsigned long WaitForSingleObject(HANDLE, unsigned int) { return 0; }
static inline int ReleaseMutex(HANDLE) { return 1; }
static inline unsigned long GetCurrentThreadId() { return 0; }
static inline void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext) {
    if (drive) { drive[0] = '\0'; }
    if (dir) {
        const char *last_slash = strrchr(path, '/');
        if (last_slash) {
            size_t dirlen = (last_slash - path) + 1;
            if (dirlen > _MAX_DIR - 1) dirlen = _MAX_DIR - 1;
            strncpy(dir, path, dirlen);
            dir[dirlen] = '\0';
        } else {
            dir[0] = '\0';
        }
    }
    if (fname) { fname[0] = '\0'; }
    if (ext) { ext[0] = '\0'; }
}
static inline int DeleteFileA(const char *) { return 0; }
static inline int MoveFileA(const char *, const char *) { return 0; }
#endif // _WIN32

#endif // WIN_H
