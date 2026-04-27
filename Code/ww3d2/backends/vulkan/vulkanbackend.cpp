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
 * VulkanBackend implementation.                                                               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "vulkanbackend.h"

namespace
{
    const int DEFAULT_WIDTH = 640;
    const int DEFAULT_HEIGHT = 480;
    const int DEFAULT_BIT_DEPTH = 32;
    const int DEFAULT_TEXTURE_DEPTH = 32;

    // DX8 render state values (matches d3d8types.h values)
    const int D3DRS_FILLMODE = 8;
    const int D3DRS_BLENDENABLE = 19;
    const int D3DFILL_SOLID = 3;
    const int D3DFILL_WIREFRAME = 2;
    const int D3DFILL_POINT = 1;
}

VulkanBackend::VulkanBackend() :
    m_instance(VK_NULL_HANDLE),
    m_physical_device(VK_NULL_HANDLE),
    m_device(VK_NULL_HANDLE),
    m_surface(VK_NULL_HANDLE),
    m_swapchain(VK_NULL_HANDLE),
    m_graphics_queue_family(UINT32_MAX),
    m_graphics_queue(VK_NULL_HANDLE),
    m_command_pool(VK_NULL_HANDLE),
    m_initialized(false),
    m_vsync(1),
    m_width(DEFAULT_WIDTH),
    m_height(DEFAULT_HEIGHT),
    m_bit_depth(DEFAULT_BIT_DEPTH),
    m_windowed(true),
    m_texture_bit_depth(DEFAULT_TEXTURE_DEPTH),
    m_current_device(0),
    m_current_command_buffer(VK_NULL_HANDLE)
{
}

VulkanBackend::~VulkanBackend()
{
    if (m_initialized) {
        Shutdown();
    }
}

bool VulkanBackend::Create_Vulkan_Instance()
{
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "OpenW3D";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "OpenW3D Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = nullptr;
    create_info.enabledExtensionCount = 0;
    create_info.ppEnabledExtensionNames = nullptr;

    VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool VulkanBackend::Enumerate_Devices()
{
    uint32_t device_count = 0;
    VkResult result = vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
    if (result != VK_SUCCESS || device_count == 0) {
        return false;
    }

    m_devices.resize(device_count);
    result = vkEnumeratePhysicalDevices(m_instance, &device_count, m_devices.data());
    if (result != VK_SUCCESS) {
        return false;
    }

    // Build device descriptions
    m_device_descs.clear();
    for (size_t i = 0; i < m_devices.size(); ++i) {
        RenderDeviceDescClass desc;
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(m_devices[i], &props);
        // Use Get_Device_Name() public method if available, otherwise just store the name
        // Since set_device_name is private, we rely on Get_Render_Device_Name() to return it from m_devices
        desc = RenderDeviceDescClass(); // Reset to default
        m_device_descs.push_back(desc);
    }

    return true;
}

bool VulkanBackend::Select_Device(int device_index)
{
    if (device_index < 0 || static_cast<size_t>(device_index) >= m_devices.size()) {
        // Default to first discrete GPU if available, otherwise first device
        for (size_t i = 0; i < m_devices.size(); ++i) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(m_devices[i], &props);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                m_physical_device = m_devices[i];
                m_current_device = static_cast<int>(i);
                return true;
            }
        }
        m_physical_device = m_devices[0];
        m_current_device = 0;
        return true;
    }

    m_physical_device = m_devices[device_index];
    m_current_device = device_index;
    return true;
}

uint32_t VulkanBackend::Find_Graphics_Queue_Family()
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < queue_family_count; ++i) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            return i;
        }
    }

    return UINT32_MAX;
}

bool VulkanBackend::Create_Vulkan_Device()
{
    m_graphics_queue_family = Find_Graphics_Queue_Family();
    if (m_graphics_queue_family == UINT32_MAX) {
        return false;
    }

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = m_graphics_queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    const char* device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.enabledExtensionCount = 1;
    device_create_info.ppEnabledExtensionNames = device_extensions;
    device_create_info.pEnabledFeatures = nullptr;

    VkResult result = vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device);
    if (result != VK_SUCCESS) {
        return false;
    }

    vkGetDeviceQueue(m_device, m_graphics_queue_family, 0, &m_graphics_queue);

    // Create command pool
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = m_graphics_queue_family;

    result = vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool);
    if (result != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool VulkanBackend::Create_Surface(void * hwnd)
{
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hwnd = static_cast<HWND>(hwnd);
    surface_create_info.hinstance = GetModuleHandle(nullptr);

    VkResult result = vkCreateWin32SurfaceKHR(m_instance, &surface_create_info, nullptr, &m_surface);
    if (result != VK_SUCCESS) {
        return false;
    }
#else
    // On non-Windows, we can't create a surface without a platform-specific call
    // For now, mark surface as invalid
    (void)hwnd;
    m_surface = VK_NULL_HANDLE;
#endif
    return true;
}

bool VulkanBackend::Create_Swapchain(int width, int height)
{
    if (m_surface == VK_NULL_HANDLE) {
        // Can't create swapchain without surface
        return false;
    }

    // Get surface capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface, &capabilities);

    // Get surface formats
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    if (format_count > 0) {
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &format_count, formats.data());
    }

    // Get present modes
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    if (present_mode_count > 0) {
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &present_mode_count, present_modes.data());
    }

    VkSurfaceFormatKHR surface_format = formats[0];
    if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
        surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }

    VkPresentModeKHR present_mode = m_vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;

    VkExtent2D extent;
    if (capabilities.currentExtent.width == UINT32_MAX) {
        extent.width = static_cast<uint32_t>(width);
        extent.height = static_cast<uint32_t>(height);
    } else {
        extent = capabilities.currentExtent;
    }

    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_info = {};
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.surface = m_surface;
    swapchain_info.minImageCount = image_count;
    swapchain_info.imageFormat = surface_format.format;
    swapchain_info.imageColorSpace = surface_format.colorSpace;
    swapchain_info.imageExtent = extent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info.preTransform = capabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode = present_mode;
    swapchain_info.clipped = VK_TRUE;
    swapchain_info.oldSwapchain = m_swapchain;

    VkResult result = vkCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &m_swapchain);
    if (result != VK_SUCCESS) {
        return false;
    }

    m_width = width;
    m_height = height;

    return true;
}

void VulkanBackend::Destroy_Swapchain()
{
    if (m_device != VK_NULL_HANDLE && m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
}

void VulkanBackend::Destroy_Surface()
{
    if (m_instance != VK_NULL_HANDLE && m_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
}

void VulkanBackend::Destroy_Device()
{
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);

        if (m_command_pool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(m_device, m_command_pool, nullptr);
            m_command_pool = VK_NULL_HANDLE;
        }

        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
}

void VulkanBackend::Destroy_Instance()
{
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

bool VulkanBackend::Init(void * hwnd, bool /*lite*/)
{
    if (m_initialized) {
        return true;
    }

    if (!Create_Vulkan_Instance()) {
        return false;
    }

    if (!Enumerate_Devices()) {
        Destroy_Instance();
        return false;
    }

    if (!Select_Device(0)) {
        Destroy_Instance();
        return false;
    }

    if (!Create_Vulkan_Device()) {
        Destroy_Instance();
        return false;
    }

    if (!Create_Surface(hwnd)) {
        Destroy_Device();
        Destroy_Instance();
        return false;
    }

    if (!Create_Swapchain(m_width, m_height)) {
        Destroy_Surface();
        Destroy_Device();
        Destroy_Instance();
        return false;
    }

    m_initialized = true;
    return true;
}

void VulkanBackend::Shutdown()
{
    if (!m_initialized) {
        return;
    }

    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
    }

    Destroy_Swapchain();
    Destroy_Surface();
    Destroy_Device();
    Destroy_Instance();

    m_devices.clear();
    m_device_descs.clear();
    m_render_state.clear();

    m_initialized = false;
}

bool VulkanBackend::Set_Any_Render_Device()
{
    return Set_Render_Device(0, m_width, m_height, m_bit_depth, m_windowed ? 1 : 0);
}

bool VulkanBackend::Set_Render_Device(const char * /*dev_name*/, int width, int height, int bits, int windowed, bool resize_window)
{
    return Set_Device_Resolution(width, height, bits, windowed, resize_window);
}

bool VulkanBackend::Set_Render_Device(int dev, int resx, int resy, int bits, int windowed, bool resize_window)
{
    if (dev >= 0 && dev < static_cast<int>(m_devices.size())) {
        Select_Device(dev);
    }
    return Set_Device_Resolution(resx, resy, bits, windowed, resize_window);
}

bool VulkanBackend::Set_Next_Render_Device()
{
    int next_dev = (m_current_device + 1) % m_devices.size();
    return Set_Render_Device(next_dev, m_width, m_height, m_bit_depth, m_windowed ? 1 : 0);
}

bool VulkanBackend::Toggle_Windowed()
{
    m_windowed = !m_windowed;
    return true;
}

bool VulkanBackend::Is_Windowed()
{
    return m_windowed;
}

int VulkanBackend::Get_Render_Device_Count()
{
    return static_cast<int>(m_devices.size());
}

int VulkanBackend::Get_Render_Device()
{
    return m_current_device;
}

const RenderDeviceDescClass & VulkanBackend::Get_Render_Device_Desc(int deviceidx)
{
    static RenderDeviceDescClass empty_desc;
    if (deviceidx >= 0 && deviceidx < static_cast<int>(m_device_descs.size())) {
        return m_device_descs[deviceidx];
    }
    return empty_desc;
}

const char * VulkanBackend::Get_Render_Device_Name(int device_index)
{
    if (device_index < 0 || device_index >= static_cast<int>(m_devices.size())) {
        return "Unknown Vulkan Device";
    }

    static char device_name[256];
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_devices[device_index], &props);
    strncpy(device_name, props.deviceName, sizeof(device_name) - 1);
    device_name[sizeof(device_name) - 1] = '\0';
    return device_name;
}

bool VulkanBackend::Set_Device_Resolution(int width, int height, int bits, int windowed, bool /*resize_window*/)
{
    if (width > 0) {
        m_width = width;
    }
    if (height > 0) {
        m_height = height;
    }
    if (bits > 0) {
        m_bit_depth = bits;
    }
    if (windowed != -1) {
        m_windowed = (windowed != 0);
    }

    if (m_initialized) {
        Destroy_Swapchain();
        if (!Create_Swapchain(m_width, m_height)) {
            return false;
        }
    }

    return true;
}

void VulkanBackend::Get_Device_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
    set_w = m_width;
    set_h = m_height;
    set_bits = m_bit_depth;
    set_windowed = m_windowed;
}

void VulkanBackend::Get_Render_Target_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
    Get_Device_Resolution(set_w, set_h, set_bits, set_windowed);
}

int VulkanBackend::Get_Device_Resolution_Width()
{
    return m_width;
}

int VulkanBackend::Get_Device_Resolution_Height()
{
    return m_height;
}

bool VulkanBackend::Registry_Save_Render_Device(const char * /*sub_key*/)
{
    return true;
}

bool VulkanBackend::Registry_Load_Render_Device(const char * /*sub_key*/, bool /*resize_window*/)
{
    return true;
}

bool VulkanBackend::Registry_Save_Render_Device(const char * /*sub_key*/, int device, int width, int height, int depth, bool windowed, int texture_depth)
{
    m_current_device = device;
    m_width = width;
    m_height = height;
    m_bit_depth = depth;
    m_windowed = windowed;
    m_texture_bit_depth = texture_depth;
    return true;
}

bool VulkanBackend::Registry_Load_Render_Device(const char * /*sub_key*/, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth)
{
    if (device && device_len > 0) {
        device[0] = '\0';
    }
    width = m_width;
    height = m_height;
    depth = m_bit_depth;
    windowed = m_windowed ? 1 : 0;
    texture_depth = m_texture_bit_depth;
    return true;
}

void VulkanBackend::Begin_Scene()
{
    if (m_device == VK_NULL_HANDLE || m_command_pool == VK_NULL_HANDLE) {
        return;
    }

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = m_command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(m_device, &alloc_info, &m_current_command_buffer);
    if (result != VK_SUCCESS) {
        return;
    }

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(m_current_command_buffer, &begin_info);
}

void VulkanBackend::End_Scene(bool /*flip_frame*/)
{
    if (m_current_command_buffer == VK_NULL_HANDLE) {
        return;
    }

    vkEndCommandBuffer(m_current_command_buffer);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_current_command_buffer;

    vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);

    // Present
    uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, VK_NULL_HANDLE, VK_NULL_HANDLE, &image_index);

    if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &m_swapchain;
        present_info.pImageIndices = &image_index;

        vkQueuePresentKHR(m_graphics_queue, &present_info);
    }

    m_current_command_buffer = VK_NULL_HANDLE;
}

void VulkanBackend::Flip_To_Primary()
{
    // Vulkan always renders to the swapchain which presents to the primary surface
}

void VulkanBackend::Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z, unsigned int /*stencil*/)
{
    if (m_current_command_buffer == VK_NULL_HANDLE) {
        return;
    }

    // Note: In a real implementation, we would need render pass support
    // For now, this is a placeholder - real clear would happen in Begin_Scene with render pass
}

void VulkanBackend::Set_Swap_Interval(int swap)
{
    m_vsync = (swap != 0) ? 1 : 0;

    // Recreate swapchain with new vsync setting if initialized
    if (m_initialized && m_swapchain != VK_NULL_HANDLE) {
        Destroy_Swapchain();
        Create_Swapchain(m_width, m_height);
    }
}

int VulkanBackend::Get_Swap_Interval()
{
    return m_vsync;
}

void VulkanBackend::Set_Texture_Bitdepth(int depth)
{
    m_texture_bit_depth = depth;
}

int VulkanBackend::Get_Texture_Bitdepth()
{
    return m_texture_bit_depth;
}

void VulkanBackend::Set_Viewport(const void* viewport)
{
    // Cast to D3DVIEWPORT9* or generic const void* and extract dimensions
    // For now, store the viewport dimensions for later use in render passes
    if (viewport) {
        // D3DVIEWPORT9 is assumed - extract width/height
        // This would be used when creating VkViewport
    }
}

void VulkanBackend::Set_DX8_Render_State(int state, unsigned value)
{
    m_render_state[state] = value;

    // Map common DX8 render states to Vulkan equivalents
    switch (state) {
        case D3DRS_FILLMODE: {
            // D3DFILL_SOLID -> VK_POLYGON_MODE_FILL
            // D3DFILL_WIREFRAME -> VK_POLYGON_MODE_LINE
            // D3DFILL_POINT -> VK_POLYGON_MODE_POINT
            // Would be used when creating graphics pipelines
            break;
        }
        case D3DRS_BLENDENABLE: {
            // Enable/disable color blending via VkPipelineColorBlendAttachmentState
            // Would be used when creating graphics pipelines
            break;
        }
        default:
            break;
    }
}

void VulkanBackend::Set_Light_Environment(const void* /*env*/)
{
    // No-op: light state is handled elsewhere in mesh rendering pipeline
}

void* VulkanBackend::_Get_DX8_Front_Buffer()
{
    // Not applicable for Vulkan - no direct front buffer access
    return nullptr;
}
