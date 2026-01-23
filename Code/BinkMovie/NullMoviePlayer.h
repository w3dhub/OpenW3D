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

class FontCharsClass;

class NullMovieClass : public MovieClass
{
	public:
		NullMovieClass(const char* /* filename */, const char* /* subtitlename */, FontCharsClass* /* font */) {}
		~NullMovieClass() override {}

		void Update() override {}
		void Render() override {}
		bool Is_Complete() override { return true; }
};
