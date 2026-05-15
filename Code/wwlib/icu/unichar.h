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

	if (size_t(length) > len) {
		return size_t(-1);
	}

	return size_t(length) + 1;
}

inline size_t u_wstomb(char* dst, const unichar_t* src, size_t len)
{
	int32_t length;
	UErrorCode error = U_ZERO_ERROR;
	u_strToUTF8(dst, int32_t(len), &length, src, -1, &error);

	if (size_t(length) > len) {
		return size_t(-1);
	}

	return size_t(length) + 1;
}

inline int u_vsnprintf_n(char *buffer, const size_t buffer_count, const char *format, va_list args)
{
	// No char buffer that vsprintf is used for is larger than this in game.
	unichar_t fmt_buffer[8192];
	
	// Maybe it truncated, maybe it didn't. If it did, it would have in the orignal code too or worse overflowed.
	u_vsnprintf(fmt_buffer, sizeof(fmt_buffer) / sizeof(*fmt_buffer), format, args);
	return int(u_wstomb(buffer, fmt_buffer, buffer_count));
}

#endif // UNICHAR_H
