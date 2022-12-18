#include "descriptor_pool.h"
#include "vector"

namespace undicht {

    namespace vulkan {

        
        // taken from https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
        // typical numbers of descriptors of different types in a descriptor pool
        std::vector<std::pair<VkDescriptorType,float>> descriptor_frequency = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
		};

        void DescriptorPool::init(const VkDevice& device, uint32_t pool_size) {

            _device_handle = device;

            // creating the descriptor pool
            std::vector<VkDescriptorPoolSize> pool_sizes;

            for(const std::pair<VkDescriptorType,float>& descriptor_type : descriptor_frequency)
                pool_sizes.push_back(createDescriptorPoolSize(pool_size * descriptor_type.second, descriptor_type.first));

            VkDescriptorPoolCreateInfo info = createDescriptorPoolCreateInfo(pool_size, pool_sizes);
            vkCreateDescriptorPool(device, &info, {}, &_descriptor_pool);

        }

        void DescriptorPool::cleanUp() {
            
            vkDestroyDescriptorPool(_device_handle, _descriptor_pool, {});

        }

        const VkDescriptorPool& DescriptorPool::getDescriptorPool() const {
            
            return _descriptor_pool;
        }

        ////////////////////////////// creating descriptor pool related structs //////////////////////////////

        VkDescriptorPoolSize DescriptorPool::createDescriptorPoolSize(uint32_t pool_size, VkDescriptorType type) {

            VkDescriptorPoolSize info{};
            info.type = type;
            info.descriptorCount = pool_size;

            return info;
        }

        VkDescriptorPoolCreateInfo DescriptorPool::createDescriptorPoolCreateInfo(uint32_t max_sets, const std::vector<VkDescriptorPoolSize>& pool_sizes) {

            VkDescriptorPoolCreateInfo info{};
	        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	        info.flags = 0;
	        info.maxSets = max_sets;
	        info.poolSizeCount = pool_sizes.size();
	        info.pPoolSizes = pool_sizes.data();

            return info;
        }


    } // vulkan

} // undicht