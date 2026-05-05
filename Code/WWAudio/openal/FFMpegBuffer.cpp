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

#include "FFMpegBuffer.h"

#include "Utils.h"
#include "wwdebug.h"
#include "wwprofile.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
}

namespace
{
	constexpr unsigned int PCM_BITS = 16;

	unsigned int Calculate_Duration_MS(unsigned int bytes, unsigned int rate, unsigned int channels, unsigned int bits)
	{
		const unsigned int bytes_per_second = rate * channels * (bits / 8);
		if (bytes_per_second == 0) {
			return 0;
		}

		return static_cast<unsigned int>((static_cast<uint64_t>(bytes) * 1000ULL) / bytes_per_second);
	}
}

/////////////////////////////////////////////////////////////////////////////////
//
//	FFMpegAudioStreamClass
//
FFMpegAudioStreamClass::FFMpegAudioStreamClass()
	: m_PendingOffset(0),
	  m_Duration(0),
	  m_Rate(0),
	  m_Bits(PCM_BITS),
	  m_Channels(0),
	  m_Open(false),
	  m_EOF(false)
{
}

FFMpegAudioStreamClass::~FFMpegAudioStreamClass()
{
	Close();
}

bool FFMpegAudioStreamClass::Open(const char *filename)
{
	Close();

	if (!m_File.Open(filename) || !m_File.Has_Audio()) {
		Close();
		return false;
	}

	m_File.Set_Frame_Callback(&FFMpegAudioStreamClass::On_Frame);
	m_File.Set_User_Data(this);

	m_Duration = static_cast<unsigned int>(std::max(m_File.Get_Duration(), 0));
	m_Rate = static_cast<unsigned int>(std::max(m_File.Get_Sample_Rate(), 0));
	const int source_channels = m_File.Get_Num_Channels();
	m_Channels = source_channels == 1 ? 1u : (source_channels > 1 ? 2u : 0u);
	m_Bits = PCM_BITS;
	m_Open = true;
	m_EOF = false;
	m_PendingData.clear();
	m_PendingOffset = 0;
	return true;
}

void FFMpegAudioStreamClass::Close()
{
	m_File.Close();
	m_PendingData.clear();
	m_PendingOffset = 0;
	m_Duration = 0;
	m_Rate = 0;
	m_Bits = PCM_BITS;
	m_Channels = 0;
	m_Open = false;
	m_EOF = false;
}

bool FFMpegAudioStreamClass::Rewind()
{
	if (!m_Open) {
		return false;
	}

	m_File.Rewind();
	m_PendingData.clear();
	m_PendingOffset = 0;
	m_EOF = false;
	return true;
}

bool FFMpegAudioStreamClass::Seek_MS(unsigned int ms)
{
	if (!Rewind()) {
		return false;
	}

	if (m_Rate == 0 || m_Channels == 0 || m_Bits == 0) {
		return ms == 0;
	}

	const unsigned int bytes_per_sample = m_Channels * (m_Bits / 8);
	uint64_t target_bytes = (static_cast<uint64_t>(ms) * m_Rate * bytes_per_sample) / 1000ULL;
	target_bytes -= target_bytes % bytes_per_sample;
	std::vector<unsigned char> scratch(16384);
	uint64_t skipped = 0;
	while (skipped < target_bytes) {
		const unsigned int to_read = static_cast<unsigned int>(std::min<uint64_t>(scratch.size(), target_bytes - skipped));
		const unsigned int read = Read(scratch.data(), to_read);
		if (read == 0) {
			break;
		}
		skipped += read;
	}

	return true;
}

unsigned int FFMpegAudioStreamClass::Read(unsigned char *buffer, unsigned int buffer_size)
{
	if (!m_Open || buffer == nullptr || buffer_size == 0) {
		return 0;
	}

	unsigned int written = 0;
	while (written < buffer_size) {
		if (m_PendingOffset < m_PendingData.size()) {
			const size_t available = m_PendingData.size() - m_PendingOffset;
			const size_t to_copy = std::min<size_t>(available, buffer_size - written);
			std::memcpy(buffer + written, m_PendingData.data() + m_PendingOffset, to_copy);
			m_PendingOffset += to_copy;
			written += static_cast<unsigned int>(to_copy);
			Compact_Pending();
			continue;
		}

		if (m_EOF || !m_File.Decode_Packet()) {
			m_EOF = true;
			break;
		}
	}

	return written;
}

void FFMpegAudioStreamClass::On_Frame(AVFrame *frame, int /* stream_idx */, int stream_type, void *user_data)
{
	FFMpegAudioStreamClass *stream = static_cast<FFMpegAudioStreamClass *>(user_data);
	if (stream != nullptr) {
		stream->Append_Frame(frame, stream_type);
	}
}

void FFMpegAudioStreamClass::Append_Frame(AVFrame *frame, int stream_type)
{
	if (stream_type != AVMEDIA_TYPE_AUDIO) {
		return;
	}

	std::vector<unsigned char> converted;
	unsigned int rate = 0;
	unsigned int channels = 0;
	unsigned int bits = 0;
	if (!FFmpeg_Convert_Audio_Frame_To_PCM16(frame, converted, rate, channels, bits)) {
		return;
	}

	if (m_Rate == 0) {
		m_Rate = rate;
	}
	if (m_Channels == 0) {
		m_Channels = channels;
	}
	m_Bits = bits;
	m_PendingData.insert(m_PendingData.end(), converted.begin(), converted.end());
}

void FFMpegAudioStreamClass::Compact_Pending()
{
	if (m_PendingOffset == 0) {
		return;
	}

	if (m_PendingOffset >= m_PendingData.size()) {
		m_PendingData.clear();
		m_PendingOffset = 0;
	} else if (m_PendingOffset > 65536) {
		m_PendingData.erase(m_PendingData.begin(), m_PendingData.begin() + m_PendingOffset);
		m_PendingOffset = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////
//
//	FFMpegBufferClass
//
FFMpegBufferClass::FFMpegBufferClass(void)
	: m_Filename(nullptr),
	  m_Duration(0),
	  m_Rate(0),
	  m_Bits(PCM_BITS),
	  m_Channels(0),
	  m_IsStreaming(false)
{
}

FFMpegBufferClass::~FFMpegBufferClass(void)
{
	SAFE_FREE(m_Filename);
}

void FFMpegBufferClass::Set_Filename(const char *name)
{
	SAFE_FREE(m_Filename);
	if (name != nullptr) {
		m_Filename = ::strdup(name);
	}
}

bool FFMpegBufferClass::Load_From_File(const char *filename, bool streaming)
{
	WWPROFILE("FFMpegBufferClass::Load_From_File");

	if (filename == nullptr || filename[0] == '\0') {
		return false;
	}

	WWDEBUG_SAY(("Loading sound file %s.\r\n", filename));

	Set_Filename(filename);
	m_Buffer.clear();
	m_IsStreaming = streaming;
	m_Duration = 0;
	m_Rate = 0;
	m_Bits = PCM_BITS;
	m_Channels = 0;

	if (streaming) {
		FFMpegAudioStreamClass stream;
		if (!stream.Open(filename)) {
			return false;
		}
		m_Duration = stream.Get_Duration();
		m_Rate = stream.Get_Rate();
		m_Bits = stream.Get_Bits();
		m_Channels = stream.Get_Channels();
		return m_Rate != 0 && m_Channels != 0;
	}

	FFMpegAudioStreamClass stream;
	if (!stream.Open(filename)) {
		return false;
	}

	m_Duration = stream.Get_Duration();
	m_Rate = stream.Get_Rate();
	m_Bits = stream.Get_Bits();
	m_Channels = stream.Get_Channels();

	std::vector<unsigned char> chunk(32768);
	for (;;) {
		const unsigned int read = stream.Read(chunk.data(), static_cast<unsigned int>(chunk.size()));
		if (read == 0) {
			break;
		}
		m_Buffer.insert(m_Buffer.end(), chunk.begin(), chunk.begin() + read);
	}

	if (m_Buffer.empty()) {
		return false;
	}

	if (m_Duration == 0) {
		m_Duration = Calculate_Duration_MS(Get_Raw_Length(), m_Rate, m_Channels, m_Bits);
	}

	return true;
}
