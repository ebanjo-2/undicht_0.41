#include "sampler.h"

namespace undicht {

    namespace vulkan {

        void Sampler::setMinFilter(VkFilter filter) {
            
            _min_filter = filter;
        }

        void Sampler::setMaxFilter(VkFilter filter) {

            _max_filter = filter;
        }

        void Sampler::setMipMapMode(VkSamplerMipmapMode mode) {
            
            _mip_map_mode = mode;
        }


        void Sampler::init(const VkDevice& device) {

            _device_handle = device;

            VkSamplerCreateInfo info = createSamplerCreateInfo(_min_filter, _max_filter, _mip_map_mode, false, 1.0f);
            vkCreateSampler(_device_handle, &info, {}, &_sampler);

        }

        void Sampler::cleanUp() {
            
            vkDestroySampler(_device_handle, _sampler, {});

        }

        const VkSampler& Sampler::getSampler() const {

            return _sampler;
        }

        //////////////////////////// creating sampler related structs /////////////////////////////

        VkSamplerCreateInfo Sampler::createSamplerCreateInfo(VkFilter min_filter, VkFilter max_filter, VkSamplerMipmapMode mip_map_mode, bool anisotropy, float max_anisotropy) {
            
            VkSamplerCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            info.pNext = nullptr;
            info.magFilter = min_filter;
            info.minFilter = max_filter;
            info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            info.anisotropyEnable = anisotropy;
            info.maxAnisotropy = max_anisotropy;
            info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            info.unnormalizedCoordinates = VK_FALSE; // use uv range of [0,1] instead of the actual pixel size
            info.compareEnable = VK_FALSE;
            info.compareOp = VK_COMPARE_OP_ALWAYS;
            info.mipmapMode = mip_map_mode;
            info.mipLodBias = 0.0f;
            info.minLod = 0.0f;
            info.maxLod = 16.0f; // max number of mip levels

            return info;
        }


    } // vulkan

} // undicht