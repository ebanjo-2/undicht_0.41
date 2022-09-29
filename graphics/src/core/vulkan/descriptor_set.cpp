#include "descriptor_set.h"

namespace undicht {

    namespace vulkan {

        void DescriptorSet::init(const VkDevice& device) {

            _device_handle = device;

        }

        void DescriptorSet::cleanUp() {

        }

        ////////////////////////////// creating descriptor set related structs //////////////////////////////

        

    } // vulkan

} // undicht