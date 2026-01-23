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
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/SoundBuffer.h                        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/17/01 11:12a                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MILESSOUNDBUFFER_H
#define __MILESSOUNDBUFFER_H

#include "SoundBuffer.h"


// Forward declarations
class FileClass;

/////////////////////////////////////////////////////////////////////////////////
//
//	SingleSoundBufferClass
//
//	A sound buffer manages the raw sound data for any of the SoundObj types
// except for the StreamSoundClass object.
//
class SingleSoundBufferClass : public SoundBufferClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		SingleSoundBufferClass (void);
		~SingleSoundBufferClass (void) override;

		//////////////////////////////////////////////////////////////////////
		//	File methods
		//////////////////////////////////////////////////////////////////////
		bool				Load_From_File (const char *filename) override;
		bool				Load_From_File (FileClass &file) override;

		//////////////////////////////////////////////////////////////////////
		//	Memory methods
		//////////////////////////////////////////////////////////////////////
		bool				Load_From_Memory (unsigned char *mem_buffer, unsigned int size) override;

		//////////////////////////////////////////////////////////////////////
		//	Buffer access
		//////////////////////////////////////////////////////////////////////		
		unsigned char *	Get_Raw_Buffer (void) const override	{ return m_Buffer; }
		unsigned int	Get_Raw_Length (void) const override	{ return m_Length; }

		//////////////////////////////////////////////////////////////////////
		//	Information methods
		//////////////////////////////////////////////////////////////////////
		const char *		Get_Filename (void) const override{ return m_Filename; }
		void				Set_Filename (const char *name) override;
		unsigned int	Get_Duration (void) const override{ return m_Duration; }
		unsigned int	Get_Rate (void) const override { return m_Rate; }
		unsigned int	Get_Bits (void) const override { return m_Bits; }
		unsigned int	Get_Channels (void) const override { return m_Channels; }
		unsigned int	Get_Type (void) const override{ return m_Type; }

		//////////////////////////////////////////////////////////////////////
		//	Type methods
		//////////////////////////////////////////////////////////////////////
		bool				Is_Streaming (void) const override	{ return false; }

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////
		void			Free_Buffer (void) override;
		void			Determine_Stats (unsigned char *buffer) override;

		//////////////////////////////////////////////////////////////////////
		//	Protected member data
		//////////////////////////////////////////////////////////////////////		
		unsigned char *		m_Buffer;
		unsigned int			m_Length;
		char *					m_Filename;
		unsigned int			m_Duration;
		unsigned int			m_Rate;
		unsigned int			m_Bits;
		unsigned int			m_Channels;
		unsigned int			m_Type;
};

/////////////////////////////////////////////////////////////////////////////////
//
//	StreamSoundBufferClass
//
//	A sound buffer manages the raw sound data for any of the SoundObj types
// except for the StreamSoundClass object.
//
class StreamSoundBufferClass : public SingleSoundBufferClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		StreamSoundBufferClass (void);
		~StreamSoundBufferClass (void);

		//////////////////////////////////////////////////////////////////////
		//	File methods
		//////////////////////////////////////////////////////////////////////
		bool			Load_From_File (const char *filename) override;
		bool			Load_From_File (FileClass &file) override;

		//////////////////////////////////////////////////////////////////////
		//	Memory methods
		//////////////////////////////////////////////////////////////////////
		bool			Load_From_Memory (unsigned char * /* mem_buffer */, unsigned int /* size */) override { return false;  }

		//////////////////////////////////////////////////////////////////////
		//	Type methods
		//////////////////////////////////////////////////////////////////////
		bool			Is_Streaming (void) const override		{ return true; }

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////
		void			Free_Buffer (void) override;

		//////////////////////////////////////////////////////////////////////
		//	Protected member data
		//////////////////////////////////////////////////////////////////////		
};


#endif //__MILESSOUNDBUFFER_H
