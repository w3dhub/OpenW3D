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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * DX8Backend implementation.                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dx8backend.h"
#include "dx8wrapper.h"

DX8Backend::DX8Backend()
{
}

DX8Backend::~DX8Backend()
{
}

bool DX8Backend::Init(void * hwnd, bool lite)
{
    return DX8Wrapper::Init(hwnd, lite);
}

void DX8Backend::Shutdown()
{
    DX8Wrapper::Shutdown();
}

bool DX8Backend::Set_Any_Render_Device()
{
    return DX8Wrapper::Set_Any_Render_Device();
}

bool DX8Backend::Set_Render_Device(const char * dev_name, int width, int height, int bits, int windowed, bool resize_window)
{
    return DX8Wrapper::Set_Render_Device(dev_name, width, height, bits, windowed, resize_window);
}

bool DX8Backend::Set_Render_Device(int dev, int resx, int resy, int bits, int windowed, bool resize_window)
{
    return DX8Wrapper::Set_Render_Device(dev, resx, resy, bits, windowed, resize_window);
}

bool DX8Backend::Set_Next_Render_Device()
{
    return DX8Wrapper::Set_Next_Render_Device();
}

bool DX8Backend::Toggle_Windowed()
{
    return DX8Wrapper::Toggle_Windowed();
}

bool DX8Backend::Is_Windowed()
{
    return DX8Wrapper::Is_Windowed();
}

int DX8Backend::Get_Render_Device_Count()
{
    return DX8Wrapper::Get_Render_Device_Count();
}

int DX8Backend::Get_Render_Device()
{
    return DX8Wrapper::Get_Render_Device();
}

const RenderDeviceDescClass & DX8Backend::Get_Render_Device_Desc(int deviceidx)
{
    return DX8Wrapper::Get_Render_Device_Desc(deviceidx);
}

const char * DX8Backend::Get_Render_Device_Name(int device_index)
{
    return DX8Wrapper::Get_Render_Device_Name(device_index);
}

bool DX8Backend::Set_Device_Resolution(int width, int height, int bits, int windowed, bool resize_window)
{
    return DX8Wrapper::Set_Device_Resolution(width, height, bits, windowed, resize_window);
}

void DX8Backend::Get_Device_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
    DX8Wrapper::Get_Device_Resolution(set_w, set_h, set_bits, set_windowed);
}

void DX8Backend::Get_Render_Target_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
    DX8Wrapper::Get_Render_Target_Resolution(set_w, set_h, set_bits, set_windowed);
}

int DX8Backend::Get_Device_Resolution_Width()
{
    return DX8Wrapper::Get_Device_Resolution_Width();
}

int DX8Backend::Get_Device_Resolution_Height()
{
    return DX8Wrapper::Get_Device_Resolution_Height();
}

bool DX8Backend::Registry_Save_Render_Device(const char * sub_key)
{
    return DX8Wrapper::Registry_Save_Render_Device(sub_key);
}

bool DX8Backend::Registry_Load_Render_Device(const char * sub_key, bool resize_window)
{
    return DX8Wrapper::Registry_Load_Render_Device(sub_key, resize_window);
}

bool DX8Backend::Registry_Save_Render_Device(const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth)
{
    return DX8Wrapper::Registry_Save_Render_Device(sub_key, device, width, height, depth, windowed, texture_depth);
}

bool DX8Backend::Registry_Load_Render_Device(const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth)
{
    return DX8Wrapper::Registry_Load_Render_Device(sub_key, device, device_len, width, height, depth, windowed, texture_depth);
}

void DX8Backend::Begin_Scene()
{
    DX8Wrapper::Begin_Scene();
}

void DX8Backend::End_Scene(bool flip_frame)
{
    DX8Wrapper::End_Scene(flip_frame);
}

void DX8Backend::Flip_To_Primary()
{
    DX8Wrapper::Flip_To_Primary();
}

void DX8Backend::Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z, unsigned int stencil)
{
    DX8Wrapper::Clear(clear_color, clear_z_stencil, color, z, stencil);
}

void DX8Backend::Set_Swap_Interval(int swap)
{
    DX8Wrapper::Set_Swap_Interval(swap);
}

int DX8Backend::Get_Swap_Interval()
{
    return DX8Wrapper::Get_Swap_Interval();
}

void DX8Backend::Set_Texture_Bitdepth(int depth)
{
    DX8Wrapper::Set_Texture_Bitdepth(depth);
}

int DX8Backend::Get_Texture_Bitdepth()
{
    return DX8Wrapper::Get_Texture_Bitdepth();
}
