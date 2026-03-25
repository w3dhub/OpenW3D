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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Launcher/Toolkit/Debug/DebugPrint.h $
*
* DESCRIPTION
*     Debug printing mechanism
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Modtime: 9/26/00 2:27a $
*     $Revision: 1 $
*
******************************************************************************/

#ifndef _DEBUGPRINT_H_
#define _DEBUGPRINT_H_

#ifdef _DEBUG

#ifdef __cplusplus
extern "C"
{
#endif

//! Ouput debug print messages to the debugger and log file.
void DebugPrint(const char* string, ...) OPENW3D_PRINTF_VARARG_FUNC(1);
void PrintWin32Error(const char* string, ...) OPENW3D_PRINTF_VARARG_FUNC(1);

#ifdef __cplusplus
}
#endif

#else // _DEBUG

#define DebugPrint
#define PrintWin32Error

#endif // _DEBUG

#endif // _DEBUGPRINT_H_
