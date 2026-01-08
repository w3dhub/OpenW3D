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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/floodprotectionmgr.h                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/10/02 11:39a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __FLOODPROTECTIONMGR_H
#define __FLOODPROTECTIONMGR_H


#include "widestring.h"
#include "bittype.h"
#include "simplevec.h"


//////////////////////////////////////////////////////////////////////
//
//	FloodProtectionMgrClass
//
//////////////////////////////////////////////////////////////////////
class FloodProtectionMgrClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	
	//
	//	Initialization
	//
	static void		Reset (void);

	//
	//	Flood determination
	//
	static bool		Detect_Flooding (const unichar_t *text);
	
private:
	
	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////
	static void		Decay_Old_Entries (void);
	static void		Display_Flood_Message (void);
	
	///////////////////////////////////////////////////////////////////
	//	Private structures
	///////////////////////////////////////////////////////////////////
	typedef struct
	{
		int		text_len;
		uint32	time;
	} FLOOD_ENTRY;
	

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	static SimpleDynVecClass<FLOOD_ENTRY>	FloodList;
};


#endif //__FLOODPROTECTIONMGR_H
