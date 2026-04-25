/**
 * VulkanBackend — stub Vulkan render backend implementation.
 *
 * STATUS: Not implemented. All methods return false or no-op.
 *
 * This file exists to:
 * - Establish the Vulkan backend's concrete shape matching VulkanBackend class
 * - Provide a compile-valid stub so the CMake plumbing can reference it
 * - Serve as the implementation entry point when real Vulkan work begins
 *
 * Build enable: cmake -DENABLE_VULKAN_BACKEND=ON
 */

#include "backends/vulkan/vulkanbackend.h"

VulkanBackend::VulkanBackend() = default;
VulkanBackend::~VulkanBackend() = default;

bool VulkanBackend::Init(void * /*hwnd*/, bool /*lite*/)
{
    // TODO: create VkInstance with required extensions (VK_KHR_surface, etc.)
    // TODO: enumerate physical devices and select GPU
    // TODO: create VkDevice with queues and extensions
    // TODO: create window surface (VkSurfaceKHR from hwnd)
    // TODO: create swapchain
    m_initialized = false;
    return false;
}

void VulkanBackend::Shutdown()
{
    // TODO: wait for device idle
    // TODO: destroy swapchain, surface, device, instance
    m_initialized = false;
}

bool VulkanBackend::Set_Any_Render_Device() { return false; }
bool VulkanBackend::Set_Render_Device(const char * /*dev_name*/, int, int, int, int, bool) { return false; }
bool VulkanBackend::Set_Render_Device(int, int, int, int, int, bool) { return false; }
bool VulkanBackend::Set_Next_Render_Device() { return false; }
bool VulkanBackend::Toggle_Windowed() { return false; }
bool VulkanBackend::Is_Windowed() { return false; }

int VulkanBackend::Get_Render_Device_Count() { return 0; }
int VulkanBackend::Get_Render_Device() { return -1; }

const RenderDeviceDescClass & VulkanBackend::Get_Render_Device_Desc(int /*deviceidx*/)
{
    static RenderDeviceDescClass null_desc;
    return null_desc;
}

const char * VulkanBackend::Get_Render_Device_Name(int /*device_index*/) { return "Vulkan (not implemented)"; }

bool VulkanBackend::Set_Device_Resolution(int, int, int, int, bool) { return false; }
void VulkanBackend::Get_Device_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
    set_w = set_h = set_bits = 0;
    set_windowed = false;
}
void VulkanBackend::Get_Render_Target_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
    set_w = set_h = set_bits = 0;
    set_windowed = false;
}
int VulkanBackend::Get_Device_Resolution_Width() { return 0; }
int VulkanBackend::Get_Device_Resolution_Height() { return 0; }

bool VulkanBackend::Registry_Save_Render_Device(const char * /*sub_key*/) { return false; }
bool VulkanBackend::Registry_Load_Render_Device(const char * /*sub_key*/, bool) { return false; }
bool VulkanBackend::Registry_Save_Render_Device(const char *, int, int, int, int, bool, int) { return false; }
bool VulkanBackend::Registry_Load_Render_Device(const char *, char *, int, int&, int&, int&, int&, int&) { return false; }

void VulkanBackend::Begin_Scene() {}
void VulkanBackend::End_Scene(bool /*flip_frame*/) {}
void VulkanBackend::Flip_To_Primary() {}

void VulkanBackend::Clear(bool, bool, const Vector3 &, float, unsigned int) {}
void VulkanBackend::Set_Swap_Interval(int) {}
int VulkanBackend::Get_Swap_Interval() { return 0; }
void VulkanBackend::Set_Texture_Bitdepth(int) {}
int VulkanBackend::Get_Texture_Bitdepth() { return 0; }