#ifndef DESCRIPTOR_POOL_H
#define DESCRIPTOR_POOL_H

#include "vector"
#include "vulkan/vulkan.h"

namespace undicht {

    namespace vulkan {

        class DescriptorPool {

        protected:

            VkDevice _device_handle;

            VkDescriptorPool _descriptor_pool;

        public:

            void init(const VkDevice& device, const std::vector<VkDescriptorType> descriptor_types, uint32_t pool_size = 1000);
            void cleanUp(); // will also destroy any descriptor sets allocated from this pool

            const VkDescriptorPool& getDescriptorPool() const;

        protected:
            // creating descriptor pool related structs

            VkDescriptorPoolSize static createDescriptorPoolSize(uint32_t pool_size, VkDescriptorType type);
            VkDescriptorPoolCreateInfo static createDescriptorPoolCreateInfo(uint32_t max_sets, const std::vector<VkDescriptorPoolSize>& pool_sizes);

        };
        
    } // vulkan
    
} // undicht

#endif // DESCRIPTOR_POOL_H