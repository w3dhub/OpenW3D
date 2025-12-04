/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**	Copyright 2026 OpenW3D Contributors.
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

#ifndef __FFMPEGBUFFER_H
#define __FFMPEGBUFFER_H

#include "SoundBuffer.h"
#include "FFmpegFile.h"
#include <vector>


// Forward declarations
class FileClass;

/////////////////////////////////////////////////////////////////////////////////
//
//	SingleSoundBufferClass
//
//	A sound buffer manages the raw sound data for any of the SoundObj types
// except for the StreamSoundClass object.
//
class FFMpegBufferClass : public SoundBufferClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		FFMpegBufferClass (void);
		~FFMpegBufferClass (void) override;

		//////////////////////////////////////////////////////////////////////
		//	File methods
		//////////////////////////////////////////////////////////////////////
		bool				Load_From_File (const char *filename) override;
		bool				Load_From_File (FileClass &/* file */) override { return false; }

		//////////////////////////////////////////////////////////////////////
		//	Buffer access
		//////////////////////////////////////////////////////////////////////		
		unsigned char *	Get_Raw_Buffer (void) const override	{ return m_Buffer.data(); }
		unsigned int	Get_Raw_Length (void) const override	{ return m_Buffer.size(); }

		//////////////////////////////////////////////////////////////////////
		//	Information methods
		//////////////////////////////////////////////////////////////////////
		const char *		Get_Filename (void) const override{ return m_Filename; }
		void				Set_Filename (const char *name) override;
		unsigned int	Get_Duration (void) const override{ return m_Duration; }
		unsigned int	Get_Rate (void) const override { return m_Rate; }
		unsigned int	Get_Bits (void) const override { return m_Bits; }
		unsigned int	Get_Channels (void) const override { return m_Channels; }
		unsigned int	Get_Type (void) const override{ return 0; } // Unused as we don't care what the type is.

		//////////////////////////////////////////////////////////////////////
		//	Type methods
		//////////////////////////////////////////////////////////////////////
		bool				Is_Streaming (void) const override	{ return true; }

		bool 			Refresh_Buffer(void);
		void 			Reset_Buffer(void);
	protected:
		//////////////////////////////////////////////////////////////////////
		//	Protected member data
		//////////////////////////////////////////////////////////////////////
		FFmpegFile m_FileHandle;
		mutable std::vector<unsigned char>		m_Buffer;
		char *					m_Filename;
		unsigned int			m_Duration;
		unsigned int			m_Rate;
		unsigned int			m_Bits;
		unsigned int			m_Channels;
		unsigned int			m_MaxBuffer;
};


#endif //__FFMPEGBUFFER_H
