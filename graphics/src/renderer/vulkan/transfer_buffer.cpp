#include "transfer_buffer.h"
#include "renderer/vulkan/immediate_command.h"
#include "core/vulkan/buffer.h"
#include "core/vulkan/image.h"
#include "vulkan/vulkan.h"
#include "debug.h"
#include "algorithm"

namespace undicht {

    namespace vulkan {

        void TransferBuffer::init(const LogicalDevice& device) {
            
            // used to init the internal buffers when they are needed
            _device_handle = device;

            _queue_ids = {device.getGraphicsQueueFamily(), device.getTransferQueueFamily()};
        }

        void TransferBuffer::cleanUp() {

            freeInternalBuffers();
        }

        void TransferBuffer::freeInternalBuffers() {
            /// @brief frees all internal buffers, 
            /// no need to init() again if you want to use the transfer buffer again

            // UND_LOG << "freeing " << _transfer_buffers.size() << " transfer buffers\n";

            for(InternalBufferData& internal_buffer : _transfer_buffers)
                internal_buffer._buffer.cleanUp();

            _transfer_buffers.clear();
        }

        TransferBuffer::InternalBufferData& TransferBuffer::allocateInternalBuffer(uint32_t byte_size) {
            /// @brief allocates a new internal buffer of the specified size

            // avoid allocating very small buffers
            byte_size = std::max(byte_size, 4096u);

            InternalBufferData new_buffer;
            new_buffer._buffer.init(_device_handle.getDevice(), _queue_ids, true, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            new_buffer._buffer.allocate(_device_handle, byte_size);

            _transfer_buffers.push_back(new_buffer);
            return _transfer_buffers.back();
        }

        VkBufferCopy TransferBuffer::stageTransfer(const char* data, uint32_t byte_size, VkBuffer& transfer_src, uint32_t dst_offset) {
            /** @brief adds the data to the internal buffer
             * @param transfer_src a pointer to the internal buffer in which the data was stored
             * and which can be used as the source buffer for the copy command
             * @return the VkBufferCopy, which can be used in a command buffer */
            
            // store the data in an internal buffer
            InternalBufferData& buffer_data = storeInInternalBuffer(data, byte_size);

            // fill out the VkBufferCopy struct
            uint32_t src_offset = buffer_data._offsets.back();
            VkBufferCopy buffer_copy = Buffer::createBufferCopy(byte_size, src_offset, dst_offset);

            // return the copy info + buffer
            transfer_src = buffer_data._buffer.getBuffer();
            return buffer_copy;
        }

        VkBufferImageCopy TransferBuffer::stageTransfer(const char* data, uint32_t byte_size, VkBuffer& transfer_src, VkExtent3D image_extent, VkOffset3D image_offset, VkImageAspectFlags flags, uint32_t layer, uint32_t mip_level) {
            
            // store the data in an internal buffer
            InternalBufferData& buffer_data = storeInInternalBuffer(data, byte_size);

            // fill out the VkBufferCopy struct
            uint32_t src_offset = buffer_data._offsets.back();
            VkBufferImageCopy image_copy = Image::createBufferImageCopy(image_extent, image_offset, flags, layer, mip_level, src_offset);

            // return the copy info + buffer
            transfer_src = buffer_data._buffer.getBuffer();
            return image_copy;
        }

        void TransferBuffer::clearStagedTransfers() {
            /** @brief to be called after all the data was transfered to the final memory location
            * will clear the internal buffer, so that its allocated memory can be reused for 
            * future transfers */

            for(InternalBufferData& internal_buffer : _transfer_buffers) {
                //internal_buffer._buffer.setUsedSize(0);
                internal_buffer._offsets.clear();
                internal_buffer._sizes.clear();
            }

        }

        //////////////////////////////////////// internal functions ////////////////////////////////////////

        TransferBuffer::InternalBufferData& TransferBuffer::storeInInternalBuffer(const char* data, uint32_t byte_size) {

            // find (or allocate) a buffer to store the data in
            InternalBufferData& buffer_data = findInternalBuffer(byte_size);

            // store the data in that buffer
            uint32_t offset = 0;
            if(buffer_data._offsets.size()) offset = nextOffset(buffer_data._offsets.back() + buffer_data._sizes.back());
            buffer_data._buffer.setData(byte_size, offset, data);
            buffer_data._offsets.push_back(offset);
            buffer_data._sizes.push_back(byte_size);

            return buffer_data;
        }

        TransferBuffer::InternalBufferData& TransferBuffer::findInternalBuffer(uint32_t byte_size) {
            /// @brief find an internal buffer with enough empty memory for the transfer
            /// if no such buffer was found, a new one will be allocated
            /// @return a reference to the InternalBufferData struct

            // searching for a buffer that has enough free memory
            for(InternalBufferData& internal_buffer : _transfer_buffers) {

                uint32_t next_offset = 0;
                if(internal_buffer._offsets.size()) next_offset = nextOffset(internal_buffer._offsets.back() + internal_buffer._sizes.back());
                next_offset = std::min(internal_buffer._buffer.getAllocatedSize(), next_offset); // to avoid negative numbers

                if(internal_buffer._buffer.getAllocatedSize() - next_offset >= byte_size) {
                    return internal_buffer;
                }
            }

            // no buffer could be reused, so allocate a new one
            return allocateInternalBuffer(byte_size);
        }


        uint32_t TransferBuffer::nextOffset(uint32_t location) {
            /// @brief offsets need to be aligned to a multiple of 16
            /// this function calculates the next valid offset following the location

            return location + (location % 16);
        }

    } // vulkan

} // undicht