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
 *                     $Archive:: /Commando/Code/WWAudio/soundhandle.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/13/01 3:11p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SOUNDHANDLE_H
#define __SOUNDHANDLE_H

#include "WWAudio.h"

//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class Sound3DHandleClass;
class Sound2DHandleClass;
class SoundStreamHandleClass;
class SoundBufferClass;
class ListenerHandleClass;


//////////////////////////////////////////////////////////////////////
//
//	SoundHandleClass
//
//////////////////////////////////////////////////////////////////////
class SoundHandleClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	SoundHandleClass (void);
	virtual ~SoundHandleClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	RTTI
	//
	virtual Sound3DHandleClass *		As_Sound3DHandleClass (void)		{ return NULL; }
	virtual Sound2DHandleClass *		As_Sound2DHandleClass (void)		{ return NULL; }
	virtual SoundStreamHandleClass *	As_SoundStreamHandleClass (void)	{ return NULL; }
	virtual ListenerHandleClass *		As_ListenerHandleClass (void)		{ return NULL; }

	//
	//	Handle access
	//
	virtual H3DSAMPLE		Get_H3DSAMPLE (void)		{ return NULL; }
	virtual HSAMPLE		Get_HSAMPLE (void)		{ return NULL; }
	virtual HSTREAM		Get_HSTREAM (void)		{ return NULL; }

	//
	//	Initialization
	//	
	virtual void	Set_Miles_Handle (void *handle) = 0;
	virtual void	Initialize (SoundBufferClass *buffer);

	//
	//	Sample control
	//	
	virtual void	Start_Sample (void) = 0;
	virtual void	Stop_Sample (void) = 0;
	virtual void	Resume_Sample (void) = 0;
	virtual void	End_Sample (void) = 0;
	virtual void	Set_Sample_Pan (int pan) = 0;
	virtual int		Get_Sample_Pan (void) = 0;
	virtual void	Set_Sample_Volume (int volume) = 0;
	virtual int		Get_Sample_Volume (void) = 0;
	virtual void	Set_Sample_Loop_Count (unsigned count) = 0;
	virtual unsigned		Get_Sample_Loop_Count (void) = 0;
	virtual void	Set_Sample_MS_Position (unsigned ms) = 0;
	virtual void	Get_Sample_MS_Position (int *len, int *pos) = 0;
	virtual void	Set_Sample_User_Data (int i, void *val) = 0;
	virtual void *	Get_Sample_User_Data (int i) = 0;
	virtual int		Get_Sample_Playback_Rate (void) = 0;
	virtual void	Set_Sample_Playback_Rate (int rate) = 0;
	
protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	SoundBufferClass *	Buffer;
};


#endif //__SOUNDHANDLE_H
