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

#include "binkmovie.h"
#include "MoviePlayer.h"

#if defined W3D_HAS_FFMPEG
#include "FFMpegPlayer.h"
#endif

#if defined W3D_HAS_BINK
#include "BinkPlayer.h"
#endif

static MovieClass* CurrentMovie;

void BINKMovie::Play(const char* filename,const char* subtitlename, FontCharsClass* font)
{
	if (CurrentMovie) {
		delete CurrentMovie;
		CurrentMovie = nullptr;
	}
#if defined W3D_HAS_FFMPEG
	CurrentMovie = new FFMpegMovieClass(filename,subtitlename,font);
#elif defined W3D_HAS_BINK
	CurrentMovie = new BINKMovieClass(filename,subtitlename,font);
#else
	CurrentMovie = new MovieClass(filename,subtitlename,font);
#endif
}

void BINKMovie::Stop()
{
	if (CurrentMovie) {
		delete CurrentMovie;
		CurrentMovie = nullptr;
	}
}

void BINKMovie::Update()
{
	if (CurrentMovie) {
		CurrentMovie->Update();
	}
}

void BINKMovie::Render()
{
	if (CurrentMovie) {
		CurrentMovie->Render();
	}
}

void BINKMovie::Init()
{
#ifdef W3D_HAS_BINK
	BinkSoundUseDirectSound(0);
#endif
}

void BINKMovie::Shutdown()
{
	Stop();
}

bool BINKMovie::Is_Complete()
{
	if (CurrentMovie) {
		return CurrentMovie->Is_Complete();
	}

	return true;
}
