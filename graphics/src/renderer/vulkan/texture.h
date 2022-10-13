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

            VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB;
            VkImageLayout _layout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkAccessFlags _access_flags = VK_ACCESS_NONE;

            CommandBuffer _copy_cmd;
            CommandBuffer _layout_cmd;
            Buffer _transfer_buffer;

        public:

            // set the settings before initializing the texture
            void setExtent(uint32_t width, uint32_t height, uint32_t depth = 1);

            void setFormat(VkFormat format);

            void init(const LogicalDevice& device);
            void cleanUp();

            const Image& getImage() const;
            const VkImageLayout& getLayout() const;

            void setData(const char* data, uint32_t byte_size);

        protected:
            // internal functions

            void transitionToLayout(VkImageLayout new_layout, VkAccessFlags new_access, VkPipelineStageFlagBits src_stage, VkPipelineStageFlagBits dst_stage);

        };

    } // vulkan

} // undicht

#endif // TEXTURE_H