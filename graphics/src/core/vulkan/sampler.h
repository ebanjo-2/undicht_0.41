#ifndef SAMPLER_H
#define SAMPLER_H

#include "vulkan/vulkan.h"

namespace undicht {

    namespace vulkan {

        class Sampler {

        protected:

            VkDevice _device_handle;
            VkSampler _sampler;

            VkFilter _min_filter = VK_FILTER_LINEAR;
            VkFilter _max_filter = VK_FILTER_LINEAR;
            VkSamplerAddressMode _repeat_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            VkBorderColor _border_color = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
            VkSamplerMipmapMode _mip_map_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        public:

            // set these settings before initializing the sampler
            void setMinFilter(VkFilter filter);
            void setMaxFilter(VkFilter filter);
            void setRepeatMode(VkSamplerAddressMode mode);
            void setBorderColor(VkBorderColor color);
            void setMipMapMode(VkSamplerMipmapMode mode);

            void init(const VkDevice& device);
            void cleanUp();

            const VkSampler& getSampler() const;

        protected:
            // creating sampler related structs

            VkSamplerCreateInfo static createSamplerCreateInfo(VkFilter min_filter, VkFilter max_filter, VkSamplerAddressMode repeat_mode, VkBorderColor border_color, VkSamplerMipmapMode mip_map_mode, bool anisotropy, float max_anisotropy);

        };

    } // vulkan

} // undicht

#endif // SAMPLER_H