/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 OpenW3D Contributors.
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
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SOUNDBUFFER_H
#define __SOUNDBUFFER_H

#include "refcount.h"


// Forward declarations
class FileClass;

/////////////////////////////////////////////////////////////////////////////////
//
//	SoundBufferClass
//
//	A sound buffer manages the raw sound data for any of the SoundObj types
// except for the StreamSoundClass object.
//
class SoundBufferClass : public RefCountClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		virtual ~SoundBufferClass (void) {}

		//////////////////////////////////////////////////////////////////////
		//	Public operators
		//////////////////////////////////////////////////////////////////////
		operator unsigned char * (void)							{ return Get_Raw_Buffer (); }

		//////////////////////////////////////////////////////////////////////
		//	File methods
		//////////////////////////////////////////////////////////////////////
		virtual bool				Load_From_File (const char *filename) = 0;
		virtual bool				Load_From_File (FileClass &file) = 0;

		//////////////////////////////////////////////////////////////////////
		//	Memory methods
		//////////////////////////////////////////////////////////////////////
		virtual bool				Load_From_Memory (unsigned char *mem_buffer, unsigned int size) = 0;

		//////////////////////////////////////////////////////////////////////
		//	Buffer access
		//////////////////////////////////////////////////////////////////////		
		virtual unsigned char *	Get_Raw_Buffer (void) const = 0;
		virtual unsigned int	Get_Raw_Length (void) const = 0;

		//////////////////////////////////////////////////////////////////////
		//	Information methods
		//////////////////////////////////////////////////////////////////////
		virtual const char *		Get_Filename (void) const = 0;
		virtual void				Set_Filename (const char *name) = 0;
		virtual unsigned int	Get_Duration (void) const = 0;
		virtual unsigned int	Get_Rate (void) const = 0;
		virtual unsigned int	Get_Bits (void) const = 0;
		virtual unsigned int	Get_Channels (void) const = 0;
		virtual unsigned int	Get_Type (void) const = 0;

		//////////////////////////////////////////////////////////////////////
		//	Type methods
		//////////////////////////////////////////////////////////////////////
		virtual bool				Is_Streaming (void) const = 0;

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////
		virtual void			Free_Buffer (void) = 0;
		virtual void			Determine_Stats (unsigned char *buffer) = 0;
};

#endif //__SOUNDBUFFER_H
