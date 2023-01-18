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

            std::vector<uint32_t> _pool_sizes;
            std::vector<DescriptorSetLayout> _layouts;
            std::vector<DescriptorPool> _descriptor_pools;
            std::vector<std::vector<DescriptorSet>> _descriptor_sets;
            std::vector<uint32_t> _unused_sets;

            
            LogicalDevice _device_handle;

        public:

            void init(const LogicalDevice& device, const std::vector<DescriptorSetLayout>& layouts, const std::vector<uint32_t>& pool_sizes);
            void cleanUp();

            void reset(const std::vector<uint32_t>& slots);
            DescriptorSet& accquire(uint32_t slot);

        };

    } // vulkan

} // undicht

#endif // DESCRIPTOR_SET_CACHE