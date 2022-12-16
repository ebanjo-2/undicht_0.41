#include "buffer.h"
#include "command_buffer.h"
#include "debug.h"
#include <cstring>

namespace undicht {

    namespace vulkan {

        void Buffer::init(const VkDevice& device, const std::vector<uint32_t>& queue_ids, bool cpu_visible, VkBufferUsageFlags usage) {
            /** @param queue_ids: the queues from which the buffer is going to be accessed 
             * @param usage: you have to specify in which way the buffer is going to be used:
             * i.e. will it be used as a source or destination for a copy command
             * or as a vertex buffer, uniform buffer
             * @param cpu_visible should it be directly accessible by the cpu (slow, mainly used for transfer buffers)
            */

            _device_handle = device;
            _usage = usage;
            _queue_ids = queue_ids;
            _cpu_visible = cpu_visible;

            if(cpu_visible) {
                _mem_properties |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
                _mem_properties |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            } else {
                _mem_properties |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            }

        }

        void Buffer::init(const Buffer& b) {
            // copy the init settings, wont allocate memory

            init(b._device_handle, b._queue_ids, b._cpu_visible, b._usage);
        }

        void Buffer::cleanUp() {
            
            vkFreeMemory(_device_handle, _memory, {});
            vkDestroyBuffer(_device_handle, _buffer, {});

            _allocated_mem_size = 0;
        }

        const VkBuffer& Buffer::getBuffer() const {
            
            return _buffer;
        }


        void Buffer::setData(uint32_t byte_size, uint32_t offset, const void* data) {
            /** @brief writes the specified data to the buffer
             * make sure that there is enough memory allocated before trying to set the data
             * will only work if the buffer is initialized as cpu visible */

            if(byte_size + offset > _allocated_mem_size) {
                UND_ERROR << "failed to store data in buffer: not enough memory allocated\n";
                return;
            }

            if(!(_mem_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
                UND_ERROR << "failed to store data in buffer: memory is not accessible from the cpu\n";
                return;
            }

            _used_mem_size = byte_size + offset;

            void* mem_handle;
            vkMapMemory(_device_handle, _memory, offset, byte_size, {}, &mem_handle);
            if(mem_handle)
                std::memcpy(mem_handle, data, byte_size);
            else
                UND_ERROR << "failed to map vram buffer to cpu accessible memory\n";
            vkUnmapMemory(_device_handle, _memory);
            
        }

        void Buffer::allocate(const LogicalDevice& device, uint32_t byte_size) {

            // freeing previously allocated memory 
            if(_buffer != VK_NULL_HANDLE) vkDestroyBuffer(_device_handle, _buffer, {});
            if(_memory != VK_NULL_HANDLE) vkFreeMemory(_device_handle, _memory, {});

            // creating the buffer
            VkBufferCreateInfo buffer_info = createBufferCreateInfo(byte_size, _usage, _queue_ids);
            vkCreateBuffer(_device_handle, &buffer_info, {}, &_buffer);

            // getting the drivers? requirements for this buffers memory
            VkMemoryRequirements mem_requirements;
            vkGetBufferMemoryRequirements(_device_handle, _buffer, &mem_requirements);

            byte_size = std::max(byte_size, (uint32_t)mem_requirements.size);

            // declaring the memory type
            VkMemoryType mem_type{};
            mem_type.heapIndex = mem_requirements.memoryTypeBits;
            mem_type.propertyFlags = _mem_properties;
            uint32_t mem_type_index = device.getMemoryTypeIndex(mem_type);

            // allocating the memory
            VkMemoryAllocateInfo allocate_info = createMemoryAllocateInfo(byte_size, mem_type_index);
            vkAllocateMemory(_device_handle, &allocate_info, {}, &_memory);

            // linking the memory to the buffer
            vkBindBufferMemory(_device_handle, _buffer, _memory, 0);
            _allocated_mem_size = byte_size;

        }

        uint32_t Buffer::getAllocatedSize() const {

            return _allocated_mem_size;
        }


        void Buffer::setUsedSize(uint32_t byte_size) {
            // when the data is copyied into the buffer via a command buffer
            // the size of the data stored in the buffer has to be manually set

            _used_mem_size = byte_size;
        }

        uint32_t Buffer::getUsedSize() const {

            return _used_mem_size;
        }

        ///////////////////////////////// creating buffer related structs /////////////////////////////////

        VkBufferCreateInfo Buffer::createBufferCreateInfo(uint32_t byte_size, VkBufferUsageFlags usage, const std::vector<uint32_t>& queue_ids) {

            VkBufferCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            info.pNext = nullptr;
            info.usage = usage;
            info.queueFamilyIndexCount = queue_ids.size();
            info.pQueueFamilyIndices = queue_ids.data();
            info.sharingMode = queue_ids.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
            info.size = byte_size;

            return info;
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

            VkBufferCopy copy_info{};
            copy_info.size = byte_size;
            copy_info.srcOffset = src_offset;
            copy_info.dstOffset = dst_offset;

            return copy_info;
        }

    } // vulkan

} // undicht