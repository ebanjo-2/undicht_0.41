#ifndef DESCRIPTOR_SET_CACHE
#define DESCRIPTOR_SET_CACHE

#include "vector"
#include "core/vulkan/descriptor_set.h"
#include "core/vulkan/descriptor_pool.h"
#include "core/vulkan/logical_device.h"

namespace undicht {

    namespace vulkan {

        class DescriptorSetCache {
            // after being used in a draw command descriptor sets may not be modified until the draw command has finished execution
            // this class is designed to make it easier to reuse descriptor sets
            // and to accquire new descriptor sets when needed

        protected:

            std::vector<DescriptorPool> _descriptor_pools;
            std::vector<DescriptorSet> _descriptor_sets;
            uint32_t _unused_set = 0;

            DescriptorSetLayout _layout;
            LogicalDevice _device_handle;

        public:

            void init(const LogicalDevice& device, const DescriptorSetLayout& layout);
            void cleanUp();

            void reset();
            DescriptorSet& accquire();

        };

    } // vulkan

} // undicht

#endif // DESCRIPTOR_SET_CACHE