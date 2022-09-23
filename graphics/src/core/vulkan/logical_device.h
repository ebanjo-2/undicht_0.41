#ifndef LOGICAL_DEVICE_H
#define LOGICAL_DEVICE_H

#include "vector"
#include "vulkan/vulkan.h"
#include "command_buffer.h"

namespace undicht {

    namespace vulkan {

        class LogicalDevice {

        protected:

            VkPhysicalDevice _physical_device;
            VkDevice _device;

            // queue family ids
            int _graphics_queue_id = -1;
            int _present_queue_id = -1;
            int _transfer_queue_id = -1;

            // queues
            VkQueue _graphics_queue;
            VkQueue _present_queue;
            VkQueue _transfer_queue;

            // command pools
            VkCommandPool _graphics_cmds;
            VkCommandPool _transfer_cmds;

        public:

            void init(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface);

            void cleanUp();

            const VkDevice& getDevice() const;
            const VkPhysicalDevice& getPhysicalDevice() const;

            uint32_t getGraphicsQueueFamily() const;
            uint32_t getPresentQueueFamily() const;
            uint32_t getTransferQueueFamily() const;

            const VkCommandPool& getGraphicsCmdPool() const;
            const VkCommandPool& getTransferCmdPool() const;

            void submitOnGraphicsQueue(const VkCommandBuffer& cmd, VkFence signal_fen = VK_NULL_HANDLE, const std::vector<VkSemaphore>& wait_on = {}, const std::vector<VkPipelineStageFlags>& wait_stages = {}, const std::vector<VkSemaphore>& signal_sem = {});
            void submitOnTransferQueue(const VkCommandBuffer& cmd);

            void presentOnPresentQueue(const VkSwapchainKHR& swap_chain, const uint32_t& image_index, const std::vector<VkSemaphore>& wait_on = {});

            void waitForProcessesToFinish() const;

        protected:
            // finding the correct queue families

            // getting a description of the queue families present on the device
            std::vector<VkQueueFamilyProperties> static getQueueFamilyProperties(const VkPhysicalDevice& physical_device);

            // will return -1 if no such queue was found
            int static findGraphicsQueue(const std::vector<VkQueueFamilyProperties>& queue_families);
            int static findPresentQueue(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface, const std::vector<VkQueueFamilyProperties>& queue_families);
            int static findTransferQueue(const std::vector<VkQueueFamilyProperties>& queue_families);

        protected:
            // initializing the logical device

            void initLogicalDevice();

            void initCmdPools();

        protected:
            // creating logical device related structs

            VkDeviceCreateInfo static createDeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queue_infos, const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& features);
            VkDeviceQueueCreateInfo static createQueueCreateInfo(unsigned queue_family, float& priority);
            VkCommandPoolCreateInfo static createCommandPoolCreateInfo(unsigned queue_family);
            VkSubmitInfo static createSubmitInfo(const VkCommandBuffer& cmd, const std::vector<VkSemaphore>& wait_on, const std::vector<VkPipelineStageFlags>& wait_stages, const std::vector<VkSemaphore>& signal);
            VkPresentInfoKHR static createPresentInfo(const VkSwapchainKHR& swap_chain, const uint32_t& image_index, const std::vector<VkSemaphore>& wait_on);

        };

    } // vulkan

} // undicht

#endif // LOGICAL_DEVICE_H