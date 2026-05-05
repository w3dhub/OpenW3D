/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 TheSuperHackers
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
//////// FFmpegFile.cpp ///////////////////////////
// Stephan Vedder, April 2025
/////////////////////////////////////////////////

#include "FFmpegFile.h"
#include "ffactory.h"
#include "wwdebug.h"
#include "wwfile.h"

#include <algorithm>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

bool FFmpeg_Convert_Audio_Frame_To_PCM16(
	const AVFrame *frame,
	std::vector<uint8_t> &pcm,
	unsigned int &rate,
	unsigned int &channels,
	unsigned int &bits)
{
	pcm.clear();

	if (frame == nullptr || frame->nb_samples <= 0 || frame->sample_rate <= 0) {
		return false;
	}

	AVChannelLayout input_layout = {};
	if (!(frame->ch_layout.nb_channels > 0 && av_channel_layout_copy(&input_layout, &frame->ch_layout) == 0)) {
		av_channel_layout_default(&input_layout, 1);
	}

	const int input_channels = std::max(input_layout.nb_channels, 1);
	const int output_channels = input_channels == 1 ? 1 : 2;

	AVChannelLayout output_layout = {};
	av_channel_layout_default(&output_layout, output_channels);

	SwrContext *swr = nullptr;
	int result = swr_alloc_set_opts2(
		&swr,
		&output_layout,
		AV_SAMPLE_FMT_S16,
		frame->sample_rate,
		&input_layout,
		static_cast<AVSampleFormat>(frame->format),
		frame->sample_rate,
		0,
		nullptr);

	if (result < 0 || swr == nullptr) {
		av_channel_layout_uninit(&input_layout);
		av_channel_layout_uninit(&output_layout);
		return false;
	}

	result = swr_init(swr);
	if (result < 0) {
		swr_free(&swr);
		av_channel_layout_uninit(&input_layout);
		av_channel_layout_uninit(&output_layout);
		return false;
	}

	const int output_samples = static_cast<int>(
		av_rescale_rnd(
			swr_get_delay(swr, frame->sample_rate) + frame->nb_samples,
			frame->sample_rate,
			frame->sample_rate,
			AV_ROUND_UP));

	uint8_t *output_data = nullptr;
	int output_line_size = 0;
	result = av_samples_alloc(
		&output_data,
		&output_line_size,
		output_channels,
		output_samples,
		AV_SAMPLE_FMT_S16,
		0);

	if (result < 0 || output_data == nullptr) {
		swr_free(&swr);
		av_channel_layout_uninit(&input_layout);
		av_channel_layout_uninit(&output_layout);
		return false;
	}

	const uint8_t **input_data = const_cast<const uint8_t **>(frame->extended_data);
	const int converted_samples = swr_convert(swr, &output_data, output_samples, input_data, frame->nb_samples);
	if (converted_samples > 0) {
		const int output_size = av_samples_get_buffer_size(
			nullptr,
			output_channels,
			converted_samples,
			AV_SAMPLE_FMT_S16,
			1);
		if (output_size > 0) {
			pcm.assign(output_data, output_data + output_size);
		}
	}

	av_freep(&output_data);
	swr_free(&swr);
	av_channel_layout_uninit(&input_layout);
	av_channel_layout_uninit(&output_layout);

	if (converted_samples <= 0 || pcm.empty()) {
		return false;
	}

	rate = static_cast<unsigned int>(frame->sample_rate);
	channels = static_cast<unsigned int>(output_channels);
	bits = 16;
	return true;
}

FFmpegFile::FFmpegFile() {}

FFmpegFile::FFmpegFile(const char *file, FileFactoryClass *fact)
{
	Open(file, fact);
}

FFmpegFile::~FFmpegFile()
{
	Close();
}

bool FFmpegFile::Open(const char *file, FileFactoryClass *fact)
{
	WWASSERT_PRINT(file != nullptr, ("null file pointer"));
	Close();
#ifdef WWDEBUG
	av_log_set_level(AV_LOG_INFO);
#endif

// This is required for FFmpeg older than 4.0 -> deprecated afterwards though
#if LIBAVFORMAT_VERSION_MAJOR < 58
	av_register_all();
#endif
	Factory = fact == nullptr ? _TheFileFactory : fact;
	if (Factory == nullptr) {
		return false;
	}

	File = Factory->Get_File(file);
	if (File == nullptr || File->Open(FileClass::READ) == 0) {
		Close();
		return false;
	}

	// FFmpeg setup
	FmtCtx = avformat_alloc_context();
	if (!FmtCtx) {
		WWDEBUG_SAY(("Failed to alloc AVFormatContext\n"));
		Close();
		return false;
	}

	constexpr size_t avio_ctx_buffer_size = 0x10000;
	uint8_t *buffer = static_cast<uint8_t *>(av_malloc(avio_ctx_buffer_size));
	if (buffer == nullptr) {
		WWDEBUG_SAY(("Failed to alloc AVIOContextBuffer\n"));
		Close();
		return false;
	}

	AvioCtx = avio_alloc_context(buffer, avio_ctx_buffer_size, 0, File, &Read_Packet, nullptr, &Seek_Packet);
	if (AvioCtx == nullptr) {
		WWDEBUG_SAY(("Failed to alloc AVIOContext\n"));
		av_freep(&buffer);
		Close();
		return false;
	}

	FmtCtx->pb = AvioCtx;
	FmtCtx->flags |= AVFMT_FLAG_CUSTOM_IO;
	AvioCtx->seekable = AVIO_SEEKABLE_NORMAL;

	int result = avformat_open_input(&FmtCtx, nullptr, nullptr, nullptr);
	if (result < 0) {
		char error_buffer[1024];
		av_strerror(result, error_buffer, sizeof(error_buffer));
		WWDEBUG_SAY(("Failed 'avformat_open_input': %s\n", error_buffer));
		Close();
		return false;
	}

	result = avformat_find_stream_info(FmtCtx, nullptr);
	if (result < 0) {
		char error_buffer[1024];
		av_strerror(result, error_buffer, sizeof(error_buffer));
		WWDEBUG_SAY(("Failed 'avformat_find_stream_info': %s\n", error_buffer));
		Close();
		return false;
	}

	Streams.resize(FmtCtx->nb_streams);
	for (unsigned int stream_idx = 0; stream_idx < FmtCtx->nb_streams; stream_idx++) {
		AVStream *av_stream = FmtCtx->streams[stream_idx];
		const AVCodec *input_codec = avcodec_find_decoder(av_stream->codecpar->codec_id);
		if (input_codec == nullptr) {
			WWDEBUG_SAY(("Codec not supported: '%s'\n", avcodec_get_name(av_stream->codecpar->codec_id)));
			Close();
			return false;
		}

		AVCodecContext *codec_ctx = avcodec_alloc_context3(input_codec);
		if (codec_ctx == nullptr) {
			WWDEBUG_SAY(("Could not allocate codec context\n"));
			Close();
			return false;
		}

		result = avcodec_parameters_to_context(codec_ctx, av_stream->codecpar);
		if (result < 0) {
			char error_buffer[1024];
			av_strerror(result, error_buffer, sizeof(error_buffer));
			WWDEBUG_SAY(("Failed 'avcodec_parameters_to_context': %s\n", error_buffer));
			Close();
			return false;
		}

		result = avcodec_open2(codec_ctx, input_codec, nullptr);
		if (result < 0) {
			char error_buffer[1024];
			av_strerror(result, error_buffer, sizeof(error_buffer));
			WWDEBUG_SAY(("Failed 'avcodec_open2': %s\n", error_buffer));
			Close();
			return false;
		}

		FFmpegStream &output_stream = Streams[stream_idx];
		output_stream.codec_ctx = codec_ctx;
		output_stream.codec = input_codec;
		output_stream.stream_type = input_codec->type;
		output_stream.stream_idx = stream_idx;
		output_stream.frame = av_frame_alloc();
	}

	Packet = av_packet_alloc();
	Flushed = false;

	return true;
}

/**
 * Read an FFmpeg packet from file
 */
int FFmpegFile::Read_Packet(void *opaque, uint8_t *buf, int buf_size)
{
	FileClass *file = static_cast<FileClass *>(opaque);
	const int read = file->Read(buf, buf_size);

	// Streaming protocol requires us to return real errors - when we read less equal 0 we're at EOF
	if (read <= 0) {
		return AVERROR_EOF;
	}

	return read;
}

/**
 * Seek an FFmpeg packet from file
 */
int64_t FFmpegFile::Seek_Packet(void* opaque, int64_t offset, int whence)
{
	FileClass* file = static_cast<FileClass*>(opaque);
	if (whence & AVSEEK_FORCE) {
		whence &= ~AVSEEK_FORCE;
	}

	if (whence == AVSEEK_SIZE) {
		return file->Size();
	}

	const int seek = file->Seek(int(offset), whence);

	return seek;
}

/**
 * close all the open FFmpeg handles for an open file.
 */
void FFmpegFile::Close()
{
	if (FmtCtx != nullptr) {
		avformat_close_input(&FmtCtx);
	}

	for (auto &stream : Streams) {
		if (stream.codec_ctx != nullptr) {
			avcodec_free_context(&stream.codec_ctx);
			av_frame_free(&stream.frame);
		}
	}
	Streams.clear();

	if (AvioCtx != nullptr) {
		av_freep(&AvioCtx->buffer);
		avio_context_free(&AvioCtx);
		AvioCtx = nullptr;
	}

	if (Packet != nullptr) {
		av_packet_free(&Packet);
		Packet = nullptr;
	}

	if (File != nullptr) {
		Factory->Return_File(File);
		File = nullptr;
	}

	if (Factory != nullptr) {
		Factory = nullptr;
	}

	FrameCallback = nullptr;
	UserData = nullptr;
	Flushed = false;
}

bool FFmpegFile::Decode_Packet()
{
	WWASSERT_PRINT(FmtCtx != nullptr, ("null format context"));
	WWASSERT_PRINT(Packet != nullptr, ("null packet pointer"));

	if (Flushed) {
		return false;
	}

	int result = av_read_frame(FmtCtx, Packet);
	if (result == AVERROR_EOF) {
		return Flush_Decoders();
	}
	if (result < 0) {
		char error_buffer[1024];
		av_strerror(result, error_buffer, sizeof(error_buffer));
		WWDEBUG_SAY(("Failed 'av_read_frame': %s", error_buffer));
		return false;
	}

	const int stream_idx = Packet->stream_index;
	WWASSERT_PRINT(Streams.size() > unsigned(stream_idx), ("stream index out of bounds"));
	if (stream_idx < 0 || Streams.size() <= unsigned(stream_idx)) {
		av_packet_unref(Packet);
		return true;
	}

	auto &stream = Streams[stream_idx];
	AVCodecContext *codec_ctx = stream.codec_ctx;
	result = avcodec_send_packet(codec_ctx, Packet);
	av_packet_unref(Packet);

	// Check if we need more data
	if (result == AVERROR(EAGAIN)) {
		return true;
	}

	// Handle any other errors
	if (result < 0) {
		char error_buffer[1024];
		av_strerror(result, error_buffer, sizeof(error_buffer));
		WWDEBUG_SAY(("Failed 'avcodec_send_packet': %s\n", error_buffer));
		return false;
	}

	// Get all frames in this packet
	while (result >= 0) {
		result = avcodec_receive_frame(codec_ctx, stream.frame);

		// Check if we need more data
		if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
			return true;
		}

		// Handle any other errors
		if (result < 0) {
			char error_buffer[1024];
			av_strerror(result, error_buffer, sizeof(error_buffer));
			WWDEBUG_SAY(("Failed 'avcodec_receive_frame': %s\n", error_buffer));
			return false;
		}

		if (FrameCallback != nullptr) {
			FrameCallback(stream.frame, stream_idx, stream.stream_type, UserData);
		}
	}

	return true;
}

bool FFmpegFile::Flush_Decoders()
{
	bool emitted_frame = false;

	for (auto &stream : Streams) {
		if (stream.codec_ctx == nullptr) {
			continue;
		}

		int result = avcodec_send_packet(stream.codec_ctx, nullptr);
		if (result < 0 && result != AVERROR_EOF) {
			continue;
		}

		for (;;) {
			result = avcodec_receive_frame(stream.codec_ctx, stream.frame);
			if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
				break;
			}

			if (result < 0) {
				char error_buffer[1024];
				av_strerror(result, error_buffer, sizeof(error_buffer));
				WWDEBUG_SAY(("Failed 'avcodec_receive_frame' while flushing: %s", error_buffer));
				Flushed = true;
				return emitted_frame;
			}

			if (FrameCallback != nullptr) {
				FrameCallback(stream.frame, stream.stream_idx, stream.stream_type, UserData);
			}
			emitted_frame = true;
		}
	}

	Flushed = true;
	return emitted_frame;
}

void FFmpegFile::Seek_Frame(int frame_idx)
{
	// Note: not tested, since not used ingame
	for (const auto &stream : Streams) {
		int64_t timestamp = av_q2d(FmtCtx->streams[stream.stream_idx]->time_base) * frame_idx
			* av_q2d(FmtCtx->streams[stream.stream_idx]->avg_frame_rate);
		int result = av_seek_frame(FmtCtx, stream.stream_idx, timestamp, AVSEEK_FLAG_ANY);
		if (result < 0) {
			char error_buffer[1024];
			av_strerror(result, error_buffer, sizeof(error_buffer));
			WWDEBUG_SAY(("Failed 'av_seek_frame': %s\n", error_buffer));
		}
	}
}

void FFmpegFile::Rewind()
{
	if (Packet != nullptr) {
		av_packet_unref(Packet);
	}
	for (const auto& stream : Streams) {
		int result = avformat_seek_file(FmtCtx, stream.stream_idx, 0, 0, 0, AVSEEK_FLAG_ANY);
		if (result < 0) {
			char error_buffer[1024];
			av_strerror(result, error_buffer, sizeof(error_buffer));
			WWDEBUG_SAY(("Failed 'avformat_seek_file': %s\n", error_buffer));
		}
		if (stream.codec_ctx != nullptr) {
			avcodec_flush_buffers(stream.codec_ctx);
		}
	}
	Flushed = false;
}

bool FFmpegFile::Has_Audio() const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_AUDIO);
	return stream != nullptr;
}

bool FFmpegFile::Has_Video() const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
	return stream != nullptr;
}

int FFmpegFile::Get_Duration() const
{
	// Convert duration to ms.
	return int((float(FmtCtx->duration) / float(AV_TIME_BASE)) * 1000.0F);
}

const FFmpegFile::FFmpegStream *FFmpegFile::Find_Match(int type) const
{
	for (auto &stream : Streams) {
		if (stream.stream_type == type) {
			return &stream;
		}
	}

	return nullptr;
}

int FFmpegFile::Get_Num_Channels() const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_AUDIO);
	if (stream == nullptr) {
		return 0;
	}

	return stream->codec_ctx->ch_layout.nb_channels;
}

int FFmpegFile::Get_Sample_Rate() const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_AUDIO);
	if (stream == nullptr) {
		return 0;
	}

	return stream->codec_ctx->sample_rate;
}

int FFmpegFile::Get_Bytes_Per_Sample() const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_AUDIO);
	if (stream == nullptr) {
		return 0;
	}

	return av_get_bytes_per_sample(stream->codec_ctx->sample_fmt);
}

int FFmpegFile::Get_Size_For_Samples(int numSamples) const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_AUDIO);
	if (stream == nullptr) {
		return 0;
	}

	return av_samples_get_buffer_size(nullptr, stream->codec_ctx->ch_layout.nb_channels, numSamples, stream->codec_ctx->sample_fmt, 1);
}

int FFmpegFile::Get_Height() const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
	if (stream == nullptr) {
		return 0;
	}

	return stream->codec_ctx->height;
}

int FFmpegFile::Get_Width() const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
	if (stream == nullptr) {
		return 0;
	}

	return stream->codec_ctx->width;
}

int FFmpegFile::Get_Num_Frames() const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
	if (FmtCtx == nullptr || stream == nullptr || FmtCtx->streams[stream->stream_idx] == nullptr) {
		return 0;
	}

	return (FmtCtx->duration / (double)AV_TIME_BASE) * av_q2d(FmtCtx->streams[stream->stream_idx]->avg_frame_rate);
}

int FFmpegFile::Get_Current_Frame() const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
	if (stream == nullptr) {
		return 0;
	}

	return stream->codec_ctx->frame_num;
}

int FFmpegFile::Get_Pixel_Format() const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
	if (stream == nullptr) {
		return AV_PIX_FMT_NONE;
	}

	return stream->codec_ctx->pix_fmt;
}

unsigned FFmpegFile::Get_Frame_Time() const
{
	const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
	if (stream == nullptr) {
		return 0u;
	}

	return 1000u / av_q2d(FmtCtx->streams[stream->stream_idx]->avg_frame_rate);
}
