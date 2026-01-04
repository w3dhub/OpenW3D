/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 OpenW3D Contributors.
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

#ifndef UNICHAR_H
#define UNICHAR_H

#define W3D_USING_ICU 1

//#include <wchar.h>
#include <unicode/uchar.h>
#include <unicode/ustdio.h>
#include <unicode/ustring.h>
#include <unicode/unumberformatter.h>
#include <unicode/unumberoptions.h>
typedef UChar unichar_t;
#define U_CHAR(str) (u##str)

inline size_t u_mbtows(unichar_t* dst, const char* src, size_t len)
{
	int32_t length;
	UErrorCode error = U_ZERO_ERROR;
	u_strFromUTF8(dst, int32_t(len), &length, src, -1, &error);

	if (U_FAILURE(error) || size_t(length) > len) {
		return size_t(-1);
	}

	return size_t(length);
}

inline size_t u_wstomb(char* dst, const unichar_t* src, size_t len)
{
	int32_t length;
	UErrorCode error = U_ZERO_ERROR;
	u_strToUTF8(dst, int32_t(len), &length, src, -1, &error);

	if (U_FAILURE(error) || size_t(length) > len) {
		return size_t(-1);
	}

	return size_t(length);
}

#endif // UNICHAR_H
