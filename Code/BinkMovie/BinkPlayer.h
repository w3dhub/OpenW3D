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
#pragma once

#include "MoviePlayer.h"
#include "rect.h"
#include "render2dsentence.h"
#include "wwstring.h"
#include <bink.h>
#include <vector>

class FontCharsClass;
class SubTitleManagerClass;

class BINKMovieClass : public MovieClass
{
	private:
		StringClass Filename;
		HBINK Bink;
		bool FrameChanged;
		unsigned TextureCount;
		unsigned long TicksPerFrame;

		struct TextureInfoStruct {
			TextureClass* Texture;
			int TextureWidth;
			int TextureHeight;
			int TextureLocX;
			int TextureLocY;
			RectClass UV;
			RectClass Rect;
		};

		TextureInfoStruct* TextureInfos;
		unsigned char* TempBuffer;
		Render2DClass Renderer;
		SubTitleManagerClass* SubTitleManager;
		std::vector<char> FileBuffer;

	public:
		BINKMovieClass(const char* filename,const char* subtitlename,FontCharsClass* font);
		~BINKMovieClass() override;

		void Update() override;
		void Render() override;
		bool Is_Complete() override;
};
