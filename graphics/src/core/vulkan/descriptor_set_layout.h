#ifndef DESCRIPTOR_SET_LAYOUT_H
#define DESCRIPTOR_SET_LAYOUT_H

#include "vulkan/vulkan.h"
#include "vector"

namespace undicht {

    namespace vulkan {

        class DescriptorSetLayout {

        protected:

            VkDevice _device_handle;

            std::vector<VkDescriptorSetLayoutBinding> _bindings;
            VkDescriptorSetLayout _layout;

        public:

            // set all bindings before calling init()
            void setBinding(uint32_t binding, const VkDescriptorType& type);

            void init(const VkDevice& device);
            void cleanUp();

            const VkDescriptorSetLayout& getLayout() const;

        protected:
            // creating DescriptorSetLayout related structs

            VkDescriptorSetLayoutBinding static createDescriptorSetLayoutBinding(uint32_t binding, const VkDescriptorType& type, const VkShaderStageFlagBits& shader_stage);
            VkDescriptorSetLayoutCreateInfo static createDescriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
        };


    } // vulkan
    
} // undicht


#endif // DESCRIPTOR_SET_LAYOUT_H