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
*     DPrint.h
*
* DESCRIPTION
*     Debug printing mechanism
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Denzil_l $
*     $Revision: 1 $
*     $Modtime: 3/24/00 1:27p $
*     $Archive: /Commando/Code/Scripts/DPrint.h $
*
****************************************************************************/

#ifndef _DPRINT_H_
#define _DPRINT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _DEBUG
//! Ouput debug print messages to the debugger and log file.
void DebugPrint(const char* string, ...);
#else
#define DebugPrint
#endif


#ifdef __cplusplus
}
#endif

#endif // _DPRINT_H_
