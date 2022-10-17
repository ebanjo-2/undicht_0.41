#include "descriptor_set_cache.h"
#include "debug.h"

namespace undicht {

    namespace vulkan {

        void DescriptorSetCache::init(const LogicalDevice& device, const DescriptorSetLayout& layout) {
            
            _layout = layout;
            _device_handle = device;

            _descriptor_pools.resize(1);
            _descriptor_pools[0].init(device.getDevice());

        }

        void DescriptorSetCache::cleanUp() {

            for(DescriptorPool& pool : _descriptor_pools)
                pool.cleanUp();

        }

        void DescriptorSetCache::reset() {

            _unused_set = 0;
        }

        DescriptorSet& DescriptorSetCache::accquire() {
            
            if(_unused_set == _descriptor_sets.size()) {
                // need to create more descriptor sets

                DescriptorSet set;
                set.init(_device_handle.getDevice(), _descriptor_pools.back().getDescriptorPool(), _layout.getLayout());
                _descriptor_sets.push_back(set);
            }

            // return a currently unused set
            return _descriptor_sets.at(_unused_set++);
        }

    } // vulkan

} // undicht