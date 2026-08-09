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
#include <cstdio>
#include <cstring>
#include <io.h>
#include <limits>
#include <string>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FrameGrabClass::FrameGrabClass(const char *filename, MODE mode, int width, int height, int bitcount, float framerate) :
	FrameRate(framerate),
	Mode(mode),
	Counter(0),
	Width(width),
	Height(height),
	BufferStride(Calculate_Row_Stride(width, bitcount)),
	AVIInitialized(false),
	Ready(false),
	LastError(S_OK),
	AVIFile(nullptr),
	Bitmap(nullptr),
	Stream(nullptr)
{
	std::memset(&AVIStreamInfo, 0, sizeof(AVIStreamInfo));
	std::memset(&BitmapInfoHeader, 0, sizeof(BitmapInfoHeader));

	if (Mode != AVI) {
		return;
	}

	const unsigned long long image_size =
		static_cast<unsigned long long>(BufferStride) * static_cast<unsigned long long>(Height);
	if (filename == nullptr || filename[0] == '\0' || Width <= 0 || Height <= 0 ||
		bitcount <= 0 || FrameRate <= 0.0f || BufferStride == 0 ||
		image_size > std::numeric_limits<DWORD>::max()) {
		LastError = E_INVALIDARG;
		Mode = RAW;
		return;
	}

	AVIFileInit();
	AVIInitialized = true;

	// Find the first free file with this prefix.
	int counter = 0;
	int result;
	std::string file;
	do {
		file = std::string(filename) + std::to_string(counter++) + ".AVI";
		result = _access(file.c_str(), 0);
	} while (result != -1);

	HRESULT hr = AVIFileOpenA(&AVIFile, file.c_str(), OF_WRITE | OF_CREATE, nullptr);
	if (FAILED(hr)) {
		LastError = hr;
		OutputDebugStringA("Unable to open AVI movie capture file.\n");
		CleanupAVI();
		return;
	}

	// Set the format of the new stream.
	BitmapInfoHeader.biWidth = Width;
	BitmapInfoHeader.biHeight = Height;
	BitmapInfoHeader.biBitCount = static_cast<unsigned short>(bitcount);
	BitmapInfoHeader.biSizeImage = static_cast<DWORD>(image_size);
	BitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfoHeader.biPlanes = 1;
	BitmapInfoHeader.biCompression = BI_RGB;
	BitmapInfoHeader.biXPelsPerMeter = 1;
	BitmapInfoHeader.biYPelsPerMeter = 1;
	BitmapInfoHeader.biClrUsed = 0;
	BitmapInfoHeader.biClrImportant = 0;

	AVIStreamInfo.fccType = streamtypeVIDEO;
	AVIStreamInfo.fccHandler = mmioFOURCC('M','S','V','C');
	AVIStreamInfo.dwScale = 1;
	AVIStreamInfo.dwRate = static_cast<DWORD>(FrameRate);
	if (AVIStreamInfo.dwRate == 0) {
		AVIStreamInfo.dwRate = 1;
	}
	AVIStreamInfo.dwSuggestedBufferSize = BitmapInfoHeader.biSizeImage;
	SetRect(&AVIStreamInfo.rcFrame, 0, 0, Width, Height);
	AVIStreamInfo.szName[0] = 'G';

	hr = AVIFileCreateStreamA(AVIFile, &Stream, &AVIStreamInfo);
	if (FAILED(hr)) {
		LastError = hr;
		CleanupAVI();
		return;
	}

	hr = AVIStreamSetFormat(Stream, 0, &BitmapInfoHeader, sizeof(BitmapInfoHeader));
	if (FAILED(hr)) {
		LastError = hr;
		CleanupAVI();
		return;
	}

	Bitmap = static_cast<int *>(GlobalAllocPtr(GMEM_MOVEABLE, BitmapInfoHeader.biSizeImage));
	if (Bitmap == nullptr) {
		LastError = E_OUTOFMEMORY;
		CleanupAVI();
		return;
	}

	Ready = true;
}

FrameGrabClass::~FrameGrabClass()
{
	CleanupAVI();
}

void FrameGrabClass::CleanupAVI()
{
	Ready = false;
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

	if (AVIInitialized) {
		AVIFileExit();
		AVIInitialized = false;
	}
	Mode = RAW;
}

bool FrameGrabClass::GrabAVI(void *BitmapPointer)
{
	if (!Ready || Stream == nullptr || BitmapPointer == nullptr) {
		LastError = E_POINTER;
		Ready = false;
		return false;
	}

	const HRESULT hr = AVIStreamWrite(Stream, Counter, 1, BitmapPointer,
		BitmapInfoHeader.biSizeImage, AVIIF_KEYFRAME, nullptr, nullptr);
	if (FAILED(hr)) {
		LastError = hr;
		Ready = false;
		char buf[256];
		std::snprintf(buf, sizeof(buf), "avi write error %lx/%ld\n",
			static_cast<unsigned long>(hr), static_cast<long>(hr));
		OutputDebugStringA(buf);
		return false;
	}

	++Counter;
	return true;
}

bool FrameGrabClass::GrabRawFrame(void * /*BitmapPointer*/)
{
	return false;
}

void FrameGrabClass::ConvertGrab(void *BitmapPointer)
{
	if (!Ready || BitmapPointer == nullptr || Bitmap == nullptr) {
		return;
	}

	ConvertFrame(BitmapPointer);
	Grab(Bitmap);
}

bool FrameGrabClass::Grab(void *BitmapPointer)
{
	if (Mode == AVI) {
		return GrabAVI(BitmapPointer);
	}

	return GrabRawFrame(BitmapPointer);
}

unsigned int FrameGrabClass::Calculate_Row_Stride(int width, int bitdepth)
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

void FrameGrabClass::ConvertFrame(void *BitmapPointer)
{
	int width = BitmapInfoHeader.biWidth;
	int height = BitmapInfoHeader.biHeight;
	int *image = static_cast<int *>(BitmapPointer);

	// Copy the data, doing a vertical flip and byte re-ordering of the pixel longwords.
	int y = height;
	while (y--) {
		int x = width;
		int yoffset = y * width;
		int yoffset2 = (height - y) * width;
		while (x--) {
			int *source = &image[yoffset + x];
			int *dest = &Bitmap[yoffset2 + x];
			*dest = *source;
			unsigned char *c = reinterpret_cast<unsigned char *>(dest);
			c[3] = c[0];
			c[0] = c[2];
			c[2] = c[3];
			c[3] = 0;
		}
	}
}
