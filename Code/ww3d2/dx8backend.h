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
 * DX8Backend - concrete WW3DBackend implementation wrapping DX8Wrapper static methods.         *
 * DX8Wrapper stays standalone and does NOT inherit from WW3DBackend.                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef DX8BACKEND_H
#define DX8BACKEND_H

#include "ww3dbackend.h"

// DX8Wrapper static methods are called directly from here.
// No include of dx8wrapper.h at header level to keep D3D9 out of the ww3d2 include chain.
class DX8Wrapper;

class DX8Backend : public WW3DBackend
{
public:
    DX8Backend();
    virtual ~DX8Backend();

    // WW3DBackend interface - delegate to DX8Wrapper static methods
    virtual bool Init(void * hwnd, bool lite = false) override;
    virtual void Shutdown() override;

    virtual bool Set_Any_Render_Device() override;
    virtual bool Set_Render_Device(const char * dev_name, int width = -1, int height = -1, int bits = -1, int windowed = -1, bool resize_window = false) override;
    virtual bool Set_Render_Device(int dev = -1, int resx = -1, int resy = -1, int bits = -1, int windowed = -1, bool resize_window = false) override;
    virtual bool Set_Next_Render_Device() override;
    virtual bool Toggle_Windowed() override;
    virtual bool Is_Windowed() override;

    virtual int Get_Render_Device_Count() override;
    virtual int Get_Render_Device() override;
    virtual const RenderDeviceDescClass & Get_Render_Device_Desc(int deviceidx) override;
    virtual const char * Get_Render_Device_Name(int device_index) override;
    virtual bool Set_Device_Resolution(int width = -1, int height = -1, int bits = -1, int windowed = -1, bool resize_window = false) override;
    virtual void Get_Device_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed) override;
    virtual void Get_Render_Target_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed) override;
    virtual int Get_Device_Resolution_Width() override;
    virtual int Get_Device_Resolution_Height() override;

    virtual bool Registry_Save_Render_Device(const char * sub_key) override;
    virtual bool Registry_Load_Render_Device(const char * sub_key, bool resize_window) override;
    virtual bool Registry_Save_Render_Device(const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth) override;
    virtual bool Registry_Load_Render_Device(const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth) override;

    virtual void Begin_Scene() override;
    virtual void End_Scene(bool flip_frame = true) override;
    virtual void Flip_To_Primary() override;

    virtual void Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z = 1.0f, unsigned int stencil = 0) override;

    virtual void SetRenderType(SceneClass::PolyRenderType) override;

    virtual void Set_Swap_Interval(int swap) override;
    virtual int Get_Swap_Interval() override;

    virtual void Set_Texture_Bitdepth(int depth) override;
    virtual int Get_Texture_Bitdepth() override;
};

#endif // DX8BACKEND_H
