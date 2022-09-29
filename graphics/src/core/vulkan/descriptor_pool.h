#ifndef DESCRIPTOR_POOL_H
#define DESCRIPTOR_POOL_H

#include "vulkan/vulkan.h"

namespace undicht {

    namespace vulkan {

        class DescriptorPool {

        protected:

            VkDevice _device_handle;

            VkDescriptorPool _descriptor_pool;

        public:

            void init(const VkDevice& device, uint32_t pool_size = 1000);
            void cleanUp(); // will also destroy any descriptor sets allocated from this pool

            const VkDescriptorPool& getDescriptorPool() const;

        protected:
            // creating descriptor pool related structs

        };
        
    } // vulkan
    
} // undicht

#endif // DESCRIPTOR_POOL_H