#include "texture.h"

#include "debug.h"

namespace undicht {

    namespace vulkan {

        void Texture::setExtent(uint32_t width, uint32_t height, uint32_t depth) {
            
            _width = width;
            _height = height;
            _depth = depth;

        }

        void Texture::setFormat(VkFormat format) {

            _format = format;

        }

        void Texture::init(const LogicalDevice& device) {

            _device_handle = &device;

            // init the image
            _image.init(device.getDevice());
            _image.allocate(device, _width, _height, _depth, _layers, _format);

            // init the transfer buffer
            _transfer_buffer.init(device.getDevice(), {device.getTransferQueueFamily()}, true, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            
            // initializing the command buffers
            _copy_cmd.init(device.getDevice(), device.getTransferCmdPool());
            _layout_cmd.init(device.getDevice(), device.getGraphicsCmdPool());

        }

        void Texture::cleanUp() {

            _copy_cmd.cleanUp();
            _layout_cmd.cleanUp();
            _transfer_buffer.cleanUp();
            _image.cleanUp();

        }

        const Image& Texture::getImage() const {

            return _image;
        }

        const VkImageLayout& Texture::getLayout() const {

            return _layout;
        }


        void Texture::setData(const char* data, uint32_t byte_size) {

            // transition to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            transitionToLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

            // store the data in the transfer buffer
            _transfer_buffer.allocate(*_device_handle, byte_size);
            _transfer_buffer.setData(byte_size, 0, data);

            // copy data from transfer buffer to texture
            VkBufferImageCopy copy_info = Image::createBufferImageCopy(_image.getExtent(), VK_IMAGE_ASPECT_COLOR_BIT);
            _copy_cmd.beginCommandBuffer(true);
            _copy_cmd.copy(_transfer_buffer.getBuffer(), _image.getImage(), _layout, copy_info);
            _copy_cmd.endCommandBuffer();
            _device_handle->submitOnTransferQueue(_copy_cmd.getCommandBuffer());
            _device_handle->waitTransferQueueIdle();

            // transition to VK_ACCESS_SHADER_READ_BIT
            transitionToLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            
        }

        //////////////////////////// internal functions ////////////////////////////

        void Texture::transitionToLayout(VkImageLayout new_layout, VkAccessFlags new_access, VkPipelineStageFlagBits src_stage, VkPipelineStageFlagBits dst_stage) {

            VkImageSubresourceRange range = Image::createImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
            VkImageMemoryBarrier barrier = Image::createImageMemoryBarrier(_image.getImage(), range, _layout, new_layout, _access_flags, new_access);
            
            _layout_cmd.beginCommandBuffer(true);
            _layout_cmd.pipelineBarrier(barrier, src_stage, dst_stage);
            _layout_cmd.endCommandBuffer();
            _device_handle->submitOnGraphicsQueue(_layout_cmd.getCommandBuffer());
            _device_handle->waitGraphicsQueueIdle();

            _layout = new_layout;
            _access_flags = new_access;

        }

    } // vulkan

} // undicht