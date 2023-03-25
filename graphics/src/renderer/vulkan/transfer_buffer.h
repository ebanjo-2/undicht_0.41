#ifndef TRANSFER_BUFFER_H
#define TRANSFER_BUFFER_H

#include "core/vulkan/logical_device.h"
#include "core/vulkan/buffer.h"
#include "vector"

namespace undicht {

    namespace vulkan {

        class TransferBuffer {
            /**
             * @brief fast gpu memory is often not directly accessible from the cpu
             * so, to copy data into that memory, you need to first store that data
             * in a transfer buffer, and then tell the vulkan api to copy the data 
             * from the buffer to the gpu
             * this class represents such a transfer buffer, it can store data for 
             * multiple transfers to the gpu and generate the necessary buffer copy infos
             * for vulkan
             */
          protected:

            struct InternalBufferData {
                // information about the data that is stored in a single buffer
                // (using multiple buffers to avoid having to resize a single large buffer
                // which would disrupt previously issued copy commands)

                vulkan::Buffer _buffer;
                std::vector<uint32_t> _offsets; // offsets for the data in the buffer
                std::vector<uint32_t> _sizes; // sizes of the data in the buffer
            };

            std::vector<InternalBufferData> _transfer_buffers;

          protected:

            undicht::vulkan::LogicalDevice _device_handle;
            std::vector<uint32_t> _queue_ids;

          public:

            void init(const LogicalDevice& device);
            void cleanUp();

            /// @brief frees all internal buffers, 
            /// no need to init() again if you want to use the transfer buffer again
            void freeInternalBuffers();

            /// @brief allocates a new internal buffer of the specified size
            InternalBufferData& allocateInternalBuffer(uint32_t byte_size);

            /** @brief adds the data to the internal buffer
             * @param transfer_src a pointer to the internal buffer in which the data was stored
             * and which can be used as the source buffer for the copy command
             * @return the VkBufferCopy, which can be used in a command buffer */
            VkBufferCopy stageTransfer(const char* data, uint32_t byte_size, VkBuffer& transfer_src, uint32_t dst_offset);
            VkBufferImageCopy stageTransfer(const char* data, uint32_t byte_size, VkBuffer& transfer_src, VkExtent3D image_extent, VkOffset3D image_offset, VkImageAspectFlags flags, uint32_t layer = 0, uint32_t mip_level = 0);

            /** @brief to be called after all the data was transfered to the final memory location
             * will clear the internal buffers, so that their allocated memory can be reused for 
             * future transfers */
            void clearStagedTransfers();

          protected:
            // internal functions

            InternalBufferData& storeInInternalBuffer(const char* data, uint32_t byte_size);

            /// @brief find an internal buffer with enough empty memory for the transfer
            /// if no such buffer was found, a new one will be allocated
            /// @return a reference to the InternalBufferData struct
            InternalBufferData& findInternalBuffer(uint32_t byte_size);

            /// @brief offsets need to be aligned to a multiple of 16
            /// this function calculates the next valid offset following the location
            uint32_t nextOffset(uint32_t location);

        };

    } // vulkan

} // undicht

#endif // TRANSFER_BUFFER_H