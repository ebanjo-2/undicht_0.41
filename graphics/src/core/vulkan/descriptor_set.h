#ifndef DESCRIPTOR_SET_H
#define DESCRIPTOR_SET_H

#include "vulkan/vulkan.h"

namespace undicht {

    namespace vulkan {

        class DescriptorSet {

        protected:

            VkDevice _device_handle;

        public:

            void init(const VkDevice& device);
            void cleanUp();

        protected:
            // creating descriptor set related structs

        };
        
    } // vulkan
    
} // undicht

#endif // DESCRIPTOR_SET_H