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

            class PerFrameData {
            
              protected:

                std::vector<std::vector<DescriptorSet>> _descriptor_sets;
                std::vector<uint32_t> _unused_sets;

              public:

                PerFrameData(uint32_t num_layouts) {
                    _descriptor_sets.resize(num_layouts);
                    _unused_sets.resize(num_layouts);
                }

                DescriptorSet* accquire(uint32_t slot) {
                    // tries to find an unused set
                    if(_unused_sets.at(slot) < _descriptor_sets.at(slot).size()) {
                        return &_descriptor_sets.at(slot).at(_unused_sets.at(slot)++); // increment after getting the current unused set
                    } else {
                        return nullptr;
                    }
                }

                void storeSet(uint32_t slot, DescriptorSet& set) {

                    _descriptor_sets.at(slot).push_back(set);
                }

                void reset(const std::vector<uint32_t>& slots) {

                    for(int slot : slots) {
                        // marking the first set as "unused"
                        _unused_sets.at(slot) = 0;
                    }
                }

            };

        protected:

            std::vector<uint32_t> _pool_sizes;
            std::vector<DescriptorSetLayout> _layouts;
            std::vector<DescriptorPool> _descriptor_pools;
            /*std::vector<std::vector<DescriptorSet>> _descriptor_sets;
            std::vector<uint32_t> _unused_sets;*/

            std::vector<PerFrameData> _per_frame_data;

            LogicalDevice _device_handle;

        public:

            void init(const LogicalDevice& device, const std::vector<DescriptorSetLayout>& layouts, const std::vector<uint32_t>& pool_sizes, uint32_t num_frames);
            void cleanUp();

            void reset(const std::vector<uint32_t>& slots, uint32_t frame);
            DescriptorSet& accquire(uint32_t slot, uint32_t frame);

        };

    } // vulkan

} // undicht

#endif // DESCRIPTOR_SET_CACHE