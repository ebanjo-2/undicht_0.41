#include "buffer.h"
#include "command_buffer.h"
#include "debug.h"
#include <cstring>
#include "set"

#include "vma_global_allocator.h"

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
            _cpu_visible = cpu_visible;

            // making sure that all queue ids are unique
            std::set<uint32_t> unique_queue_ids(queue_ids.begin(), queue_ids.end());
            _queue_ids.clear();
            _queue_ids.insert(_queue_ids.begin(), unique_queue_ids.begin(), unique_queue_ids.end());

        }

        void Buffer::init(const Buffer& b) {
            // copy the init settings, wont allocate memory

            init(b._device_handle, b._queue_ids, b._cpu_visible, b._usage);
        }

        void Buffer::cleanUp() {

            vmaDestroyBuffer(vma::GlobalAllocator::get(), _buffer, _vma_memory);
            _vma_memory_info = {};
        }

        const VkBuffer& Buffer::getBuffer() const {
            
            return _buffer;
        }

        void Buffer::setData(uint32_t byte_size, uint32_t offset, const void* data) {
            /** @brief writes the specified data to the buffer
             * make sure that there is enough memory allocated before trying to set the data
             * will only work if the buffer is initialized as cpu visible */

            if(byte_size + offset > getAllocatedSize()) {
                UND_ERROR << "failed to store data in buffer: not enough memory allocated\n";
                UND_LOG << "Note: allocated memory: " << getAllocatedSize() << " byte_size + offset: " << byte_size + offset << "\n";
                return;
            }

            if(!_cpu_visible) {
                UND_ERROR << "failed to store data in buffer: memory is not accessible from the cpu\n";
                return;
            }

            // if _cpu_visible is true, the memory is permanently mapped
            if(_vma_memory_info.pMappedData != nullptr) {
                std::memcpy(_vma_memory_info.pMappedData + offset, data, byte_size); 
            } else {
                UND_ERROR << "failed to copy data to buffer : memory hasnt been mapped\n";
            }

        }

        void Buffer::allocate(const LogicalDevice& device, uint32_t byte_size) {

            if(byte_size <= getAllocatedSize()) return;

            // freeing previously allocated memory
            if(_buffer != VK_NULL_HANDLE) vmaDestroyBuffer(vma::GlobalAllocator::get(), _buffer, _vma_memory);

            VkBufferCreateInfo buffer_info = createBufferCreateInfo(byte_size, _usage, _queue_ids);
            VmaAllocationCreateInfo alloc_info = {};
            
            if(_cpu_visible) {
                alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
                alloc_info.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
                alloc_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
            } else {
                alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            }

            VkResult result = vmaCreateBuffer(vma::GlobalAllocator::get(), &buffer_info, &alloc_info, &_buffer, &_vma_memory, &_vma_memory_info);
            if(result != VK_SUCCESS) {
                UND_LOG << "failed to create buffer\n";
            }

        }

        uint32_t Buffer::getAllocatedSize() const {

            return _vma_memory_info.size;
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
            info.flags = 0;

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