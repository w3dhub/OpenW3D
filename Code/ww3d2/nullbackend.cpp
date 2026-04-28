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

#include "nullbackend.h"
#include "rddesc.h"

namespace
{
	const int DEFAULT_WIDTH = 640;
	const int DEFAULT_HEIGHT = 480;
	const int DEFAULT_BIT_DEPTH = 16;
}

NullBackend::NullBackend() :
	Width(DEFAULT_WIDTH),
	Height(DEFAULT_HEIGHT),
	BitDepth(DEFAULT_BIT_DEPTH),
	Windowed(true),
	SwapInterval(0),
	TextureBitDepth(DEFAULT_BIT_DEPTH)
{
}

NullBackend::~NullBackend()
{
}

bool NullBackend::Init(void * /*hwnd*/, bool /*lite*/)
{
	return true;
}

void NullBackend::Shutdown()
{
}

bool NullBackend::Set_Any_Render_Device()
{
	return true;
}

bool NullBackend::Set_Render_Device(const char * /*dev_name*/, int width, int height, int bits, int windowed, bool /*resize_window*/)
{
	return Set_Device_Resolution(width, height, bits, windowed, false);
}

bool NullBackend::Set_Render_Device(int /*dev*/, int resx, int resy, int bits, int windowed, bool /*resize_window*/)
{
	return Set_Device_Resolution(resx, resy, bits, windowed, false);
}

bool NullBackend::Set_Next_Render_Device()
{
	return true;
}

bool NullBackend::Toggle_Windowed()
{
	Windowed = !Windowed;
	return true;
}

bool NullBackend::Is_Windowed()
{
	return Windowed;
}

int NullBackend::Get_Render_Device_Count()
{
	return 1;
}

int NullBackend::Get_Render_Device()
{
	return 0;
}

const RenderDeviceDescClass & NullBackend::Get_Render_Device_Desc(int /*deviceidx*/)
{
	static RenderDeviceDescClass desc;
	return desc;
}

const char * NullBackend::Get_Render_Device_Name(int /*device_index*/)
{
	return "Null Renderer";
}

bool NullBackend::Set_Device_Resolution(int width, int height, int bits, int windowed, bool /*resize_window*/)
{
	if (width > 0) {
		Width = width;
	}
	if (height > 0) {
		Height = height;
	}
	if (bits > 0) {
		BitDepth = bits;
	}
	if (windowed != -1) {
		Windowed = (windowed != 0);
	}
	return true;
}

void NullBackend::Get_Device_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
	set_w = Width;
	set_h = Height;
	set_bits = BitDepth;
	set_windowed = Windowed;
}

void NullBackend::Get_Render_Target_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
	Get_Device_Resolution(set_w, set_h, set_bits, set_windowed);
}

int NullBackend::Get_Device_Resolution_Width()
{
	return Width;
}

int NullBackend::Get_Device_Resolution_Height()
{
	return Height;
}

bool NullBackend::Registry_Save_Render_Device(const char * /*sub_key*/)
{
	return true;
}

bool NullBackend::Registry_Load_Render_Device(const char * /*sub_key*/, bool /*resize_window*/)
{
	return true;
}

bool NullBackend::Registry_Save_Render_Device(const char */*sub_key*/, int /*device*/, int /*width*/, int /*height*/, int /*depth*/, bool /*windowed*/, int /*texture_depth*/)
{
	return true;
}

bool NullBackend::Registry_Load_Render_Device(const char * /*sub_key*/, char */*device*/, int /*device_len*/, int &width, int &height, int &depth, int &windowed, int &texture_depth)
{
	width = Width;
	height = Height;
	depth = BitDepth;
	windowed = Windowed ? 1 : 0;
	texture_depth = TextureBitDepth;
	return true;
}

void NullBackend::Begin_Scene()
{
}

void NullBackend::End_Scene(bool /*flip_frame*/)
{
}

void NullBackend::Flip_To_Primary()
{
}

void NullBackend::Clear(bool /*clear_color*/, bool /*clear_z_stencil*/, const Vector3 &/*color*/, float /*z*/, unsigned int /*stencil*/)
{
}

void NullBackend::Set_Swap_Interval(int swap)
{
	SwapInterval = swap;
}

int NullBackend::Get_Swap_Interval()
{
	return SwapInterval;
}

void NullBackend::Set_Texture_Bitdepth(int depth)
{
	TextureBitDepth = depth;
}

int NullBackend::Get_Texture_Bitdepth()
{
	return TextureBitDepth;
}

void NullBackend::Set_Viewport(const void* /*viewport*/)
{
}

void NullBackend::Set_DX8_Render_State(int /*state*/, unsigned /*value*/)
{
}

void NullBackend::Set_Light_Environment(const void* /*env*/)
{
}

void* NullBackend::_Get_DX8_Front_Buffer()
{
	return nullptr;
}
