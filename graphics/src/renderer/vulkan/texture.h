#ifndef TEXTURE_H
#define TEXTURE_H

#include "core/vulkan/image.h"
#include "core/vulkan/logical_device.h"
#include "core/vulkan/command_buffer.h"
#include "core/vulkan/buffer.h"

namespace undicht {

    namespace vulkan {

        class Texture {
        
        protected:

            const LogicalDevice* _device_handle = 0;

            Image _image;

            uint32_t _width = 0;
            uint32_t _height = 0;
            uint32_t _depth = 0;
            uint32_t _layers = 1;
            uint32_t _mip_levels = -1;
            
            bool _enable_mip_maps = false;
            bool _auto_gen_mip_maps = false;
            bool _is_cube_map = false;

            VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB;
            VkImageLayout _layout = VK_IMAGE_LAYOUT_UNDEFINED;

            CommandBuffer _copy_cmd;
            CommandBuffer _layout_cmd;
            Buffer _transfer_buffer;

        public:

            // set the settings before initializing the texture
            void setExtent(uint32_t width, uint32_t height, uint32_t depth = 1, uint32_t layers = 1);
            void setCubeMap(bool is_cube_map = true); // the texture has to have 6 layers
            void setFormat(VkFormat format);
            /// @param enable_mip_maps allocate additional memory for mip maps
            /// @param auto_generate if the mip maps should be automatically generated after data is stored in the texture
            /// @param mip_levels restrict the number of mip levels that should be allocated 
            void setMipMaps(bool enable_mip_maps, bool auto_generate = true, uint32_t mip_levels = -1);

            void init(const LogicalDevice& device);
            void cleanUp();

            const Image& getImage() const;
            const VkImageLayout& getLayout() const;

            void setData(const char* data, uint32_t byte_size, uint32_t layer = 0, uint32_t mip_level = 0, VkExtent3D data_image_extent = {}, VkOffset3D offset_in_image = {});

        protected:
            // internal functions

            void transitionToLayout(VkImageLayout new_layout, VkAccessFlags src_access, VkAccessFlags dst_access, VkPipelineStageFlagBits src_stage, VkPipelineStageFlagBits dst_stage);
            void genMipMaps(VkImageLayout new_layout, VkAccessFlags new_access);

            uint32_t static calcMipLevelCount(uint32_t width, uint32_t height);
        
        };

    } // vulkan

} // undicht

#endif // TEXTURE_H