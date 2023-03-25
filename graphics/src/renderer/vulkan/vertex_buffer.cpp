#include "vertex_buffer.h"
#include "vector"
#include "debug.h"
#include "immediate_command.h"

namespace undicht {

    namespace vulkan {

        void VertexBuffer::init(const LogicalDevice& device) {

            _device_handle = &device;
            
            // initializing the internal buffers
            std::vector<uint32_t> queue_ids = {device.getGraphicsQueueFamily(), device.getTransferQueueFamily()};

            _vertex_buffer.init(device.getDevice(), queue_ids, false, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            _index_buffer.init(device.getDevice(), queue_ids, false, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            _instance_buffer.init(device.getDevice(), queue_ids, false, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            
        }

        void VertexBuffer::cleanUp() {

            _vertex_buffer.cleanUp();
            _index_buffer.cleanUp();
            _instance_buffer.cleanUp();
        }

        void VertexBuffer::allocateVertexBuffer(uint32_t byte_size, bool copy_old_data) {

            allocateBufferMemory(_vertex_buffer, byte_size, copy_old_data);
        }

        void VertexBuffer::allocateIndexBuffer(uint32_t byte_size, bool copy_old_data) {

            allocateBufferMemory(_index_buffer, byte_size, copy_old_data);
        }

        void VertexBuffer::allocateInstanceBuffer(uint32_t byte_size, bool copy_old_data) {

            allocateBufferMemory(_instance_buffer, byte_size, copy_old_data);
        }

        bool VertexBuffer::setVertexData(const void* data, uint32_t byte_size, uint32_t offset, CommandBuffer& cmd, TransferBuffer& transfer_buffer) {
        
            return storeData(data, byte_size, offset, cmd, _vertex_buffer, transfer_buffer);
        }

        bool VertexBuffer::setIndexData(const void* data, uint32_t byte_size, uint32_t offset, CommandBuffer& cmd, TransferBuffer& transfer_buffer) {
            
            return storeData(data, byte_size, offset, cmd, _index_buffer, transfer_buffer);
        }

        bool VertexBuffer::setInstanceData(const void* data, uint32_t byte_size, uint32_t offset, CommandBuffer& cmd, TransferBuffer& transfer_buffer) {
            
            return storeData(data, byte_size, offset, cmd, _instance_buffer, transfer_buffer);
        }

        const Buffer& VertexBuffer::getVertexBuffer() const {
            
            return _vertex_buffer;
        }

        const Buffer& VertexBuffer::getIndexBuffer() const {
            
            return _index_buffer;
        }

        const Buffer& VertexBuffer::getInstanceBuffer() const {

            return _instance_buffer;
        }

        ////////////////////////////////////// internal funktions //////////////////////////////

        void VertexBuffer::allocateBufferMemory(Buffer& dst, uint32_t byte_size, bool copy_old_data) {
            /// @brief allocate memory for the buffer
            /// @param copy_old_data copies the data from the old buffer to the new one
            /// will wait for the gpu to finish the copying (because the old buffer will be deleted afterwards)

            if(copy_old_data && dst.getUsedSize()) {
                Buffer new_buffer;
                new_buffer.init(dst);
                new_buffer.allocate(*_device_handle, byte_size);
                {
                    ImmediateCommand cmd(*_device_handle);
                    copyData(dst, 0, dst.getUsedSize(), new_buffer, 0, cmd);
                } // cmd gets submitted
                dst.cleanUp();
                dst = new_buffer;
            } else {
                dst.allocate(*_device_handle, byte_size);
            }
            
        }

        bool VertexBuffer::storeData(const void* data, uint32_t byte_size, uint32_t offset, CommandBuffer& cmd, Buffer& dst, TransferBuffer& transfer_buffer) {
            
            if(dst.getAllocatedSize() < (byte_size + offset)) {
                // cant store the data
                UND_ERROR << "failed to store the data, please allocate enough memory\n";
                UND_LOG << "Note: data size: " << byte_size << ", offset: " << offset << " allocated: " << dst.getAllocatedSize() << "\n";
                return false;
            }

            // store the data in the transfer buffer
            VkBuffer transfer_src;
            VkBufferCopy copy_info = transfer_buffer.stageTransfer((const char*)data, byte_size, transfer_src, offset);

            // add the copy command to the cmd buffer
            cmd.copy(transfer_src, dst.getBuffer(), copy_info);

            // update the used size
            dst.setUsedSize(std::max(dst.getUsedSize(), byte_size + offset));

            return true;
        }  

        void VertexBuffer::copyData(Buffer& src, uint32_t offset_src, uint32_t byte_size, Buffer& dst, uint32_t offset_dst, CommandBuffer& cmd) {
            // assuming that both buffers have sufficient memory allocated

            VkBufferCopy copy_info = Buffer::createBufferCopy(byte_size, offset_src, offset_dst);
            cmd.copy(src.getBuffer(), dst.getBuffer(), copy_info);
        }

    } // vulkan

} // undicht