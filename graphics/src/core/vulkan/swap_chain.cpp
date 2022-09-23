#include "swap_chain.h"
#include "vector"
#include "algorithm"
#include "debug.h"
#include <set>

namespace undicht {

    namespace vulkan {

        void SwapChain::init(const LogicalDevice& device, const VkSurfaceKHR& surface, VkPresentModeKHR preferred) {

            _device_handle = device.getDevice();
            _physical_device_handle = device.getPhysicalDevice();
            _surface_handle = surface;

            // getting the unique queue families that are going to access the swap chain
            std::set<uint32_t> unique_queue_families = {device.getGraphicsQueueFamily(), device.getPresentQueueFamily()};
            _queue_families.insert(_queue_families.begin(), unique_queue_families.begin(), unique_queue_families.end());

            // choosing the features of the swap chain
            _surface_format = chooseSurfaceFormat(_physical_device_handle, surface);
            _present_mode = choosePresentMode(_physical_device_handle, surface);
            _swap_image_count = chooseSwapImageCount(_physical_device_handle, surface);

            // "recreating" the swap chain
            recreate(surface);
        }

        void SwapChain::cleanUp() {
            // waiting for processes that are using the swap chain to finish
            vkDeviceWaitIdle(_device_handle);

            // destroying the swap images
            for(Image& i : _swap_images)
                i.cleanUp();

            // destroying the swap chain
            vkDestroySwapchainKHR(_device_handle, _swap_chain, {});
        }

        void SwapChain::recreate(const VkSurfaceKHR &surface) {

            // destroying the old swap chain
            if(_swap_chain != VK_NULL_HANDLE) cleanUp();

            // getting the swap extent
            VkSurfaceCapabilitiesKHR capabilities;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physical_device_handle, surface, &capabilities);
            _extent = capabilities.currentExtent;

            // creating the swap chain
            VkSwapchainCreateInfoKHR info = createSwapchainCreateInfo(surface, _swap_image_count,_surface_format, _present_mode, _extent, _queue_families, capabilities.currentTransform);
            vkCreateSwapchainKHR(_device_handle, &info, {}, &_swap_chain);

            // retrieving the swap images
            unsigned swap_image_count;
            vkGetSwapchainImagesKHR(_device_handle, _swap_chain, &swap_image_count, nullptr);
            std::vector<VkImage> swap_images(swap_image_count);
            vkGetSwapchainImagesKHR(_device_handle, _swap_chain, &swap_image_count, swap_images.data());

            // storing the swap images
            _swap_images.resize(swap_image_count);
            for(int i = 0; i < swap_images.size(); i++)
                _swap_images[i].initWithExternalImage(_device_handle, swap_images[i], _surface_format.format);

        }

        const VkSwapchainKHR& SwapChain::getSwapchain() const {

            return _swap_chain;
        }

        const VkFormat& SwapChain::getSwapImageFormat() const {

            return _surface_format.format;
        }

        const VkExtent2D& SwapChain::getExtent() const {

            return _extent;
        }

        const VkImageView& SwapChain::getSwapImageView(int id) const {

            return _swap_images.at(id).getImageView();
        }

        int SwapChain::getSwapImageCount() const {

            return _swap_image_count;
        }

        uint32_t SwapChain::acquireNextSwapImage(VkSemaphore signal_sem, VkFence signal_fen) {

            uint32_t next_image;
            vkAcquireNextImageKHR(_device_handle, _swap_chain, UINT32_MAX, signal_sem, signal_fen, &next_image);

            return next_image;
        }

        ////////////////////////////////// specifying features of the swap chain //////////////////////////////////

        VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface) {

            // getting the formats supported by the physical device and the surface
            unsigned format_count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
            std::vector<VkSurfaceFormatKHR> available_formats(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, available_formats.data());

            // looking for the preferred format
            VkSurfaceFormatKHR BGRA_srgb {VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR};

            for(VkSurfaceFormatKHR& format : available_formats)
                if(format.format == BGRA_srgb.format && format.colorSpace == BGRA_srgb.colorSpace)
                    return BGRA_srgb;

            // returning an undefined alternative format (may result in a crash or weird resulting images)
            UND_WARNING << "preferred swap chain format VK_FORMAT_B8G8R8A8_SRGB is not supported\n";
            if(!available_formats.size())
                UND_ERROR << "No formats are supported\n";
            // if there are no available formats the application is doomed anyway
            return available_formats.front();
        }

        VkPresentModeKHR SwapChain::choosePresentMode(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface, VkPresentModeKHR preferred) {

            // getting the present modes supported by the physical device and the surface
            unsigned present_mode_count;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
            std::vector<VkPresentModeKHR> present_modes(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());

            // looking for the preferred present mode

            for(VkPresentModeKHR& mode : present_modes)
                if(mode == preferred)
                    return preferred;

            // returning an alternative present mode (vsync mode is guaranteed to exist)
            UND_WARNING << "Using alternative present mode : vsync\n";
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        uint32_t SwapChain::chooseSwapImageCount(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface) {

            // getting the surface capabilities
            VkSurfaceCapabilitiesKHR capabilities;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

            // choosing a swap image count
            uint32_t count = 2;
            count = std::max(count, capabilities.minImageCount);

            if(capabilities.maxImageCount)
                count = std::min(count, capabilities.maxImageCount);

            return count;
        }

        //////////////////////////////////  creating swap chain related structs //////////////////////////////////

        VkSwapchainCreateInfoKHR SwapChain::createSwapchainCreateInfo(const VkSurfaceKHR& surface,uint32_t image_count, const VkSurfaceFormatKHR& format, const VkPresentModeKHR& present_mode, const VkExtent2D& extent, const std::vector<uint32_t>& queue_families, const VkSurfaceTransformFlagBitsKHR& transform) {

            VkSwapchainCreateInfoKHR info{};
            info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            info.surface = surface;
            info.minImageCount = image_count;
            info.imageFormat = format.format;
            info.imageColorSpace = format.colorSpace;
            info.imageExtent = extent;
            info.imageArrayLayers = 1;
            info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            if(queue_families.size() == 1)
                info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            else
                info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;

            info.pQueueFamilyIndices = queue_families.data();
            info.queueFamilyIndexCount = queue_families.size();
            info.preTransform = transform;
            info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            info.presentMode = present_mode;
            info.clipped = VK_TRUE;

            return info;
        }

    } // vulkan

} // undicht