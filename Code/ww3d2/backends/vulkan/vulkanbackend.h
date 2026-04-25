#pragma once

/**
 * VulkanBackend — stub Vulkan render backend for OpenW3D.
 *
 * STATUS: Not implemented. This is a compile-time skeleton that declares
 * the backend interface shape without providing any rendering logic.
 *
 * TODO:
 * - Implement Vulkan instance/device creation and destruction
 * - Implement VK_KHR_surface swapchain management
 * - Implement scene begin/end and frame submission
 * - Wire into WW3DBackend lifecycle
 *
 * Compilation requires:
 * - Vulkan SDK (vulkan-1, vulkan-headers)
 * - Graphics pipeline state objects for our draw calls
 * - Shader compilation pipeline (SPIR-V or GLSL)
 *
 * Build enable: cmake -DENABLE_VULKAN_BACKEND=ON
 */

#include "ww3dbackend.h"

class VulkanBackend : public WW3DBackend
{
public:
    VulkanBackend();
    virtual ~VulkanBackend();

    // Initialization
    // Creates VkInstance and device with requested GPU/extensions.
    virtual bool Init(void * hwnd, bool lite = false) override;
    virtual void Shutdown() override;

    // Device enumeration — enumerates Vulkan-capable GPUs
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
    // Vulkan internals — TODO: implement
    // VkInstance m_instance = VK_NULL_HANDLE;
    // VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    // VkDevice m_device = VK_NULL_HANDLE;
    // VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    // VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    //
    // std::vector<VkPhysicalDevice> m_devices;
    // int m_active_device = 0;
    // int m_vsync = 1;

    bool m_initialized = false;
};