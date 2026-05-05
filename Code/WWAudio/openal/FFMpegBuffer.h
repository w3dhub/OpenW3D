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

#include "FFmpegFile.h"
#include "SoundBuffer.h"

#include <vector>

class FileClass;
struct AVFrame;

class FFMpegAudioStreamClass
{
public:
	FFMpegAudioStreamClass();
	~FFMpegAudioStreamClass();

	bool Open(const char *filename);
	void Close();
	bool Rewind();
	bool Seek_MS(unsigned int ms);
	unsigned int Read(unsigned char *buffer, unsigned int buffer_size);

	bool Is_Open() const { return m_Open; }
	bool Is_EOF() const { return m_EOF && m_PendingOffset >= m_PendingData.size(); }
	unsigned int Get_Duration() const { return m_Duration; }
	unsigned int Get_Rate() const { return m_Rate; }
	unsigned int Get_Bits() const { return m_Bits; }
	unsigned int Get_Channels() const { return m_Channels; }

private:
	static void On_Frame(AVFrame *frame, int stream_idx, int stream_type, void *user_data);
	void Append_Frame(AVFrame *frame, int stream_type);
	void Compact_Pending();

private:
	FFmpegFile m_File;
	std::vector<unsigned char> m_PendingData;
	size_t m_PendingOffset;
	unsigned int m_Duration;
	unsigned int m_Rate;
	unsigned int m_Bits;
	unsigned int m_Channels;
	bool m_Open;
	bool m_EOF;
};

/////////////////////////////////////////////////////////////////////////////////
//
//	FFMpegBufferClass
//
//	A sound buffer either owns immutable decoded PCM for cached small sounds, or
//	describes a streaming source whose decoder state belongs to an audio handle.
//
class FFMpegBufferClass : public SoundBufferClass
{
public:
	FFMpegBufferClass(void);
	~FFMpegBufferClass(void) override;

	bool Load_From_File(const char *filename) override { return Load_From_File(filename, false); }
	bool Load_From_File(const char *filename, bool streaming);
	bool Load_From_File(FileClass &/* file */) override { return false; }

	unsigned char *Get_Raw_Buffer(void) const override
	{
		return m_Buffer.empty() ? nullptr : const_cast<unsigned char *>(m_Buffer.data());
	}
	unsigned int Get_Raw_Length(void) const override { return static_cast<unsigned int>(m_Buffer.size()); }

	const char *Get_Filename(void) const override { return m_Filename; }
	void Set_Filename(const char *name) override;
	unsigned int Get_Duration(void) const override { return m_Duration; }
	unsigned int Get_Rate(void) const override { return m_Rate; }
	unsigned int Get_Bits(void) const override { return m_Bits; }
	unsigned int Get_Channels(void) const override { return m_Channels; }
	unsigned int Get_Type(void) const override { return 1; }

	bool Is_Streaming(void) const override { return m_IsStreaming; }

private:
	char *m_Filename;
	std::vector<unsigned char> m_Buffer;
	unsigned int m_Duration;
	unsigned int m_Rate;
	unsigned int m_Bits;
	unsigned int m_Channels;
	bool m_IsStreaming;
};

#endif //__FFMPEGBUFFER_H
