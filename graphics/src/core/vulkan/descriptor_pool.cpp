#include "descriptor_pool.h"

namespace undicht {

    namespace vulkan {

        void DescriptorPool::init(const VkDevice& device, uint32_t pool_size) {

            _device_handle = device;

        }

        void DescriptorPool::cleanUp() {

        }

        const VkDescriptorPool& DescriptorPool::getDescriptorPool() const {
            
            return _descriptor_pool;
        }

        ////////////////////////////// creating descriptor pool related structs //////////////////////////////

        

    } // vulkan

} // undicht