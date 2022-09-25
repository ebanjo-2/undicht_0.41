#ifndef BUFFER_H
#define BUFFER_H

#include "vector"
#include "vulkan/vulkan.h"
#include "logical_device.h"

namespace undicht {

    namespace vulkan {

        class Buffer {

        protected:

            VkDevice _device_handle;

            VkBuffer _buffer;
            VkBufferUsageFlags _usage;
            VkDeviceMemory _memory;
            VkMemoryPropertyFlags _mem_properties;

            uint32_t _allocated_mem_size = 0;

        public:

            void init(const LogicalDevice& device, uint32_t byte_size, const std::vector<uint32_t>& queue_ids, VkBufferUsageFlags usage, bool cpu_visible = true);
            void cleanUp(); // cleanUp() first before init() again

            void setData(const void* data, uint32_t byte_size, uint32_t offset);

            uint32_t getAllocatedMemorySize() const;
            const VkBuffer& getBuffer() const;

        protected:
            // creating buffer related structs

            VkBufferCreateInfo static createBufferCreateInfo(VkBufferUsageFlags usage_flags, uint32_t byte_size, const std::vector<uint32_t>& queue_ids);
            VkMemoryType static createMemoryType(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties);
            VkMemoryAllocateInfo static createMemoryAllocateInfo(uint32_t byte_size, uint32_t mem_type_index);
            VkBufferCopy static createBufferCopy(uint32_t byte_size, uint32_t src_offset, uint32_t dst_offset);

        };

    } // vulkan

} // undicht

#endif // BUFFER_H