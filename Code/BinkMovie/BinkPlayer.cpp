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

#include "BinkPlayer.h"
#include "dx8wrapper.h"
#include "formconv.h"
#include "subtitlemanager.h"
#include "ffactory.h"
#include "wwfile.h"

// This is missing from the stub SDK headers as its unused in original game code.
#ifndef BINKFROMMEMORY
#define BINKFROMMEMORY 0x04000000
#endif

BINKMovieClass::BINKMovieClass(const char* filename, const char* subtitlename, FontCharsClass* font)
	:
	MovieClass(filename, subtitlename, font),
	Filename(filename),
	Bink(0),
	FrameChanged(true),
	TicksPerFrame(0),
	SubTitleManager(NULL)
{
	// Load the whole file so we can use our own file IO to access it.
	file_auto_ptr file(_TheFileFactory, filename);
	FileBuffer.resize(file->Size());
	file->Read(&FileBuffer[0], file->Size());

	Bink = BinkOpen(&FileBuffer[0], BINKFROMMEMORY);

	if (Bink == NULL) {
		return;
	}

	TempBuffer = new unsigned char[Bink->Width * Bink->Height*2];

	const D3DCAPS9& dx8caps = DX8Wrapper::Get_Current_Caps()->Get_DX8_Caps();
	unsigned poweroftwowidth = 1;

	while (poweroftwowidth < Bink->Width) {
		poweroftwowidth <<= 1;
	}

	unsigned poweroftwoheight = 1;
	
	while (poweroftwoheight < Bink->Height) {
		poweroftwoheight <<= 1;
	}

	if (poweroftwowidth > dx8caps.MaxTextureWidth) {
		poweroftwowidth = dx8caps.MaxTextureWidth;
	}
	
	if (poweroftwoheight > dx8caps.MaxTextureHeight) {
		poweroftwoheight = dx8caps.MaxTextureHeight;
	}

	TextureCount = 0;
	unsigned max_width = poweroftwowidth;
	unsigned max_height = poweroftwoheight;
	unsigned x, y;

	for (y = 0; y < Bink->Height; y += max_height-2) {		// Two pixels are lost due to duplicated edges to prevent bilinear artifacts
		for (x = 0; x < Bink->Width; x += max_width-2) {
			++TextureCount;
		}
	}

	TextureInfos = new TextureInfoStruct[TextureCount];
	unsigned cnt = 0;
	
	for (y = 0; y < Bink->Height; y += max_height-1) {
		for (x = 0; x < Bink->Width; x += max_width-1) {
			TextureInfos[cnt].Texture = new TextureClass(
				max_width, max_height, D3DFormat_To_WW3DFormat(D3DFMT_R5G6B5),
				TextureClass::MIP_LEVELS_1, TextureClass::POOL_MANAGED, false);

			TextureInfos[cnt].TextureLocX = x;
			TextureInfos[cnt].TextureLocY = y;
			TextureInfos[cnt].TextureWidth = max_width;
			TextureInfos[cnt].UV.Right = float(max_width) / float(max_width);

			if ((TextureInfos[cnt].TextureWidth + x) > Bink->Width) {
				TextureInfos[cnt].TextureWidth = Bink->Width - x;
				TextureInfos[cnt].UV.Right = float(TextureInfos[cnt].TextureWidth - 1) / float(max_width);
			}

			TextureInfos[cnt].TextureHeight = max_height;
			TextureInfos[cnt].UV.Bottom = float(max_height) / float(max_height);

			if ((TextureInfos[cnt].TextureHeight + y) > Bink->Height) {
				TextureInfos[cnt].TextureHeight = Bink->Height - y;
				TextureInfos[cnt].UV.Bottom = float(TextureInfos[cnt].TextureHeight + 1) / float(max_height);
			}

			TextureInfos[cnt].UV.Left = 1.0f / float(max_width);
			TextureInfos[cnt].UV.Top = 1.0f / float(max_height);

			TextureInfos[cnt].Rect.Left = float(TextureInfos[cnt].TextureLocX) / float(Bink->Width);
			TextureInfos[cnt].Rect.Top = float(TextureInfos[cnt].TextureLocY) / float(Bink->Height);
			TextureInfos[cnt].Rect.Right = float(TextureInfos[cnt].TextureLocX + TextureInfos[cnt].TextureWidth) / float(Bink->Width);
			TextureInfos[cnt].Rect.Bottom = float(TextureInfos[cnt].TextureLocY + TextureInfos[cnt].TextureHeight) / float(Bink->Height);

			++cnt;
		}
	}

	Renderer.Reset();

	// Calculate the time per frame of video
	unsigned int rate = (Bink->FrameRate / Bink->FrameRateDiv);
	TicksPerFrame = (60 / rate);

	if (subtitlename && font) {
		SubTitleManager = SubTitleManagerClass::Create(filename, subtitlename, font);
	}
}


BINKMovieClass::~BINKMovieClass()
{
	if (Bink == NULL) {
		return;
	}

	if (Bink) {
		BinkClose(Bink);
	}

	delete[] TempBuffer;

	if (TextureInfos) {
		for (unsigned t = 0; t < TextureCount; ++t) {
			REF_PTR_RELEASE(TextureInfos[t].Texture);
		}

		delete[] TextureInfos;
	}

	if (SubTitleManager) {
		delete SubTitleManager;
	}
}


void BINKMovieClass::Update()
{
	if (!Bink) {
		return;
	}

	FrameChanged |= !BinkWait(Bink);
}


static unsigned char* Get_Tex_Address(unsigned char* buffer, int x, int y, int w, int h)
{
	if (x < 0) {
		x = 0;
	} else if (x >= w) {
		x = w - 1;
	}

	if (y < 0) {
		y = 0;
	} else if (y >= h) {
		y = h - 1;
	}

	return buffer + x * 2 + y * 2 * w;
}


void BINKMovieClass::Render()
{
	if (!Bink) {
		return;
	}

	// decompress a frame
	if (FrameChanged) {
		BinkDoFrame(Bink);
		FrameChanged = false;

		BinkCopyToBuffer(Bink, TempBuffer, Bink->Width * 2, Bink->Height, 0, 0, BINKSURFACE565|BINKCOPYNOSCALING);

		for (unsigned t = 0; t < TextureCount; ++t) {
			IDirect3DTexture9* d3d_texture = TextureInfos[t].Texture->Peek_DX8_Texture();

			if (d3d_texture) {
				unsigned char* cur_tex_ptr = Get_Tex_Address(TempBuffer, TextureInfos[t].TextureLocX,
					TextureInfos[t].TextureLocY, Bink->Width, Bink->Height);

				unsigned w = TextureInfos[t].TextureWidth;
				unsigned h = TextureInfos[t].TextureHeight;

				if (w > Bink->Width-TextureInfos[t].TextureLocX) {
					w = Bink->Width-TextureInfos[t].TextureLocX;
				}

				if (h > Bink->Height-TextureInfos[t].TextureLocY) {
					h = Bink->Height-TextureInfos[t].TextureLocY;
				}

				D3DSURFACE_DESC d3d_surf_desc;
				D3DLOCKED_RECT locked_rect;

				DX8_ErrorCode(d3d_texture->GetLevelDesc(0, &d3d_surf_desc));

				RECT rect;
				rect.left = 0;
				rect.top = 0;
				rect.right = w;
				rect.bottom = h;
				DX8_ErrorCode(d3d_texture->LockRect(0,&locked_rect,&rect,0));

				for (unsigned y = 0; y < h; ++y) {
					unsigned char* dest = (unsigned char*)locked_rect.pBits + y * locked_rect.Pitch;
					memcpy(dest, cur_tex_ptr, w * 2);
					cur_tex_ptr += Bink->Width * 2;
				}

				DX8_ErrorCode(d3d_texture->UnlockRect(0));
			}
		}

		if (Bink->FrameNum < Bink->Frames) // goto the next if not on the last
			BinkNextFrame(Bink);
	}

	for (unsigned t = 0; t < TextureCount; ++t) {
		Renderer.Reset();
		Renderer.Set_Texture(TextureInfos[t].Texture);
		Renderer.Set_Coordinate_Range(RectClass(0.0f, 0.0f, 1.0f, 1.0f));//Bink->Width,Bink->Height));

		RectClass rect(TextureInfos[t].TextureLocX, TextureInfos[t].TextureLocY, TextureInfos[t].TextureWidth, TextureInfos[t].TextureHeight);
		Renderer.Add_Quad(TextureInfos[t].Rect, TextureInfos[t].UV, 0xffffffff);
		Renderer.Render();
	}

	if (SubTitleManager) {
		unsigned long movieTime = (Bink->FrameNum * TicksPerFrame);
		SubTitleManager->Process(movieTime);
		SubTitleManager->Render();
	}
}


bool BINKMovieClass::Is_Complete()
{
	if (!Bink) return true;
	return (Bink->FrameNum>=Bink->Frames);
}
