#include "vertex_buffer.h"
#include "vector"
#include "debug.h"

namespace undicht {

    namespace vulkan {

        void VertexBuffer::init(const LogicalDevice& device) {

            _device_handle = &device;
            
            // initializing the internal buffers
            std::vector<uint32_t> queue_ids = {device.getGraphicsQueueFamily(), device.getTransferQueueFamily()};

            _vertex_buffer.init(device.getDevice(), queue_ids, false, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            _index_buffer.init(device.getDevice(), queue_ids, false, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            _instance_buffer.init(device.getDevice(), queue_ids, false, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            _transfer_buffer.init(device.getDevice(), {device.getTransferQueueFamily()}, true, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

            // initializing the copy command buffer
            _copy_cmd.init(device.getDevice(), device.getTransferCmdPool());
        }

        void VertexBuffer::cleanUp() {

            _copy_cmd.cleanUp();

            _vertex_buffer.cleanUp();
            _index_buffer.cleanUp();
            _instance_buffer.cleanUp();
            _transfer_buffer.cleanUp();

        }

        void VertexBuffer::setVertexData(const void* data, uint32_t byte_size, uint32_t offset) {

            transferData(data, byte_size, offset, _vertex_buffer);

        }


        void VertexBuffer::setIndexData(const void* data, uint32_t byte_size, uint32_t offset) {

            transferData(data, byte_size, offset, _index_buffer);

        }


        void VertexBuffer::setInstanceData(const void* data, uint32_t byte_size, uint32_t offset) {

            transferData(data, byte_size, offset, _instance_buffer);

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

        void VertexBuffer::transferData(const void* data, uint32_t byte_size, uint32_t offset, Buffer& dst) {
            
            if(dst.getAllocatedSize() < byte_size + offset) dst.allocate(*_device_handle, byte_size + offset);

            if(!byte_size) return;

            // storing the data in the transfer buffer
            if(_transfer_buffer.getAllocatedSize() < byte_size) _transfer_buffer.allocate(*_device_handle, byte_size);
            _transfer_buffer.setData(byte_size, 0, data);

            // using a command buffer to copy the data to the dst buffer
            VkBufferCopy copy_info = Buffer::createBufferCopy(byte_size, 0, offset);
            _copy_cmd.beginCommandBuffer(true);
            _copy_cmd.copy(_transfer_buffer.getBuffer(), dst.getBuffer(), copy_info);
            _copy_cmd.endCommandBuffer();
            _device_handle->submitOnTransferQueue(_copy_cmd.getCommandBuffer());
            _device_handle->waitTransferQueueIdle(); // waiting for the transfer to finish (should probably use semaphores or smth.)

        }


    } // vulkan

} // undicht