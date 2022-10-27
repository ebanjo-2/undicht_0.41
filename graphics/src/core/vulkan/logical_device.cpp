#include "logical_device.h"
#include "debug.h"
#include "set"

namespace undicht {

    namespace vulkan {

        void LogicalDevice::init(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface) {

            // storing the physical device
            _physical_device = physical_device;

            // find the correct queue families
            std::vector<VkQueueFamilyProperties> queue_families = getQueueFamilyProperties(physical_device);
            _graphics_queue_id = findGraphicsQueue(queue_families);
            _present_queue_id = findPresentQueue(physical_device, surface, queue_families);
            _transfer_queue_id = findTransferQueue(queue_families);

            // initializing the logical device
            initLogicalDevice();

            // initializing the command pools
            initCmdPools();

        }

        void LogicalDevice::cleanUp() {

            // destroying the command pools
            vkDestroyCommandPool(_device, _graphics_cmds, {});
            vkDestroyCommandPool(_device, _transfer_cmds, {});

            // destroying the logical device
            vkDestroyDevice(_device, {});

        }

        uint32_t LogicalDevice::getMemoryTypeIndex(VkMemoryType mem_type) const {
            
            // getting the physical devices memory properties
            VkPhysicalDeviceMemoryProperties properties;
            vkGetPhysicalDeviceMemoryProperties(_physical_device, &properties);

            // searching for the right memory
            for(int i = 0; i < properties.memoryTypeCount; i++) {

                if(!(mem_type.heapIndex & (1 << i)))
                    continue; // heapIndex is used as a bitfield which specifies the types that can be used

                if((properties.memoryTypes[i].propertyFlags & mem_type.propertyFlags) == mem_type.propertyFlags)
                    return i;

            }

            UND_ERROR << "failed to find the right type of vram\n";
            return 0;
        }


        const VkDevice& LogicalDevice::getDevice() const {

            return _device;
        }

        const VkPhysicalDevice& LogicalDevice::getPhysicalDevice() const {

            return _physical_device;
        }

        uint32_t LogicalDevice::getGraphicsQueueFamily() const {

            return _graphics_queue_id;
        }

        uint32_t LogicalDevice::getPresentQueueFamily() const {

            return _present_queue_id;
        }

        uint32_t LogicalDevice::getTransferQueueFamily() const {

            return _transfer_queue_id;
        }

        const VkQueue& LogicalDevice::getGraphicsQueue() const {
            
            return _graphics_queue;
        }

        const VkQueue& LogicalDevice::getPresentQueue() const {

            return _present_queue;
        }

        const VkQueue& LogicalDevice::getTransferQueue() const {

            return _transfer_queue;
        }

        const VkCommandPool& LogicalDevice::getGraphicsCmdPool() const {

            return _graphics_cmds;
        }

        const VkCommandPool& LogicalDevice::getTransferCmdPool() const {

            return _transfer_cmds;
        }

        void LogicalDevice::submitOnGraphicsQueue(const VkCommandBuffer& cmd, VkFence signal_fen, const std::vector<VkSemaphore>& wait_on, const std::vector<VkPipelineStageFlags>& wait_stages, const std::vector<VkSemaphore>& signal_sem) const {

            VkSubmitInfo info = createSubmitInfo(cmd, wait_on, wait_stages, signal_sem);
            vkQueueSubmit(_graphics_queue, 1, &info, signal_fen);

        }

        void LogicalDevice::submitOnTransferQueue(const VkCommandBuffer& cmd, VkFence signal_fen, const std::vector<VkSemaphore>& wait_on, const std::vector<VkPipelineStageFlags>& wait_stages, const std::vector<VkSemaphore>& signal_sem) const {

            VkSubmitInfo info = createSubmitInfo(cmd, wait_on, wait_stages, signal_sem);
            vkQueueSubmit(_transfer_queue, 1, &info, signal_fen);

        }

        void LogicalDevice::presentOnPresentQueue(const VkSwapchainKHR& swap_chain, const uint32_t& image_index, const std::vector<VkSemaphore>& wait_on) {

            VkPresentInfoKHR info = createPresentInfo(swap_chain, image_index, wait_on);
            vkQueuePresentKHR(_present_queue, &info);

        }

        void LogicalDevice::waitGraphicsQueueIdle() const {
            
            vkQueueWaitIdle(_graphics_queue);
        }

        void LogicalDevice::waitTransferQueueIdle() const {
            
            vkQueueWaitIdle(_transfer_queue);
        }

        void LogicalDevice::waitForProcessesToFinish() const {

            vkDeviceWaitIdle(_device);
        }

        ////////////////////////////// finding the correct queue families //////////////////////////////

        std::vector<VkQueueFamilyProperties> LogicalDevice::getQueueFamilyProperties(const VkPhysicalDevice& physical_device) {
            // getting a description of the queue families present on the device

            unsigned queue_family_count;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
            std::vector<VkQueueFamilyProperties> properties(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, properties.data());

            return properties;
        }

        int LogicalDevice::findGraphicsQueue(const std::vector<VkQueueFamilyProperties>& queue_families) {

            for(int i = 0; i < queue_families.size(); i++)
                if(queue_families.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    return i;

            return -1;
        }

        int LogicalDevice::findPresentQueue(const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface, const std::vector<VkQueueFamilyProperties>& queue_families) {

            for(int i = 0; i < queue_families.size(); i++) {

                VkBool32 surface_support;
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &surface_support);

                if (surface_support)
                    return i;

            }

            return -1;
        }

        int LogicalDevice::findTransferQueue(const std::vector<VkQueueFamilyProperties>& queue_families) {

            for(int i = 0; i < queue_families.size(); i++) {
                // the transfer queue should be different to the graphics queue
                if (queue_families.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    continue;
                if (queue_families.at(i).queueFlags & VK_QUEUE_TRANSFER_BIT)
                    return i;
            }

            return -1;
        }

        //////////////////////////////// initializing the logical device ////////////////////////////////

        void LogicalDevice::initLogicalDevice() {

            // specifying the queues the device is going to use
            float priority = 1.0f;
            std::set<int> queue_families = {_graphics_queue_id, _present_queue_id, _transfer_queue_id};
            std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

            for(int i : queue_families)
                queue_create_infos.push_back(createQueueCreateInfo(i, priority));

            // specifying the extensions the device is going to use
            std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

            // specifying the features the device is going to use
            VkPhysicalDeviceFeatures features{};
            features.samplerAnisotropy = VK_TRUE;
            features.fillModeNonSolid = VK_TRUE;

            // creating the logical device
            VkDeviceCreateInfo info = createDeviceCreateInfo(queue_create_infos, extensions, features);
            vkCreateDevice(_physical_device, &info, {}, &_device);

            // creating the queue handles
            vkGetDeviceQueue(_device, _graphics_queue_id, 0, &_graphics_queue);
            vkGetDeviceQueue(_device, _present_queue_id, 0, &_present_queue);
            vkGetDeviceQueue(_device, _transfer_queue_id, 0, &_transfer_queue);

        }

        void LogicalDevice::initCmdPools() {

            // commands for the graphics queue
            VkCommandPoolCreateInfo graphics_cmd_info = createCommandPoolCreateInfo(_graphics_queue_id);
            vkCreateCommandPool(_device, &graphics_cmd_info, {}, &_graphics_cmds);

            // commands for the transfer queue
            VkCommandPoolCreateInfo transfer_cmd_info = createCommandPoolCreateInfo(_transfer_queue_id);
            vkCreateCommandPool(_device, &transfer_cmd_info, {}, &_transfer_cmds);
        }

        ////////////////////////////////  creating logical device related structs ////////////////////////////////

        VkDeviceCreateInfo LogicalDevice::createDeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queue_infos, const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& features) {

            VkDeviceCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            info.queueCreateInfoCount = queue_infos.size();
            info.pQueueCreateInfos = queue_infos.data();
            info.enabledLayerCount = 0;
            info.ppEnabledLayerNames = {};
            info.enabledExtensionCount = extensions.size();
            info.ppEnabledExtensionNames = extensions.data();
            info.pEnabledFeatures = &features;

            return info;
        }

        VkDeviceQueueCreateInfo LogicalDevice::createQueueCreateInfo(unsigned queue_family, float& priority) {

            VkDeviceQueueCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            info.queueFamilyIndex = queue_family;
            info.queueCount = 1;
            info.pQueuePriorities = &priority;

            return info;
        }

        VkCommandPoolCreateInfo LogicalDevice::createCommandPoolCreateInfo(unsigned queue_family) {

            VkCommandPoolCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // the command buffers can be reset individually
            info.queueFamilyIndex = queue_family;

            return info;
        }

        VkSubmitInfo LogicalDevice::createSubmitInfo(const VkCommandBuffer& cmd, const std::vector<VkSemaphore>& wait_on, const std::vector<VkPipelineStageFlags>& wait_stages, const std::vector<VkSemaphore>& signal) {

            VkSubmitInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.pNext = nullptr;
            info.pWaitDstStageMask = wait_stages.data();
            info.waitSemaphoreCount = wait_on.size();
            info.pWaitSemaphores = wait_on.data();
            info.signalSemaphoreCount = signal.size();
            info.pSignalSemaphores = signal.data();
            info.commandBufferCount = 1;
            info.pCommandBuffers = &cmd;

            return info;
        }

        VkPresentInfoKHR LogicalDevice::createPresentInfo(const VkSwapchainKHR& swap_chain, const uint32_t& image_index, const std::vector<VkSemaphore>& wait_on) {

            VkPresentInfoKHR info{};
            info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            info.pNext = nullptr;
            info.pSwapchains = &swap_chain;
            info.swapchainCount = 1;
            info.pWaitSemaphores = wait_on.data();
            info.waitSemaphoreCount = wait_on.size();
            info.pImageIndices = &image_index;

            return info;
        }


    }

} // undicht