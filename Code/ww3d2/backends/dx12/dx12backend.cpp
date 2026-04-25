/**
 * DX12Backend — stub DirectX 12 render backend implementation.
 *
 * STATUS: Not implemented. All methods return false or no-op.
 *
 * This file exists to:
 * - Establish the DX12 backend's concrete shape matching DX12Backend class
 * - Provide a compile-valid stub so the CMake plumbing can reference it
 * - Serve as the implementation entry point when real DX12 work begins
 *
 * Build enable: cmake -DENABLE_DX12_BACKEND=ON
 * Note: requires Windows 10 SDK (d3d12.h, dxgi1_6.h) — not available on Linux
 */

#include "backends/dx12/dx12backend.h"

DX12Backend::DX12Backend() = default;
DX12Backend::~DX12Backend() = default;

bool DX12Backend::Init(void * /*hwnd*/, bool /*lite*/)
{
    // TODO: create D3D12 device via D3D12CreateDevice()
    // TODO: create command queue (D3D12_COMMAND_LIST_TYPE_DIRECT)
    // TODO: create swapchain via CreateSwapChain()
    // TODO: create descriptor heaps (RTV, DSV, SRV)
    // TODO: create command allocators and command lists
    m_initialized = false;
    return false;
}

void DX12Backend::Shutdown()
{
    // TODO: wait for GPU idle
    // TODO: release all D3D12 COM objects
    m_initialized = false;
}

bool DX12Backend::Set_Any_Render_Device() { return false; }
bool DX12Backend::Set_Render_Device(const char * /*dev_name*/, int, int, int, int, bool) { return false; }
bool DX12Backend::Set_Render_Device(int, int, int, int, int, bool) { return false; }
bool DX12Backend::Set_Next_Render_Device() { return false; }
bool DX12Backend::Toggle_Windowed() { return false; }
bool DX12Backend::Is_Windowed() { return false; }

int DX12Backend::Get_Render_Device_Count() { return 0; }
int DX12Backend::Get_Render_Device() { return -1; }

const RenderDeviceDescClass & DX12Backend::Get_Render_Device_Desc(int /*deviceidx*/)
{
    static RenderDeviceDescClass null_desc;
    return null_desc;
}

const char * DX12Backend::Get_Render_Device_Name(int /*device_index*/) { return "DX12 (not implemented)"; }

bool DX12Backend::Set_Device_Resolution(int, int, int, int, bool) { return false; }
void DX12Backend::Get_Device_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
    set_w = set_h = set_bits = 0;
    set_windowed = false;
}
void DX12Backend::Get_Render_Target_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
    set_w = set_h = set_bits = 0;
    set_windowed = false;
}
int DX12Backend::Get_Device_Resolution_Width() { return 0; }
int DX12Backend::Get_Device_Resolution_Height() { return 0; }

bool DX12Backend::Registry_Save_Render_Device(const char * /*sub_key*/) { return false; }
bool DX12Backend::Registry_Load_Render_Device(const char * /*sub_key*/, bool) { return false; }
bool DX12Backend::Registry_Save_Render_Device(const char *, int, int, int, int, bool, int) { return false; }
bool DX12Backend::Registry_Load_Render_Device(const char *, char *, int, int&, int&, int&, int&, int&) { return false; }

void DX12Backend::Begin_Scene() {}
void DX12Backend::End_Scene(bool /*flip_frame*/) {}
void DX12Backend::Flip_To_Primary() {}

void DX12Backend::Clear(bool, bool, const Vector3 &, float, unsigned int) {}
void DX12Backend::Set_Swap_Interval(int) {}
int DX12Backend::Get_Swap_Interval() { return 0; }
void DX12Backend::Set_Texture_Bitdepth(int) {}
int DX12Backend::Get_Texture_Bitdepth() { return 0; }