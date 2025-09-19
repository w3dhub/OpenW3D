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
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BINKMOVIE_H
#define BINKMOVIE_H

class FontCharsClass;

// ----------------------------------------------------------------------------
//
// BINK movie player. You'll need to have binkw32.dll in the run directory!
//
// To start a movie call Play("movie.bik","subtitle_name");
// To end movie playing call Stop();
//
// In order to change the subtitle properties see SubTitleManagerClass.
//
// ----------------------------------------------------------------------------

class BINKMovie
{
public:
	static void Play(const char* filename,const char* subtitlename=nullptr, FontCharsClass* font = nullptr);
	static void Stop();
	static void Update();
	static void Render();
	static void Init();
	static void Shutdown();
	static bool	Is_Complete();
};

#endif