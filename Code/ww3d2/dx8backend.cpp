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

#include "dx8backend.h"
#include "dx8wrapper.h"
#include "rddesc.h"

/*
** Include dx8wrapper inline implementation for dx8backend's use of
** DX8Wrapper static methods.
** This pulls in the full DX8 implementation chain.
*/
#include "dx8wrapper_impl.h"

// Disable warning about unused parameters in this file
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4100)
#endif

DX8Backend::DX8Backend()
    : m_Initialized(false)
    , m_IsWindowed(true)
    , m_VSync(1)
{
}

DX8Backend::~DX8Backend()
{
}

//==========================================================================
// Device enumeration
//==========================================================================

int DX8Backend::Enumerate_Devices(BackendDeviceCapabilities* devices, int max_devices)
{
    // Populate the DX8 internal device list first
    DX8Wrapper::Enumerate_Devices();

    int count = DX8Wrapper::Get_Render_Device_Count();
    int count_to_report = (count < max_devices) ? count : max_devices;

    for (int i = 0; i < count_to_report; i++) {
        const RenderDeviceDescClass& desc = DX8Wrapper::Get_Render_Device_Desc(i);

        BackendDeviceCapabilities& dev = devices[i];

        dev.AdapterIndex = i;
        dev.IsHardware = true; // DX8/DX9 adapters are physical GPUs
        dev.IsExternal = false;
        dev.DedicatedVideoMemory = -1; // DX8 doesn't expose VRAM at this level
        dev.DedicatedSystemMemory = -1;
        dev.SharedSystemMemory = -1;

        // Device name
        const char* name = desc.Get_Device_Name();
        if (name) {
            strncpy(dev.DeviceName, name, sizeof(dev.DeviceName) - 1);
            dev.DeviceName[sizeof(dev.DeviceName) - 1] = '\0';
        } else {
            dev.DeviceName[0] = '\0';
        }

        // Driver name
        strncpy(dev.DriverName, desc.DriverName, sizeof(dev.DriverName) - 1);
        dev.DriverName[sizeof(dev.DriverName) - 1] = '\0';

        // Driver version from D3DADAPTER_IDENTIFIER9
        const D3DADAPTER_IDENTIFIER9& id = desc.Get_Adapter_Identifier();
        snprintf(dev.DriverVersion, sizeof(dev.DriverVersion), "%d.%d.%d.%d",
            HIWORD(id.DriverVersion.HighPart),
            LOWORD(id.DriverVersion.HighPart),
            HIWORD(id.DriverVersion.LowPart),
            LOWORD(id.DriverVersion.LowPart));

        dev.VendorId = static_cast<int>(id.VendorId);
        dev.DeviceId = static_cast<int>(id.DeviceId);
        dev.SubSystemId = static_cast<int>(id.SubSysId);
        dev.RevisionId = static_cast<int>(id.Revision);
    }

    return count_to_report;
}

//==========================================================================
// Initialization
//==========================================================================

bool DX8Backend::Init(void* hwnd, bool lite)
{
    // DX8Wrapper handles its own init — just record that we're initialized
    // The actual Init call goes through DX8Wrapper directly for now
    m_Initialized = true;
    return true;
}

void DX8Backend::Shutdown()
{
    m_Initialized = false;
}

//==========================================================================
// Device selection
//==========================================================================

bool DX8Backend::Set_Any_Render_Device()
{
    return DX8Wrapper::Set_Any_Render_Device() != WW3D_OK ? false : true;
}

bool DX8Backend::Set_Render_Device(const char* dev_name)
{
    return DX8Wrapper::Set_Render_Device(dev_name) != WW3D_OK ? false : true;
}

bool DX8Backend::Set_Render_Device(int adapter_index)
{
    return DX8Wrapper::Set_Render_Device(adapter_index) != WW3D_OK ? false : true;
}

bool DX8Backend::Set_Next_Render_Device()
{
    return DX8Wrapper::Set_Next_Render_Device() != WW3D_OK ? false : true;
}

bool DX8Backend::Toggle_Windowed()
{
    return DX8Wrapper::Toggle_Windowed() != WW3D_OK ? false : true;
}

bool DX8Backend::Is_Windowed()
{
    return DX8Wrapper::Is_Windowed();
}

//==========================================================================
// Device properties
//==========================================================================

int DX8Backend::Get_Render_Device_Count()
{
    return DX8Wrapper::Get_Render_Device_Count();
}

int DX8Backend::Get_Render_Device()
{
    return DX8Wrapper::Get_Render_Device();
}

const BackendDeviceCapabilities& DX8Backend::Get_Render_Device_Desc(int adapter_index)
{
    // DX8Wrapper doesn't have a way to get cached desc without re-fetching,
    // so we use a static temp. Caller should call Enumerate_Devices() first
    // to populate the DX8 internal list.
    static BackendDeviceCapabilities dev;
    if (adapter_index < 0 || adapter_index >= DX8Wrapper::Get_Render_Device_Count()) {
        dev = BackendDeviceCapabilities();
        return dev;
    }

    const RenderDeviceDescClass& desc = DX8Wrapper::Get_Render_Device_Desc(adapter_index);
    dev.AdapterIndex = adapter_index;
    dev.IsHardware = true;
    dev.IsExternal = false;
    dev.DedicatedVideoMemory = -1;
    dev.DedicatedSystemMemory = -1;
    dev.SharedSystemMemory = -1;

    const char* name = desc.Get_Device_Name();
    if (name) {
        strncpy(dev.DeviceName, name, sizeof(dev.DeviceName) - 1);
        dev.DeviceName[sizeof(dev.DeviceName) - 1] = '\0';
    } else {
        dev.DeviceName[0] = '\0';
    }

    strncpy(dev.DriverName, desc.DriverName, sizeof(dev.DriverName) - 1);
    dev.DriverName[sizeof(dev.DriverName) - 1] = '\0';

    const D3DADAPTER_IDENTIFIER9& id = desc.Get_Adapter_Identifier();
    snprintf(dev.DriverVersion, sizeof(dev.DriverVersion), "%d.%d.%d.%d",
        HIWORD(id.DriverVersion.HighPart),
        LOWORD(id.DriverVersion.HighPart),
        HIWORD(id.DriverVersion.LowPart),
        LOWORD(id.DriverVersion.LowPart));
    dev.VendorId = static_cast<int>(id.VendorId);
    dev.DeviceId = static_cast<int>(id.DeviceId);
    dev.SubSystemId = static_cast<int>(id.SubSysId);
    dev.RevisionId = static_cast<int>(id.Revision);

    return dev;
}

const char* DX8Backend::Get_Render_Device_Name(int adapter_index)
{
    static char name[sizeof(BackendDeviceCapabilities().DeviceName)];
    name[0] = '\0';
    if (adapter_index < 0 || adapter_index >= DX8Wrapper::Get_Render_Device_Count()) {
        return name;
    }
    const char* n = DX8Wrapper::Get_Render_Device_Name(adapter_index);
    if (n) {
        strncpy(name, n, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
    }
    return name;
}

//==========================================================================
// Resolution
//==========================================================================

bool DX8Backend::Set_Device_Resolution(int width, int height, int bits, int windowed, bool resize_window)
{
    WW3DErrorType err = WW3D::Set_Device_Resolution(width, height, bits, windowed, resize_window);
    return err != WW3D_OK ? false : true;
}

void DX8Backend::Get_Device_Resolution(int& width, int& height, int& bits, bool& windowed)
{
    WW3D::Get_Device_Resolution(width, height, bits, windowed);
}

void DX8Backend::Get_Render_Target_Resolution(int& width, int& height, int& bits, bool& windowed)
{
    WW3D::Get_Render_Target_Resolution(width, height, bits, windowed);
}

int DX8Backend::Get_Device_Resolution_Width()
{
    int w, h, bits;
    bool windowed;
    WW3D::Get_Device_Resolution(w, h, bits, windowed);
    return w;
}

int DX8Backend::Get_Device_Resolution_Height()
{
    int w, h, bits;
    bool windowed;
    WW3D::Get_Device_Resolution(w, h, bits, windowed);
    return h;
}

//==========================================================================
// Registry
//==========================================================================

bool DX8Backend::Registry_Save_Render_Device(const char* sub_key)
{
    return DX8Wrapper::Registry_Save_Render_Device(sub_key) != WW3D_OK ? false : true;
}

bool DX8Backend::Registry_Load_Render_Device(const char* sub_key, bool resize_window)
{
    return DX8Wrapper::Registry_Load_Render_Device(sub_key, resize_window) != WW3D_OK ? false : true;
}

//==========================================================================
// Scene framing
//==========================================================================

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

//==========================================================================
// Clear
//==========================================================================

void DX8Backend::Clear(bool clear_color, bool clear_z_stencil, const Vector3& color, float z, unsigned int stencil)
{
    DX8Wrapper::Clear(clear_color, clear_z_stencil, color, z, stencil);
}

//==========================================================================
// VSync
//==========================================================================

void DX8Backend::Set_Swap_Interval(int swap_interval)
{
    m_VSync = swap_interval;
    DX8Wrapper::Set_Swap_Interval(swap_interval);
}

int DX8Backend::Get_Swap_Interval()
{
    return m_VSync;
}

//==========================================================================
// Texture depth
//==========================================================================

void DX8Backend::Set_Texture_Bitdepth(int depth)
{
    DX8Wrapper::Set_Texture_Bitdepth(depth);
}

int DX8Backend::Get_Texture_Bitdepth()
{
    return DX8Wrapper::Get_Texture_Bitdepth();
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
