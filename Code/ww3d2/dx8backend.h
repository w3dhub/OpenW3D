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

#pragma once

#ifndef DX8BACKEND_H
#define DX8BACKEND_H

#include "ww3dbackend.h"

/**
 * DX8Backend
 *
 * DX8/DX9 implementation of the WW3DBackend interface.
 * Wraps the existing DX8Wrapper static API via composition — does not
 * inherit from DX8Wrapper. Translates between BackendDeviceCapabilities
 * and the underlying DX8/DX9 adapter enumeration API.
 */
class DX8Backend : public WW3DBackend
{
public:
    DX8Backend();
    virtual ~DX8Backend();

    //========================================================================
    // WW3DBackend interface — Device enumeration
    //========================================================================

    /**
     * Enumerate all DX8/DX9 adapters via Direct3D()->GetAdapterIdentifier().
     *
     * Populates the provided BackendDeviceCapabilities array with information
     * from D3DADAPTER_IDENTIFIER9 for each adapter.
     *
     * @param devices Output array to fill with adapter capabilities
     * @param max_devices Maximum number of entries to write into devices[]
     * @return Number of adapters found (may be less than max_devices)
     */
    virtual int Enumerate_Devices(BackendDeviceCapabilities* devices, int max_devices) override;

    //========================================================================
    // WW3DBackend interface — Initialization
    //========================================================================
    virtual bool Init(void* hwnd, bool lite = false) override;
    virtual void Shutdown() override;

    //========================================================================
    // WW3DBackend interface — Device selection
    //========================================================================
    virtual bool Set_Any_Render_Device() override;
    virtual bool Set_Render_Device(const char* dev_name) override;
    virtual bool Set_Render_Device(int adapter_index) override;
    virtual bool Set_Next_Render_Device() override;
    virtual bool Toggle_Windowed() override;
    virtual bool Is_Windowed() override;

    //========================================================================
    // WW3DBackend interface — Device properties
    //========================================================================
    virtual int Get_Render_Device_Count() override;
    virtual int Get_Render_Device() override;
    virtual const BackendDeviceCapabilities& Get_Render_Device_Desc(int adapter_index) override;
    virtual const char* Get_Render_Device_Name(int adapter_index) override;

    //========================================================================
    // WW3DBackend interface — Resolution
    //========================================================================
    virtual bool Set_Device_Resolution(int width = -1, int height = -1, int bits = -1, int windowed = -1, bool resize_window = false) override;
    virtual void Get_Device_Resolution(int& width, int& height, int& bits, bool& windowed) override;
    virtual void Get_Render_Target_Resolution(int& width, int& height, int& bits, bool& windowed) override;
    virtual int Get_Device_Resolution_Width() override;
    virtual int Get_Device_Resolution_Height() override;

    //========================================================================
    // WW3DBackend interface — Registry
    //========================================================================
    virtual bool Registry_Save_Render_Device(const char* sub_key) override;
    virtual bool Registry_Load_Render_Device(const char* sub_key, bool resize_window) override;

    //========================================================================
    // WW3DBackend interface — Scene framing
    //========================================================================
    virtual void Begin_Scene() override;
    virtual void End_Scene(bool flip_frame = true) override;
    virtual void Flip_To_Primary() override;

    //========================================================================
    // WW3DBackend interface — Clear
    //========================================================================
    virtual void Clear(bool clear_color, bool clear_z_stencil, const Vector3& color, float z = 1.0f, unsigned int stencil = 0) override;

    //========================================================================
    // WW3DBackend interface — VSync
    //========================================================================
    virtual void Set_Swap_Interval(int swap_interval) override;
    virtual int Get_Swap_Interval() override;

    //========================================================================
    // WW3DBackend interface — Texture depth
    //========================================================================
    virtual void Set_Texture_Bitdepth(int depth) override;
    virtual int Get_Texture_Bitdepth() override;

private:
    bool m_Initialized;
    bool m_IsWindowed;
    int m_VSync;
};

#endif // DX8BACKEND_H
