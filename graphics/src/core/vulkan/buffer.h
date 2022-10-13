#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include "vulkan/vulkan.h"
#include "logical_device.h"

namespace undicht {

    namespace vulkan {

        class Buffer {
            
        protected:

            VkDevice _device_handle;

            VkBuffer _buffer = VK_NULL_HANDLE;
            VkBufferUsageFlags _usage;

            VkDeviceMemory _memory = VK_NULL_HANDLE;
            VkMemoryPropertyFlags _mem_properties{};
            bool _cpu_visible;
            uint32_t _mem_type_index; // assigned by the device
            uint32_t _allocated_mem_size = 0;

            std::vector<uint32_t> _queue_ids;


        public:

            /** @param queue_ids: the queues from which the buffer is going to be accessed 
             * @param usage: you have to specify in which way the buffer is going to be used:
             * i.e. will it be used as a source or destination for a copy command
             * or as a vertex buffer, uniform buffer
             * @param cpu_visible should it be directly accessible by the cpu (slow, mainly used for transfer buffers)
            */
            void init(const VkDevice& device, const std::vector<uint32_t>& queue_ids, bool cpu_visible, VkBufferUsageFlags usage);
            void cleanUp();

            const VkBuffer& getBuffer()const;

            /** @brief writes the specified data to the buffer
             * make sure that there is enough memory allocated before trying to set the data
             * will only work if the buffer is initialized as cpu visible */
            void setData(uint32_t byte_size, uint32_t offset, const void* data);

            void allocate(const LogicalDevice& device, uint32_t byte_size);

            uint32_t getAllocatedSize() const;

        protected:
            // creating buffer related structs

            VkBufferCreateInfo static createBufferCreateInfo(uint32_t byte_size, VkBufferUsageFlags usage, const std::vector<uint32_t>& queue_ids);
            VkMemoryAllocateInfo static createMemoryAllocateInfo(uint32_t byte_size, uint32_t mem_type_index);

        public:
        
            VkBufferCopy static createBufferCopy(uint32_t byte_size, uint32_t src_offset, uint32_t dst_offset);

        };

    } // vulkan

} // graphics

#endif // BUFFER_H