#include "descriptor_set_cache.h"
#include "debug.h"

namespace undicht {

    namespace vulkan {

        void DescriptorSetCache::init(const LogicalDevice& device, const std::vector<DescriptorSetLayout>& layouts, const std::vector<uint32_t>& pool_sizes) {
            
            _device_handle = device;

            _layouts = layouts;
            _pool_sizes = pool_sizes;

            _descriptor_pools.resize(layouts.size());
            _descriptor_sets.resize(layouts.size());
            _unused_sets.resize(layouts.size());

            for(int i = 0; i < layouts.size(); i++) {
                _descriptor_pools.at(i).init(device.getDevice(), _layouts.at(i).getDescriptorTypes(), _pool_sizes.at(i));
            }

        }

        void DescriptorSetCache::cleanUp() {

            for(DescriptorPool& pool : _descriptor_pools)
                pool.cleanUp();

        }

        void DescriptorSetCache::reset(const std::vector<uint32_t>& slots) {

            for(int slot : slots) {
                // marking the first set as "unused"
                _unused_sets.at(slot) = 0;
            }

        }

        DescriptorSet& DescriptorSetCache::accquire(uint32_t slot) {
            
            if(_unused_sets.at(slot) == _descriptor_sets.at(slot).size()) {
                // need to create more descriptor sets

                DescriptorSet set;
                set.init(_device_handle.getDevice(), _descriptor_pools.at(slot).getDescriptorPool(), _layouts.at(slot).getLayout());
                _descriptor_sets.at(slot).push_back(set);
            }

            // return a currently unused set
            uint32_t set = _unused_sets.at(slot)++;
            return _descriptor_sets.at(slot).at(set);;
        }

    } // vulkan

} // undicht