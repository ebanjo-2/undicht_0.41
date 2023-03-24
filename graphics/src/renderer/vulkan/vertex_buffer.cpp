#include "vertex_buffer.h"
#include "vector"
#include "debug.h"

namespace undicht {

    namespace vulkan {

        void VertexBuffer::init(const LogicalDevice& device) {

            _device_handle = &device;
            
            // initializing the internal buffers
            std::vector<uint32_t> queue_ids = {device.getGraphicsQueueFamily(), device.getTransferQueueFamily()};

            _vertex_buffer.init(device.getDevice(), queue_ids, false, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            _index_buffer.init(device.getDevice(), queue_ids, false, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            _instance_buffer.init(device.getDevice(), queue_ids, false, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            _transfer_buffer.init(device.getDevice(), {device.getTransferQueueFamily()}, true, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

            // initializing the copy command buffer
            //_copy_cmd.init(device.getDevice(), device.getTransferCmdPool());
        }

        void VertexBuffer::cleanUp() {

            //_copy_cmd.cleanUp();

            _vertex_buffer.cleanUp();
            _index_buffer.cleanUp();
            _instance_buffer.cleanUp();
            _transfer_buffer.cleanUp();

        }

        void VertexBuffer::setVertexData(const void* data, uint32_t byte_size, uint32_t offset, CommandBuffer& cmd) {

            transferData(data, byte_size, offset, _vertex_buffer, cmd);

        }


        void VertexBuffer::setIndexData(const void* data, uint32_t byte_size, uint32_t offset, CommandBuffer& cmd) {

            transferData(data, byte_size, offset, _index_buffer, cmd);

        }


        void VertexBuffer::setInstanceData(const void* data, uint32_t byte_size, uint32_t offset, CommandBuffer& cmd) {

            transferData(data, byte_size, offset, _instance_buffer, cmd);
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

        void VertexBuffer::transferData(const void* data, uint32_t byte_size, uint32_t offset, Buffer& dst, CommandBuffer& cmd) {
            
            // allocating memory + copying the old data if necessary
            if(dst.getAllocatedSize() <= (byte_size + offset)) {
                // copying the data from the old dst buffer to the new one
                Buffer new_dst;
                new_dst.init(dst);
                new_dst.allocate(*_device_handle, byte_size + offset);

                if(dst.getUsedSize() > 0) {
                    copyData(dst, 0, dst.getUsedSize(), new_dst, 0, cmd);
                }
                
                dst.cleanUp();
                dst = new_dst;
            }
            
            // transfering the data
            if(!byte_size) return;

            // storing the data in the transfer buffer
            if(_transfer_buffer.getAllocatedSize() < byte_size) _transfer_buffer.allocate(*_device_handle, byte_size);
            _transfer_buffer.setData(byte_size, 0, data);

            // using a command buffer to copy the data to the dst buffer
            copyData(_transfer_buffer, 0, byte_size, dst, offset, cmd);

            dst.setUsedSize(std::max(dst.getUsedSize(), byte_size + offset));
        }

        void VertexBuffer::copyData(Buffer& src, uint32_t offset_src, uint32_t byte_size, Buffer& dst, uint32_t offset_dst, CommandBuffer& cmd) {
            // assuming that both buffers have sufficient memory allocated

            VkBufferCopy copy_info = Buffer::createBufferCopy(byte_size, offset_src, offset_dst);
            /*cmd.beginCommandBuffer(true);*/
            cmd.copy(src.getBuffer(), dst.getBuffer(), copy_info);
           // cmd.endCommandBuffer();
            /*_device_handle->submitOnTransferQueue(cmd.getCommandBuffer());
            _device_handle->waitTransferQueueIdle(); // waiting for the transfer to finish (should probably use semaphores or smth.)
*/
        }

    } // vulkan

} // undicht