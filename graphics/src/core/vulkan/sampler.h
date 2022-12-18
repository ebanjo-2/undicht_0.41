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
            VkSamplerMipmapMode _mip_map_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        public:

            // set these settings before initializing the sampler
            void setMinFilter(VkFilter filter);
            void setMaxFilter(VkFilter filter);
            void setMipMapMode(VkSamplerMipmapMode mode);

            void init(const VkDevice& device);
            void cleanUp();

            const VkSampler& getSampler() const;

        protected:
            // creating sampler related structs

            VkSamplerCreateInfo static createSamplerCreateInfo(VkFilter min_filter, VkFilter max_filter, VkSamplerMipmapMode mip_map_mode, bool anisotropy, float max_anisotropy);

        };

    } // vulkan

} // undicht

#endif // SAMPLER_H