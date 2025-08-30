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
 *                 Project Name : wwaudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/listenerhandle.h                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/13/01 11:54a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __LISTENERHANDLE_H
#define __LISTENERHANDLE_H

#include "sound3dhandle.h"


//////////////////////////////////////////////////////////////////////
//
//	ListenerHandleClass
//
//////////////////////////////////////////////////////////////////////
class ListenerHandleClass : public Sound3DHandleClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	ListenerHandleClass  (void);
	~ListenerHandleClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	RTTI
	//
	ListenerHandleClass *	As_ListenerHandleClass (void) override		{ return this; }

	//
	//	Inherited
	//
	void							Initialize (SoundBufferClass *buffer) override;
	void							Start_Sample (void) override									{ }
	void							Stop_Sample (void) override									{ }
	void							Resume_Sample (void) override									{ }
	void							End_Sample (void) override										{ }
	void							Set_Sample_Volume (S32 volume) override					{ }
	S32							Get_Sample_Volume (void) override							{ return 0; }
	void							Set_Sample_Pan (S32 pan) override							{ }
	S32							Get_Sample_Pan (void) override								{ return 64; }
	void							Set_Sample_Loop_Count (U32 count) override				{ }
	U32							Get_Sample_Loop_Count (void) override						{ return 0; }
	void							Set_Sample_MS_Position (U32 ms) override					{ }
	void							Get_Sample_MS_Position (S32 *len, S32 *pos) override	{ }
	S32							Get_Sample_Playback_Rate (void) override					{ return 0; }
	void							Set_Sample_Playback_Rate (S32 rate) override				{ }
	
protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
};


#endif //__LISTENERHANDLE_H
