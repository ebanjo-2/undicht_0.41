#include <cstring>
#include "buffer.h"
#include "debug.h"

#include "command_buffer.h"

namespace undicht {

    namespace vulkan {

        void Buffer::init(const LogicalDevice& device, uint32_t byte_size, const std::vector<uint32_t>& queue_ids, VkBufferUsageFlags usage, bool cpu_visible) {

            _device_handle = device.getDevice();
            _usage = usage;

            if(cpu_visible) {
                _mem_properties |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                _mem_properties |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            }

            // specifying additional buffer usage flags
            _usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; // to be able to copy data to the buffer from another buffer
            _usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // to be able to copy data from the buffer to another buffer

            // creating the buffer object
            VkBufferCreateInfo buffer_info = createBufferCreateInfo(_usage, byte_size, queue_ids);
            vkCreateBuffer(_device_handle, &buffer_info, {}, &_buffer);

            // declaring the type of memory that can be used for this buffer
            VkMemoryRequirements mem_requirements;
            vkGetBufferMemoryRequirements(_device_handle, _buffer, &mem_requirements);
            VkMemoryType mem_type = createMemoryType(mem_requirements, _mem_properties);

            // finding a memory of the required type
            uint32_t mem_id = device.findMemory(mem_type);

            // allocating memory
            VkMemoryAllocateInfo allocate_info = createMemoryAllocateInfo(byte_size, mem_id);
            vkAllocateMemory(_device_handle, &allocate_info, {}, &_memory);

            _allocated_mem_size = byte_size;

            // binding the memory to the buffer
            vkBindBufferMemory(_device_handle, _buffer, _memory, 0);

        }

        void Buffer::cleanUp() {

            vkFreeMemory(_device_handle, _memory, {});
            vkDestroyBuffer(_device_handle, _buffer, {});

            _memory = VK_NULL_HANDLE;
            _buffer = VK_NULL_HANDLE;
            _allocated_mem_size = 0;
        }

        void Buffer::setData(const void* data, uint32_t byte_size, uint32_t offset) {
            // directly storing the data from the cpu (buffer needs to be initialized as cpu visible)

            if(byte_size + offset > _allocated_mem_size)
                UND_ERROR << "failed to store data in buffer: not enough memory allocated\n";

            // mapping the memory to a cpu visible address
            void* buffer;
            vkMapMemory(_device_handle, _memory, offset, byte_size, {}, &buffer);

            // copying the data into the mapped buffer
            std::memcpy(buffer, data, byte_size);

            // unmapping the memory (vulkan can now transfer it to the gpu)
            vkUnmapMemory(_device_handle, _memory);

        }


        uint32_t Buffer::getAllocatedMemorySize() const {

            return _allocated_mem_size;
        }

        const VkBuffer& Buffer::getBuffer() const {

            return _buffer;
        }

        //////////////////////////////////// creating buffer related structs ////////////////////////////////////

        VkBufferCreateInfo Buffer::createBufferCreateInfo(VkBufferUsageFlags usage_flags, uint32_t byte_size, const std::vector<uint32_t>& queue_ids) {

            VkBufferCreateInfo info{};
            info.sharingMode = queue_ids.size() == 1?VK_SHARING_MODE_EXCLUSIVE:VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = queue_ids.size();
            info.pQueueFamilyIndices = queue_ids.data();
            info.size = byte_size;

            return info;
        }

        VkMemoryType Buffer::createMemoryType(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties) {

            VkMemoryType mem_type{};
            mem_type.heapIndex = requirements.memoryTypeBits; // bitfield specifying which memory types can be used
            mem_type.propertyFlags = properties;

            return mem_type;
        }

        VkMemoryAllocateInfo Buffer::createMemoryAllocateInfo(uint32_t byte_size, uint32_t mem_type_index) {

            VkMemoryAllocateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            info.pNext = nullptr;
            info.allocationSize = byte_size;
            info.memoryTypeIndex = mem_type_index;

            return info;
        }

        VkBufferCopy Buffer::createBufferCopy(uint32_t byte_size, uint32_t src_offset, uint32_t dst_offset) {

            VkBufferCopy c{};
            c.size = byte_size;
            c.srcOffset = src_offset;
            c.dstOffset = dst_offset;

            return c;
        }

    }
}

