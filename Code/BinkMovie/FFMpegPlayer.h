/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 OpenW3D.
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
#pragma once

#include "MoviePlayer.h"
#include "rect.h"
#include "render2dsentence.h"
#include "wwstring.h"
#include <memory>
#include <vector>

class FontCharsClass;
class SubTitleManagerClass;
class FFmpegFile;
struct AVFrame;
struct SwsContext;

class FFMpegMovieClass : public MovieClass
{
private:
	unsigned TextureCount;
	unsigned TicksPerFrame;
	std::unique_ptr<FFmpegFile> Bink;
	AVFrame *CurrentFrame;
	SwsContext *ScalingContext;
	uint64_t StartTime;
	bool GotFrame;
	bool FrameChanged;

	struct TextureInfoStruct {
		TextureClass* Texture;
		int TextureWidth;
		int TextureHeight;
		int TextureLocX;
		int TextureLocY;
		RectClass UV;
		RectClass Rect;
	};

	std::vector<TextureInfoStruct> TextureInfos;
	Render2DClass Renderer;
	std::unique_ptr<SubTitleManagerClass> SubTitleManager;
	
	void On_Frame(AVFrame *frame, int stream_idx, int stream_type);

	static void On_Frame(AVFrame *frame, int stream_idx, int stream_type, void *user_data)
	{
		static_cast<FFMpegMovieClass*>(user_data)->On_Frame(frame, stream_idx, stream_type);
	}

public:
	FFMpegMovieClass(const char* filename, const char* subtitlename, FontCharsClass* font);
	~FFMpegMovieClass() override;

	void Update() override;
	void Render() override;
	bool Is_Complete() override;
};
