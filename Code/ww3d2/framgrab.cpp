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

// FramGrab.cpp: implementation of the FrameGrabClass class.
//
//////////////////////////////////////////////////////////////////////

#include "framgrab.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <io.h>
#include <limits>
#include <string>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FrameGrabClass::FrameGrabClass(const char *filename, MODE mode, int width, int height, int bitcount, float framerate) :
	Filename(filename),
	FrameRate(framerate),
	Mode(RAW),
	Counter(0),
	AVIFile(nullptr),
	Bitmap(nullptr),
	Stream(nullptr),
	AVIStreamInfo(),
	BitmapInfoHeader()
{
	if (mode != AVI) {
		return;
	}

	const unsigned int row_stride = CalculateRowStride(width, bitcount);
	const unsigned long long image_size =
		static_cast<unsigned long long>(row_stride) * static_cast<unsigned long long>(height);
	if (filename == nullptr || filename[0] == '\0' || width <= 0 || height <= 0 ||
		bitcount != 24 || !std::isfinite(framerate) || framerate <= 0.0f ||
		static_cast<double>(framerate) > std::numeric_limits<DWORD>::max() || row_stride == 0 ||
		image_size > static_cast<unsigned long long>(std::numeric_limits<LONG>::max())) {
		return;
	}

	// Find the first unused AVI filename with this prefix.
	unsigned int counter = 0;
	std::string file;
	do {
		file = std::string(filename) + std::to_string(counter++) + ".AVI";
	} while (_access(file.c_str(), 0) != -1 && counter != 0);
	if (counter == 0) {
		return;
	}

	// Do not initialize Video for Windows until all throwing filename work is
	// complete; once initialized, CleanupAVI owns the matching AVIFileExit.
	Mode = AVI;
	AVIFileInit();

	// Create new AVI file using AVIFileOpenA.
	HRESULT hr = AVIFileOpenA(&AVIFile, file.c_str(), OF_WRITE | OF_CREATE, nullptr);
	if (FAILED(hr)) {
		OutputDebugStringA("Unable to open AVI movie capture file.\n");
		CleanupAVI();
		return;
	}

	BitmapInfoHeader.biWidth = width;
	BitmapInfoHeader.biHeight = height;
	BitmapInfoHeader.biBitCount = static_cast<unsigned short>(bitcount);
	BitmapInfoHeader.biSizeImage = static_cast<DWORD>(image_size);
	BitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfoHeader.biPlanes = 1;
	BitmapInfoHeader.biCompression = BI_RGB;
	BitmapInfoHeader.biXPelsPerMeter = 1;
	BitmapInfoHeader.biYPelsPerMeter = 1;

	// Create an uncompressed, bottom-up BGR24 video stream.
	AVIStreamInfo.fccType = streamtypeVIDEO;
	AVIStreamInfo.fccHandler = mmioFOURCC('M','S','V','C');
	AVIStreamInfo.dwScale = 1;
	AVIStreamInfo.dwRate = static_cast<DWORD>(FrameRate);
	if (AVIStreamInfo.dwRate == 0) {
		AVIStreamInfo.dwRate = 1;
	}
	AVIStreamInfo.dwSuggestedBufferSize = BitmapInfoHeader.biSizeImage;
	SetRect(&AVIStreamInfo.rcFrame, 0, 0, width, height);
	AVIStreamInfo.szName[0] = 'G';

	hr = AVIFileCreateStreamA(AVIFile, &Stream, &AVIStreamInfo);
	if (FAILED(hr)) {
		CleanupAVI();
		return;
	}

	hr = AVIStreamSetFormat(Stream, 0, &BitmapInfoHeader, sizeof(BitmapInfoHeader));
	if (FAILED(hr)) {
		CleanupAVI();
		return;
	}

	Bitmap = static_cast<int *>(GlobalAllocPtr(
		GMEM_MOVEABLE | GMEM_ZEROINIT, BitmapInfoHeader.biSizeImage));
	if (Bitmap == nullptr) {
		CleanupAVI();
	}
}

FrameGrabClass::~FrameGrabClass()
{
	CleanupAVI();
}

void FrameGrabClass::CleanupAVI()
{
	const bool avi_library_initialized = (Mode == AVI);
	if (Bitmap != nullptr) {
		GlobalFreePtr(Bitmap);
		Bitmap = nullptr;
	}
	if (Stream != nullptr) {
		AVIStreamRelease(Stream);
		Stream = nullptr;
	}
	if (AVIFile != nullptr) {
		AVIFileRelease(AVIFile);
		AVIFile = nullptr;
	}
	if (avi_library_initialized) {
		AVIFileExit();
	}
	Mode = RAW;
}

void FrameGrabClass::GrabAVI(void *BitmapPointer)
{
	TryGrab(BitmapPointer);
}

void FrameGrabClass::GrabRawFrame(void * /*BitmapPointer*/)
{

}


void FrameGrabClass::ConvertGrab(void *BitmapPointer)
{
	if (!IsReady() || BitmapPointer == nullptr || Bitmap == nullptr) {
		return;
	}

	const unsigned long long source_size =
		static_cast<unsigned long long>(GetWidth()) * 4ULL;
	if (source_size > std::numeric_limits<int>::max() ||
		!ConvertBGRA32ToBGR24(BitmapPointer, static_cast<int>(source_size), Bitmap,
			GetRowStride(), GetWidth(), GetHeight())) {
		return;
	}
	TryGrab(Bitmap);
}


void FrameGrabClass::Grab(void *BitmapPointer)
{
	if (Mode == AVI) {
		TryGrab(BitmapPointer);
	} else {
		GrabRawFrame(BitmapPointer);
	}
}

bool FrameGrabClass::TryGrab(void *BitmapPointer)
{
	if (!IsReady() || BitmapPointer == nullptr || Counter == std::numeric_limits<int>::max()) {
		return false;
	}

	const HRESULT hr = AVIStreamWrite(Stream, Counter, 1, BitmapPointer,
		BitmapInfoHeader.biSizeImage, AVIIF_KEYFRAME, nullptr, nullptr);
	if (FAILED(hr)) {
		char buffer[128];
		std::snprintf(buffer, sizeof(buffer), "avi write error %lx/%ld\n",
			static_cast<unsigned long>(hr), static_cast<long>(hr));
		OutputDebugStringA(buffer);
		CleanupAVI();
		return false;
	}

	++Counter;
	return true;
}

bool FrameGrabClass::IsReady() const
{
	return Mode == AVI && AVIFile != nullptr && Stream != nullptr && Bitmap != nullptr &&
		BitmapInfoHeader.biSizeImage != 0;
}

int FrameGrabClass::GetWidth() const
{
	return BitmapInfoHeader.biWidth;
}

int FrameGrabClass::GetHeight() const
{
	return BitmapInfoHeader.biHeight;
}

unsigned int FrameGrabClass::GetRowStride() const
{
	return CalculateRowStride(BitmapInfoHeader.biWidth, BitmapInfoHeader.biBitCount);
}

unsigned int FrameGrabClass::GetBufferSize() const
{
	return BitmapInfoHeader.biSizeImage;
}

unsigned int FrameGrabClass::CalculateRowStride(int width, int bitdepth)
{
	if (width <= 0 || bitdepth <= 0) {
		return 0;
	}

	const unsigned long long bit_count =
		static_cast<unsigned long long>(width) * static_cast<unsigned long long>(bitdepth);
	const unsigned long long stride = ((bit_count + 31ULL) & ~31ULL) / 8ULL;
	if (stride > std::numeric_limits<unsigned int>::max()) {
		return 0;
	}
	return static_cast<unsigned int>(stride);
}

bool FrameGrabClass::ConvertBGRA32ToBGR24(const void *source, int source_pitch,
	void *destination, unsigned int destination_stride, int width, int height)
{
	if (source == nullptr || destination == nullptr || source_pitch <= 0 ||
		width <= 0 || height <= 0) {
		return false;
	}

	const unsigned long long source_pixel_bytes = static_cast<unsigned long long>(width) * 4ULL;
	const unsigned long long destination_pixel_bytes = static_cast<unsigned long long>(width) * 3ULL;
	const unsigned int minimum_destination_stride = CalculateRowStride(width, 24);
	if (source_pixel_bytes > static_cast<unsigned long long>(std::numeric_limits<int>::max()) ||
		destination_pixel_bytes > std::numeric_limits<unsigned int>::max() ||
		static_cast<unsigned long long>(source_pitch) < source_pixel_bytes ||
		minimum_destination_stride == 0 || destination_stride < minimum_destination_stride) {
		return false;
	}

	const size_t row_count_minus_one = static_cast<size_t>(height - 1);
	const size_t maximum_offset = std::numeric_limits<size_t>::max();
	const size_t source_row_bytes = static_cast<size_t>(source_pixel_bytes);
	if (row_count_minus_one > (maximum_offset - source_row_bytes) /
			static_cast<size_t>(source_pitch) ||
		row_count_minus_one > (maximum_offset - static_cast<size_t>(destination_stride)) /
			destination_stride) {
		return false;
	}

	const auto *source_bytes = static_cast<const unsigned char *>(source);
	auto *destination_bytes = static_cast<unsigned char *>(destination);
	const size_t copied_bytes = static_cast<size_t>(destination_pixel_bytes);
	for (int source_y = 0; source_y < height; ++source_y) {
		const auto *source_row = source_bytes + static_cast<size_t>(source_y) * source_pitch;
		auto *destination_row = destination_bytes +
			static_cast<size_t>(height - source_y - 1) * destination_stride;
		for (int x = 0; x < width; ++x) {
			destination_row[3 * x] = source_row[4 * x];
			destination_row[3 * x + 1] = source_row[4 * x + 1];
			destination_row[3 * x + 2] = source_row[4 * x + 2];
		}
		std::memset(destination_row + copied_bytes, 0, destination_stride - copied_bytes);
	}
	return true;
}


void FrameGrabClass::ConvertFrame(void *BitmapPointer)
{
	if (BitmapPointer == nullptr || Bitmap == nullptr) {
		return;
	}
	const unsigned long long source_stride =
		static_cast<unsigned long long>(GetWidth()) * 4ULL;
	if (source_stride <= std::numeric_limits<int>::max()) {
		ConvertBGRA32ToBGR24(BitmapPointer, static_cast<int>(source_stride), Bitmap,
			GetRowStride(), GetWidth(), GetHeight());
	}
}
