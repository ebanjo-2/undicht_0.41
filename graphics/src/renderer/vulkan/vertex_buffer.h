#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "core/vulkan/buffer.h"
#include "core/vulkan/logical_device.h"
#include "core/vulkan/command_buffer.h"

namespace undicht {

    namespace vulkan {

        class VertexBuffer {

        protected:

            const LogicalDevice* _device_handle = 0;

            Buffer _vertex_buffer;
            Buffer _index_buffer;
            Buffer _instance_buffer;
            Buffer _transfer_buffer;

            CommandBuffer _copy_cmd;


        public:

            void init(const LogicalDevice& device);
            void cleanUp();

            // writing to the individual buffers
            // if there isnt enough memory already allocated for the buffer, new memory will be allocated 
            // and the data that was stored in the buffer will be lost
            void setVertexData(const void* data, uint32_t byte_size, uint32_t offset);
            void setIndexData(const void* data, uint32_t byte_size, uint32_t offset);
            void setInstanceData(const void* data, uint32_t byte_size, uint32_t offset);

            const Buffer& getVertexBuffer() const;
            const Buffer& getIndexBuffer() const;
            const Buffer& getInstanceBuffer() const;

        protected:
            // internal functions

            void transferData(const void* data, uint32_t byte_size, uint32_t offset, Buffer& dst);
            void copyData(Buffer& src, uint32_t offset_src, uint32_t byte_size, Buffer& dst, uint32_t offset_dst);

        };

    } // vulkan

} // undicht

#endif // VERTEX_BUFFER_H