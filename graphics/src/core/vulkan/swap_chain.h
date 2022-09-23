#ifndef SWAP_CHAIN_H
#define SWAP_CHAIN_H

#include "vector"
#include "vulkan/vulkan.h"
#include "core/vulkan/logical_device.h"
#include "core/vulkan/image.h"
#include "core/vulkan/semaphore.h"

namespace undicht {

    namespace vulkan {

        class SwapChain {

        protected:

            VkDevice _device_handle;
            VkPhysicalDevice _physical_device_handle;
            VkSurfaceKHR _surface_handle;
            std::vector<uint32_t> _queue_families;

            VkSwapchainKHR _swap_chain = VK_NULL_HANDLE;
            VkSurfaceFormatKHR _surface_format;
            VkPresentModeKHR _present_mode;
            uint32_t _swap_image_count;
            VkExtent2D _extent;

            std::vector<Image> _swap_images;

        public:

            void init(const LogicalDevice& device, const VkSurfaceKHR& surface, VkPresentModeKHR preferred = VK_PRESENT_MODE_IMMEDIATE_KHR);
            void cleanUp();

            // if the surfaces properties have changed (most likely its size) the swap chain needs to be recreated
            void recreate(const VkSurfaceKHR& surface);

            const VkSwapchainKHR& getSwapchain() const;
            const VkFormat& getSwapImageFormat() const;
            const VkExtent2D& getExtent() const;
            const VkImageView& getSwapImageView(int id) const;
            int getSwapImageCount() const;

            // request image from the swapchain
            // the semaphore and fence are signaled once the image is acquired
            uint32_t acquireNextSwapImage(VkSemaphore signal_sem = VK_NULL_HANDLE, VkFence signal_fen = VK_NULL_HANDLE);

        protected:
            // specifying features of the swap chain

            VkSurfaceFormatKHR static chooseSurfaceFormat(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface);
            VkPresentModeKHR static choosePresentMode(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface, VkPresentModeKHR preferred = VK_PRESENT_MODE_IMMEDIATE_KHR);
            uint32_t static chooseSwapImageCount(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface);

        protected:
            //  creating swap chain related structs

            VkSwapchainCreateInfoKHR static createSwapchainCreateInfo(const VkSurfaceKHR& surface, uint32_t image_count, const VkSurfaceFormatKHR& format, const VkPresentModeKHR& present_mode, const VkExtent2D& extent, const std::vector<uint32_t>& queue_families, const VkSurfaceTransformFlagBitsKHR& transform);

        };

    } // vulkan

} // undicht

#endif // SWAP_CHAIN_H