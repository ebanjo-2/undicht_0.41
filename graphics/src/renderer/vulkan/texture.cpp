#include "texture.h"

#include "debug.h"

namespace undicht {

    namespace vulkan {

        void Texture::setExtent(uint32_t width, uint32_t height, uint32_t depth, uint32_t layers) {
            
            _width = width;
            _height = height;
            _depth = depth;
            _layers = layers;

        }

        void Texture::setCubeMap(bool is_cube_map) {
            // the texture has to have 6 layers

            _is_cube_map = is_cube_map;

            if(is_cube_map)
                _layers = 6;

        }

        void Texture::setFormat(VkFormat format) {

            _format = format;

        }

        void Texture::setMipMaps(bool enable_mip_maps, bool auto_generate, uint32_t mip_levels) {

            _enable_mip_maps = enable_mip_maps;
            _auto_gen_mip_maps = auto_generate;
            _mip_levels = mip_levels;
        }

        void Texture::init(const LogicalDevice& device) {

            _device_handle = &device;

            // calc number of mip levels
            _mip_levels = _enable_mip_maps ? std::min(calcMipLevelCount(_width, _height), _mip_levels) : 1;

            // init the image
            _image.init(device.getDevice(), _is_cube_map);
            _image.allocate(device, _width, _height, _depth, _layers, _mip_levels, _format);

        }

        void Texture::cleanUp() {

            _image.cleanUp();
        }

        const Image& Texture::getImage() const {

            return _image;
        }

        const VkImageLayout& Texture::getLayout() const {

            return _layout;
        }


        void Texture::setData(CommandBuffer& cmd, TransferBuffer& transfer_buffer, const char* data, uint32_t byte_size, uint32_t layer, uint32_t mip_level, VkExtent3D data_image_extent, VkOffset3D offset_in_image) {

            if(data_image_extent.width == 0 && data_image_extent.height == 0){
                data_image_extent = _image.getExtent();
                data_image_extent.width /= (1 << mip_level);
                data_image_extent.height /= (1 << mip_level);
            }
            
            // transition to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            transitionToLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

            // store the data in the transfer buffer
            //_transfer_buffer.allocate(*_device_handle, byte_size);
            //_transfer_buffer.setData(byte_size, 0, data);
            VkBuffer transfer_src;
            VkBufferImageCopy copy_info = transfer_buffer.stageTransfer(data, byte_size, transfer_src, data_image_extent, offset_in_image, VK_IMAGE_ASPECT_COLOR_BIT, layer, mip_level);

            // copy data from transfer buffer to texture
            // VkBufferImageCopy copy_info = Image::createBufferImageCopy(data_image_extent, offset_in_image, VK_IMAGE_ASPECT_COLOR_BIT, layer, mip_level);
            cmd.copy(transfer_src, _image.getImage(), _layout, copy_info);

            if(_enable_mip_maps && _auto_gen_mip_maps) {
                // generate MipMaps (and transition to VK_ACCESS_SHADER_READ_BIT)

                genMipMaps(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);
            } else {
                // transition to VK_ACCESS_SHADER_READ_BIT without generating mip maps

                transitionToLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            }

        }



        //////////////////////////// internal functions ////////////////////////////

        void Texture::transitionToLayout(CommandBuffer& cmd, VkImageLayout new_layout, VkAccessFlags src_access, VkAccessFlags dst_access, VkPipelineStageFlagBits src_stage, VkPipelineStageFlagBits dst_stage) {

            VkImageSubresourceRange range = Image::createImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, _mip_levels, 0, _layers);
            VkImageMemoryBarrier barrier = Image::createImageMemoryBarrier(_image.getImage(), range, _layout, new_layout, src_access, dst_access);
            
            cmd.pipelineBarrier(barrier, src_stage, dst_stage);

            _layout = new_layout;

        }

        void Texture::genMipMaps(CommandBuffer& cmd, VkImageLayout new_layout, VkAccessFlags new_access) {
            // will also transition the mip levels to the new layout
            // source: https://vulkan-tutorial.com/Generating_Mipmaps

            //_layout_cmd.beginCommandBuffer(true); // generating mip maps takes place on the graphics queue

            VkImageSubresourceRange range = Image::createImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
            VkImageMemoryBarrier barrier;

            int32_t mip_width = _width;
            int32_t mip_height = _height;

            for (uint32_t i = 1; i < _mip_levels; i++) {
                // create mip maps for each mip level (starting at 1)

                // transition the next higher level to a read layout
                range.baseMipLevel = i - 1;
                range.levelCount = 1;
                barrier = Image::createImageMemoryBarrier(_image.getImage(), range, _layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
                cmd.pipelineBarrier(barrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

                // blit operation
                VkImageBlit blit = Image::createImageBlit(mip_width, mip_height, i - 1, i);
                cmd.blitImage(_image.getImage(), blit);

                // transition the next higher level to new_layout
                barrier = Image::createImageMemoryBarrier(_image.getImage(), range, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, new_layout, VK_ACCESS_TRANSFER_READ_BIT, new_access);
                cmd.pipelineBarrier(barrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
 
                // calc size of next mip level
                if (mip_width > 1) mip_width /= 2;
                if (mip_height > 1) mip_height /= 2;

            }

            // transition the last mip level to new_layout
            range.baseMipLevel = _mip_levels - 1;
            barrier = Image::createImageMemoryBarrier(_image.getImage(), range, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, new_layout, VK_ACCESS_TRANSFER_WRITE_BIT, new_access);
            cmd.pipelineBarrier(barrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);


            _layout = new_layout;

        }

        uint32_t Texture::calcMipLevelCount(uint32_t width, uint32_t height) {

            uint32_t max_dim = std::max(width, height);
            uint32_t mip_levels = 1;

            while(max_dim /= 2) mip_levels++;

            return mip_levels;
        }


    } // vulkan

} // undicht