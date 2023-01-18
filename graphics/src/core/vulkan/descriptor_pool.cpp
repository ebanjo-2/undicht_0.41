#include "descriptor_pool.h"

#include "vector"

namespace undicht {

    namespace vulkan {

        void DescriptorPool::init(const VkDevice& device, const std::vector<VkDescriptorType> descriptor_types, uint32_t pool_size) {
            
            _device_handle = device;

            // all descriptor types with the number of times they appear in the descriptor_types vector
            // taken from https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
            std::vector<std::pair<VkDescriptorType, uint32_t>> descriptor_frequency = {
                {VK_DESCRIPTOR_TYPE_SAMPLER, 0},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 0},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 0},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0}};

            // counting the descriptor types
            for (const VkDescriptorType& descriptor_type : descriptor_types)
            {
                for (std::pair<VkDescriptorType, uint32_t>& entry : descriptor_frequency)
                {
                    if (entry.first == descriptor_type)
                        entry.second++;
                }
            }

            // creating the descriptor pool
            std::vector<VkDescriptorPoolSize> pool_sizes;

            for (const std::pair<VkDescriptorType, float>& entry : descriptor_frequency)
            {
                if (entry.second != 0)
                    pool_sizes.push_back(createDescriptorPoolSize(pool_size * entry.second, entry.first));
            }

            if((pool_sizes.size() * pool_size) == 0) return;

            VkDescriptorPoolCreateInfo info = createDescriptorPoolCreateInfo(pool_size, pool_sizes);
            vkCreateDescriptorPool(device, &info, {}, &_descriptor_pool);
        }

        void DescriptorPool::cleanUp() {
            // passing VK_NULL_HANDLE should be fine
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

    }  // namespace vulkan

}  // namespace undicht