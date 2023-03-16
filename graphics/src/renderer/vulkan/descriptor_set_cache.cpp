#include "descriptor_set_cache.h"
#include "debug.h"

namespace undicht {

    namespace vulkan {

        void DescriptorSetCache::init(const LogicalDevice& device, const std::vector<DescriptorSetLayout>& layouts, const std::vector<uint32_t>& pool_sizes, uint32_t num_frames) {
            
            _device_handle = device;

            _layouts = layouts;
            _pool_sizes = pool_sizes;

            _descriptor_pools.resize(layouts.size());
            _per_frame_data.resize(num_frames, layouts.size());
            /*_descriptor_sets.resize(layouts.size());
            _unused_sets.resize(layouts.size());*/

            for(int i = 0; i < layouts.size(); i++) {
                _descriptor_pools.at(i).init(device.getDevice(), _layouts.at(i).getDescriptorTypes(), _pool_sizes.at(i) * num_frames);
            }

        }

        void DescriptorSetCache::cleanUp() {

            for(DescriptorPool& pool : _descriptor_pools)
                pool.cleanUp();

        }

        void DescriptorSetCache::reset(const std::vector<uint32_t>& slots, uint32_t frame) {

            _per_frame_data.at(frame).reset(slots);

            /*for(int slot : slots) {
                // marking the first set as "unused"
                _unused_sets.at(slot) = 0;
            }*/

        }

        DescriptorSet& DescriptorSetCache::accquire(uint32_t slot, uint32_t frame) {

            DescriptorSet* unused = _per_frame_data.at(frame).accquire(slot);
            if(unused) return *unused; // return an already existing set that is not in use 

            // create a new descriptor set
            DescriptorSet set;
            set.init(_device_handle.getDevice(), _descriptor_pools.at(slot).getDescriptorPool(), _layouts.at(slot).getLayout());
            _per_frame_data.at(frame).storeSet(slot, set);

            return accquire(slot, frame); // should now find a currently unused set (the one that was just created)
        }

    } // vulkan

} // undicht