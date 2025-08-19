/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 OpenW3D
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

//////// FFmpegFile.h ///////////////////////////
// Stephan Vedder, April 2025
/////////////////////////////////////////////////

#pragma once

#include <vector>

struct AVFormatContext;
struct AVIOContext;
struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
class FileClass;
class FileFactoryClass;

typedef void(*FFmpegFrameCallback)(AVFrame *, int, int, void *);

class FFmpegFile
{
public:
	FFmpegFile();
	// The constructur takes ownership of the file
	explicit FFmpegFile(const char *file);
	~FFmpegFile();

	bool Open(const char *file);
	void Close();
	void Set_Frame_Callback(FFmpegFrameCallback callback) { FrameCallback = callback; }
	void Set_User_Data(void *user_data) { UserData = user_data; }
	// Read & decode a packet from the container. Note that we could/should split this step
	bool Decode_Packet();
	void Seek_Frame(int frame_idx);
	bool Has_Audio() const;
	bool Has_Video() const;

	// Audio specific
	int Get_Size_For_Samples(int numSamples) const;
	int Get_Num_Channels() const;
	int Get_Sample_Rate() const;
	int Get_Bytes_Per_Sample() const;

	// Video specific
	int Get_Width() const;
	int Get_Height() const;
	int Get_Num_Frames() const;
	int Get_Current_Frame() const;
	int Get_Pixel_Format() const;
	unsigned Get_Frame_Time() const;

private:
	struct FFmpegStream
	{
		AVCodecContext *codec_ctx = nullptr;
		const AVCodec *codec = nullptr;
		int stream_idx = -1;
		int stream_type = -1;
		AVFrame *frame = nullptr;
	};

	static int Read_Packet(void *opaque, uint8_t *buf, int buf_size);
	const FFmpegStream *Find_Match(int type) const;

	FFmpegFrameCallback 		FrameCallback = nullptr; ///< Callback for frame processing
	AVFormatContext 			*FmtCtx = nullptr; ///< Format context for AVFormat
	AVIOContext 				*AvioCtx = nullptr; ///< IO context for AVFormat
	AVPacket 					*Packet = nullptr; ///< Current packet
	std::vector<FFmpegStream> 	Streams; ///< List of streams in the file
	FileClass 					*File = nullptr;	///< File handle for the file
	FileFactoryClass			*Factory = nullptr;
	void 						*UserData = nullptr; ///< User data for the callback
};
