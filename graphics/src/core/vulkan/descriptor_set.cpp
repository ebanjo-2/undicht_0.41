#include "descriptor_set.h"
#include "debug.h"

namespace undicht {

    namespace vulkan {

        void DescriptorSet::init(const VkDevice& device, const VkDescriptorPool& pool, const VkDescriptorSetLayout& layout) {

            _device_handle = device;

            VkDescriptorSetAllocateInfo info = createDescriptorSetAllocateInfo(pool, layout);
            vkAllocateDescriptorSets(device, &info, &_descriptor_set);
        }

        void DescriptorSet::bindUniformBuffer(uint32_t binding, const Buffer& buffer) {

            VkDescriptorBufferInfo buffer_info = createDescriptorBufferInfo(buffer.getBuffer(), 0, VK_WHOLE_SIZE);

            VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            _pending_writes.push_back(createWriteDescriptorSet(binding, type, _descriptor_set, &buffer_info, nullptr));

            update();
        }

        void DescriptorSet::bindImage(uint32_t binding, const VkImageView& image_view, const VkImageLayout& layout, const VkSampler& sampler) {
            
            VkDescriptorImageInfo image_info = createDescriptorImageInfo(image_view, layout, sampler);

            VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            _pending_writes.push_back(createWriteDescriptorSet(binding, type, _descriptor_set, nullptr, &image_info));

            update();
        }

        void DescriptorSet::bindInputAttachment(uint32_t binding, const VkImageView& image_view) {
            
            VkDescriptorImageInfo image_info = createDescriptorImageInfo(image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_NULL_HANDLE);
            
            VkDescriptorType type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            _pending_writes.push_back(createWriteDescriptorSet(binding, type, _descriptor_set, nullptr, &image_info));

            update();
        }


        const VkDescriptorSet& DescriptorSet::getDescriptorSet() const {
            
            return _descriptor_set;
        }

        void DescriptorSet::update() {
            // aplies all pending bind commands

            vkUpdateDescriptorSets(_device_handle, _pending_writes.size(), _pending_writes.data(), 0, nullptr);
            _pending_writes.clear();

        } 


        ////////////////////////////// creating descriptor set related structs //////////////////////////////

        VkDescriptorSetAllocateInfo DescriptorSet::createDescriptorSetAllocateInfo(const VkDescriptorPool& pool, const VkDescriptorSetLayout& layout) {

            VkDescriptorSetAllocateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            info.pNext = nullptr;
            info.descriptorSetCount = 1;
            info.descriptorPool = pool;
            info.pSetLayouts = &layout;

            return info;
        }

        VkDescriptorBufferInfo DescriptorSet::createDescriptorBufferInfo(const VkBuffer& buffer, uint32_t offset, VkDeviceSize range) {

            VkDescriptorBufferInfo info{};
            info.buffer = buffer;
            info.offset = offset;
            info.range = range;

            return info;
        }

        VkDescriptorImageInfo DescriptorSet::createDescriptorImageInfo(const VkImageView& image_view, VkImageLayout layout, VkSampler sampler) {

            VkDescriptorImageInfo info{};
            info.imageView = image_view;
            info.imageLayout = layout;
            info.sampler = sampler;

            return info;
        }

        VkWriteDescriptorSet DescriptorSet::createWriteDescriptorSet(uint32_t binding, VkDescriptorType type, VkDescriptorSet set, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info) {

            VkWriteDescriptorSet write_info{};
            write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_info.pNext = nullptr;
            write_info.descriptorCount = 1;
            write_info.descriptorType = type;
            write_info.dstSet = set;
            write_info.dstBinding = binding;
            write_info.pBufferInfo = buffer_info;
            write_info.pImageInfo = image_info;

            return write_info;
        }


    } // vulkan

} // undicht