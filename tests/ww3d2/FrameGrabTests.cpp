/*
**	Command & Conquer Renegade(tm)
**	Copyright 2026 OpenW3D Contributors.
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

#include "framgrab.h"
#include "ww3d.h"

#include <algorithm>
#include <array>
#include <climits>
#include <iostream>
#include <type_traits>

namespace
{
using LegacyGrabSignature = void (FrameGrabClass::*)(void *);
using TryGrabSignature = bool (FrameGrabClass::*)(void *);
using ConverterSignature = bool (*)(const void *, int, void *, unsigned int, int, int);
using TryStartBackBufferSignature = bool (*)(const char *, float);
using TryUpdateBackBufferSignature = bool (*)();

static_assert(std::is_same_v<decltype(&FrameGrabClass::Grab), LegacyGrabSignature>);
static_assert(std::is_same_v<decltype(&FrameGrabClass::ConvertGrab), LegacyGrabSignature>);
static_assert(std::is_same_v<decltype(&FrameGrabClass::TryGrab), TryGrabSignature>);
static_assert(std::is_same_v<decltype(&FrameGrabClass::ConvertBGRA32ToBGR24), ConverterSignature>);
static_assert(std::is_same_v<decltype(&WW3D::Try_Start_Movie_Capture_From_Back_Buffer),
	TryStartBackBufferSignature>);
static_assert(std::is_same_v<decltype(&WW3D::Try_Update_Movie_Capture_From_Back_Buffer),
	TryUpdateBackBufferSignature>);

bool Expect(bool condition, const char *message)
{
	if (!condition) {
		std::cerr << message << '\n';
	}
	return condition;
}

bool TestRowStride()
{
	bool ok = true;
	ok &= Expect(FrameGrabClass::CalculateRowStride(3, 24) == 12, "3x24 stride must include padding");
	ok &= Expect(FrameGrabClass::CalculateRowStride(4, 24) == 12, "4x24 stride must be packed");
	ok &= Expect(FrameGrabClass::CalculateRowStride(0, 24) == 0, "zero width must be rejected");
	ok &= Expect(FrameGrabClass::CalculateRowStride(3, 0) == 0, "zero bit depth must be rejected");
	ok &= Expect(FrameGrabClass::CalculateRowStride(INT_MAX, 24) == 0, "overflowing stride must be rejected");
	return ok;
}

bool TestConversion()
{
	constexpr int width = 3;
	constexpr int height = 2;
	constexpr int source_stride = 16;
	constexpr unsigned int destination_stride = 12;
	constexpr unsigned char canary = 0xcd;

	std::array<unsigned char, 4 + source_stride * height + 4> source{};
	source.fill(canary);
	unsigned char *source_pixels = source.data() + 4;
	const std::array<unsigned char, source_stride * height> source_image = {
		0x01, 0x02, 0x03, 0xa0,  0x11, 0x12, 0x13, 0xa1,  0x21, 0x22, 0x23, 0xa2,
		0xf1, 0xf2, 0xf3, 0xf4,
		0x31, 0x32, 0x33, 0xb0,  0x41, 0x42, 0x43, 0xb1,  0x51, 0x52, 0x53, 0xb2,
		0xe1, 0xe2, 0xe3, 0xe4
	};
	std::copy(source_image.begin(), source_image.end(), source_pixels);
	const auto original_source = source;

	std::array<unsigned char, 4 + destination_stride * height + 4> destination{};
	destination.fill(canary);
	unsigned char *destination_pixels = destination.data() + 4;
	const bool converted = FrameGrabClass::ConvertBGRA32ToBGR24(source_pixels, source_stride,
		destination_pixels, destination_stride, width, height);

	const std::array<unsigned char, destination_stride * height> expected = {
		0x31, 0x32, 0x33,  0x41, 0x42, 0x43,  0x51, 0x52, 0x53,  0x00, 0x00, 0x00,
		0x01, 0x02, 0x03,  0x11, 0x12, 0x13,  0x21, 0x22, 0x23,  0x00, 0x00, 0x00
	};

	bool ok = true;
	ok &= Expect(converted, "valid conversion failed");
	ok &= Expect(std::equal(expected.begin(), expected.end(), destination_pixels),
		"converted pixels, vertical order, or padding differ");
	ok &= Expect(source == original_source, "conversion modified source bytes or source canaries");
	ok &= Expect(std::all_of(destination.begin(), destination.begin() + 4,
		[](unsigned char byte) { return byte == 0xcd; }), "leading destination canary changed");
	ok &= Expect(std::all_of(destination.end() - 4, destination.end(),
		[](unsigned char byte) { return byte == 0xcd; }), "trailing destination canary changed");
	return ok;
}

bool TestInvalidConversionInputs()
{
	std::array<unsigned char, 32> source{};
	std::array<unsigned char, 32> destination{};
	bool ok = true;
	ok &= Expect(!FrameGrabClass::ConvertBGRA32ToBGR24(nullptr, 16, destination.data(), 12, 3, 2),
		"null source must fail");
	ok &= Expect(!FrameGrabClass::ConvertBGRA32ToBGR24(source.data(), 16, nullptr, 12, 3, 2),
		"null destination must fail");
	ok &= Expect(!FrameGrabClass::ConvertBGRA32ToBGR24(source.data(), 0, destination.data(), 12, 3, 2),
		"zero source pitch must fail");
	ok &= Expect(!FrameGrabClass::ConvertBGRA32ToBGR24(source.data(), 11, destination.data(), 12, 3, 2),
		"undersized source pitch must fail");
	ok &= Expect(!FrameGrabClass::ConvertBGRA32ToBGR24(source.data(), 16, destination.data(), 11, 3, 2),
		"undersized destination stride must fail");
	ok &= Expect(!FrameGrabClass::ConvertBGRA32ToBGR24(source.data(), 16, destination.data(), 12, 0, 2),
		"zero width must fail");
	ok &= Expect(!FrameGrabClass::ConvertBGRA32ToBGR24(source.data(), 16, destination.data(), 12, 3, 0),
		"zero height must fail");
	ok &= Expect(!FrameGrabClass::ConvertBGRA32ToBGR24(source.data(), INT_MAX,
		destination.data(), UINT_MAX, INT_MAX, 2), "overflowing row sizes must fail");
	return ok;
}

bool TestInvalidConstruction()
{
	FrameGrabClass null_name(nullptr, FrameGrabClass::AVI, 3, 2, 24, 30.0f);
	FrameGrabClass empty_name("", FrameGrabClass::AVI, 3, 2, 24, 30.0f);
	FrameGrabClass bad_width("Movie", FrameGrabClass::AVI, 0, 2, 24, 30.0f);
	FrameGrabClass bad_depth("Movie", FrameGrabClass::AVI, 3, 2, 32, 30.0f);
	FrameGrabClass oversized("Movie", FrameGrabClass::AVI, 100000, 10000, 24, 30.0f);
	FrameGrabClass raw("Movie", FrameGrabClass::RAW, 3, 2, 24, 30.0f);
	bool ok = true;
	ok &= Expect(!null_name.IsReady() && null_name.GetBuffer() == nullptr,
		"null filename construction must remain inert");
	ok &= Expect(!empty_name.IsReady() && empty_name.GetBuffer() == nullptr,
		"empty filename construction must remain inert");
	ok &= Expect(!bad_width.IsReady() && bad_width.GetBuffer() == nullptr &&
		bad_width.GetWidth() == 0 && bad_width.GetBufferSize() == 0,
		"invalid width construction must remain inert");
	ok &= Expect(!bad_depth.IsReady() && bad_depth.GetBuffer() == nullptr,
		"non-BGR24 construction must remain inert");
	ok &= Expect(!oversized.IsReady() && oversized.GetBuffer() == nullptr,
		"AVIStreamWrite-sized image overflow must remain inert");
	ok &= Expect(!raw.IsReady() && !raw.TryGrab(nullptr), "RAW construction must not report readiness");
	return ok;
}
}

int main()
{
	bool passed = true;
	passed &= TestRowStride();
	passed &= TestConversion();
	passed &= TestInvalidConversionInputs();
	passed &= TestInvalidConstruction();
	return passed ? 0 : 1;
}
