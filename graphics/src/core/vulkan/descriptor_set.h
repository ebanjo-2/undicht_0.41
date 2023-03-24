#ifndef DESCRIPTOR_SET_H
#define DESCRIPTOR_SET_H

#include "vector"
#include "vulkan/vulkan.h"
#include "descriptor_pool.h"
#include "descriptor_set_layout.h"
#include "buffer.h"

namespace undicht {

    namespace vulkan {

        class DescriptorSet {

        protected:

            VkDevice _device_handle;

            VkDescriptorSet _descriptor_set;

            std::vector<VkWriteDescriptorSet> _pending_writes; // pending updates to the descriptor set

        public:

            virtual ~DescriptorSet();

            void init(const VkDevice& device, const VkDescriptorPool& pool, const VkDescriptorSetLayout& layout);
            // doesnt need a cleanUp function, gets destroyed once the pool is destroyed

            // stages the changes, call update to actually apply the changes
            void bindUniformBuffer(uint32_t binding, const Buffer& buffer);
            void bindImage(uint32_t binding, const VkImageView& image_view, const VkImageLayout& layout, const VkSampler& sampler);
            void bindInputAttachment(uint32_t binding, const VkImageView& image_view);

            // aplies all pending bind commands
            void update();

            const VkDescriptorSet& getDescriptorSet() const;

        protected:

            void clearPendingWrites();

        protected:
            // creating descriptor set related structs

            VkDescriptorSetAllocateInfo static createDescriptorSetAllocateInfo(const VkDescriptorPool& pool, const VkDescriptorSetLayout& layout);
            VkDescriptorBufferInfo static createDescriptorBufferInfo(const VkBuffer& buffer, uint32_t offset, VkDeviceSize range);
            VkDescriptorImageInfo static createDescriptorImageInfo(const VkImageView& image_view, VkImageLayout layout, VkSampler sampler);
            VkWriteDescriptorSet static createWriteDescriptorSet(uint32_t binding, VkDescriptorType type, VkDescriptorSet set, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info);

        };
        
    } // vulkan
    
} // undicht

#endif // DESCRIPTOR_SET_H