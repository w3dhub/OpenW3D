#pragma once

/**
 * DX12Backend — stub DirectX 12 render backend for OpenW3D.
 *
 * STATUS: Not implemented. This is a compile-time skeleton that declares
 * the backend interface shape without providing any rendering logic.
 *
 * TODO:
 * - Implement D3D12 device creation and command queue setup
 * - Implement swapchain / present queue management
 * - Implement scene begin/end and frame presentation
 * - Wire into WW3DBackend lifecycle
 *
 * Compilation requires:
 * - Windows 10 SDK (d3d12.h, dxgi1_6.h)
 * - D3D12 SDK layers and debug layer support
 *
 * Build enable: cmake -DENABLE_DX12_BACKEND=ON
 */

#include "ww3dbackend.h"

class DX12Backend : public WW3DBackend
{
public:
    DX12Backend();
    virtual ~DX12Backend();

    // Initialization
    // Creates D3D12 device and command queues for the given window.
    virtual bool Init(void * hwnd, bool lite = false) override;
    virtual void Shutdown() override;

    // Device enumeration — enumerates D3D12-capable adapters
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

    // Registry persistence
    virtual bool Registry_Save_Render_Device(const char * sub_key) override;
    virtual bool Registry_Load_Render_Device(const char * sub_key, bool resize_window) override;
    virtual bool Registry_Save_Render_Device(const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth) override;
    virtual bool Registry_Load_Render_Device(const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth) override;

    // Scene
    virtual void Begin_Scene() override;
    virtual void End_Scene(bool flip_frame = true) override;
    virtual void Flip_To_Primary() override;

    // Clear
    virtual void Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z = 1.0f, unsigned int stencil = 0) override;

    // VSync
    virtual void Set_Swap_Interval(int swap) override;
    virtual int Get_Swap_Interval() override;

    // Texture depth
    virtual void Set_Texture_Bitdepth(int depth) override;
    virtual int Get_Texture_Bitdepth() override;

private:
    // D3D12 internals — TODO: implement
    // ID3D12Device * m_device = nullptr;
    // IDXGISwapChain3 * m_swapchain = nullptr;
    // ID3D12CommandQueue * m_command_queue = nullptr;
    // ID3D12GraphicsCommandList * m_command_list = nullptr;
    //
    // std::vector<IDXGIAdapter1 *> m_adapters;
    // int m_active_adapter = 0;
    // int m_vsync = 1;

    bool m_initialized = false;
};