#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "core/vulkan/buffer.h"
#include "core/vulkan/logical_device.h"
#include "core/vulkan/command_buffer.h"
#include "renderer/vulkan/transfer_buffer.h"

namespace undicht {

    namespace vulkan {

        class VertexBuffer {

        protected:

            const LogicalDevice* _device_handle = 0;

            Buffer _vertex_buffer;
            Buffer _index_buffer;
            Buffer _instance_buffer;

        public:

            void init(const LogicalDevice& device);
            void cleanUp();

            /// @brief allocate memory for the buffer
            /// @param copy_old_data copies the data from the old buffer to the new one
            /// will wait for the gpu to finish the copying (because the old buffer will be deleted afterwards)
            void allocateVertexBuffer(uint32_t byte_size, bool copy_old_data = false);
            void allocateIndexBuffer(uint32_t byte_size, bool copy_old_data = false);
            void allocateInstanceBuffer(uint32_t byte_size, bool copy_old_data = false);

            /// @brief writing to the individual buffers
            /// @return false, if there isnt enough memory already allocated for the buffer
            bool setVertexData(const void* data, uint32_t byte_size, uint32_t offset, CommandBuffer& cmd, TransferBuffer& transfer_buffer);
            bool setIndexData(const void* data, uint32_t byte_size, uint32_t offset, CommandBuffer& cmd, TransferBuffer& transfer_buffer);
            bool setInstanceData(const void* data, uint32_t byte_size, uint32_t offset, CommandBuffer& cmd, TransferBuffer& transfer_buffer);

            const Buffer& getVertexBuffer() const;
            const Buffer& getIndexBuffer() const;
            const Buffer& getInstanceBuffer() const;

        protected:
            // internal functions

            bool storeData(const void* data, uint32_t byte_size, uint32_t offset, CommandBuffer& cmd, Buffer& dst, TransferBuffer& transfer_buffer);
            
            /// @brief allocate memory for the buffer
            /// @param copy_old_data copies the data from the old buffer to the new one
            /// will wait for the gpu to finish the copying (because the old buffer will be deleted afterwards)
            void allocateBufferMemory(Buffer& dst, uint32_t byte_size, bool copy_old_data);

            //void transferData(const void* data, uint32_t byte_size, uint32_t offset, Buffer& dst, CommandBuffer& cmd);
            void copyData(Buffer& src, uint32_t offset_src, uint32_t byte_size, Buffer& dst, uint32_t offset_dst, CommandBuffer& cmd);

        };

    } // vulkan

} // undicht

#endif // VERTEX_BUFFER_H