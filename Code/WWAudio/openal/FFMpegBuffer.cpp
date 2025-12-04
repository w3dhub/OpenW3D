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

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/avutil.h>
}

/////////////////////////////////////////////////////////////////////////////////
//
//	FFMpegBufferClass
//
FFMpegBufferClass::FFMpegBufferClass (void)
	: m_Buffer (NULL),
	  m_Filename (NULL),
	  m_Duration (0),
	  m_Rate (0),
	  m_Bits (0),
	  m_Channels (0),
		m_MaxBuffer (0)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	~FFMpegBufferClass
//
FFMpegBufferClass::~FFMpegBufferClass (void)
{
	SAFE_FREE (m_Filename);
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Set_Filename
//
void
FFMpegBufferClass::Set_Filename (const char *name)
{
	SAFE_FREE (m_Filename);
	if (name != NULL) {
		m_Filename = ::strdup (name);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Load_From_File
//
bool
FFMpegBufferClass::Load_From_File (const char *filename)
{
	WWPROFILE ("FFMpegBufferClass::Load_From_File");
	WWDEBUG_SAY(( "Loading sound file %s.\r\n", filename));

	m_Buffer.clear();
	Set_Filename (filename);
	m_FileHandle.Open(filename);

	if (!m_FileHandle.Has_Audio()) {
		return false;
	}

	m_Duration = m_FileHandle.Get_Duration();
	m_Rate = m_FileHandle.Get_Sample_Rate();
	m_Channels = m_FileHandle.Get_Num_Channels();
	m_Bits = m_FileHandle.Get_Bytes_Per_Sample() * 8;
	m_MaxBuffer = m_Rate * m_FileHandle.Get_Bytes_Per_Sample(); // Buffer up to 1 sec at a time.
	// Return the true/false result code
	return true;
}

bool FFMpegBufferClass::Refresh_Buffer()
{
	if (!m_FileHandle.Has_Audio()) {
		m_FileHandle.Open(Get_Filename());

		if (!m_FileHandle.Has_Audio()) {
			WWDEBUG_SAY(("No audio detected in %s\n", Get_Filename()));
			return false;
		}
	}
	
	m_Buffer.clear();
	
	FFmpegFrameCallback on_frame = [](AVFrame* frame, int /* stream_idx */, int stream_type, void* user_data) {
		FFMpegBufferClass* sbc = static_cast<FFMpegBufferClass*>(user_data);
		if (stream_type != AVMEDIA_TYPE_AUDIO) {
			return;
		}

		const int frame_data_size = av_samples_get_buffer_size(nullptr,  sbc->m_Channels, frame->nb_samples, static_cast<AVSampleFormat>(frame->format), 1);
		sbc->m_Buffer.reserve(sbc->m_Buffer.size() + frame_data_size);

		if (av_sample_fmt_is_planar(static_cast<AVSampleFormat>(frame->format))) {
			// Convert planar audio to interleaved
			int num_channels = sbc->m_Channels;
			int bytes_per_sample = sbc->m_Bits / 8;
			for (int sample = 0; sample < frame->nb_samples; ++sample) {
				for (int channel = 0; channel < num_channels; ++channel) {
					const uint8_t* src = frame->data[channel] + sample * bytes_per_sample;
					sbc->m_Buffer.insert(sbc->m_Buffer.end(), src, src + bytes_per_sample);
				}
			}
		} else {
			// Directly copy interleaved audio
			sbc->m_Buffer.insert(sbc->m_Buffer.end(), frame->data[0], frame->data[0] + frame_data_size);
		}
	};

	m_FileHandle.Set_Frame_Callback(on_frame);
	m_FileHandle.Set_User_Data(this);

	// Read packets inside the file
	while (m_Buffer.size() < m_MaxBuffer) {
		if (!m_FileHandle.Decode_Packet()) {
			return false;
		}
	}
	return true;
}

void FFMpegBufferClass::Reset_Buffer(void)
{
	m_FileHandle.Rewind();
}
