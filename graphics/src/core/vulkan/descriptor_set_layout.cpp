#include "descriptor_set_layout.h"

namespace undicht {

    namespace vulkan {

        void DescriptorSetLayout::setBinding(uint32_t binding, const VkDescriptorType& type) {

            if(_bindings.size() <= binding) {
                _bindings.resize(binding + 1, {});
                _binding_types.resize(binding + 1, {});
            } 

            // determining the shader stage
            VkShaderStageFlagBits shader_stage;
            if(type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            else if(type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
                shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            else if(type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                shader_stage = VK_SHADER_STAGE_ALL_GRAPHICS;

            _bindings.at(binding) = createDescriptorSetLayoutBinding(binding, type, shader_stage);
            _binding_types.at(binding) = type;
        }

        void DescriptorSetLayout::init(const VkDevice& device) {
            
            _device_handle = device;


            VkDescriptorSetLayoutCreateInfo info = createDescriptorSetLayoutCreateInfo(_bindings);
            vkCreateDescriptorSetLayout(device, &info, {}, &_layout);

        }

        void DescriptorSetLayout::cleanUp() {
            
            vkDestroyDescriptorSetLayout(_device_handle, _layout, {});

        }

        const VkDescriptorSetLayout& DescriptorSetLayout::getLayout() const {

            return _layout;
        }

        const std::vector<VkDescriptorType>& DescriptorSetLayout::getDescriptorTypes() const {

            return _binding_types;
        }

        /////////////////////////// creating DescriptorSetLayout related structs ///////////////////////////

        VkDescriptorSetLayoutBinding DescriptorSetLayout::createDescriptorSetLayoutBinding(uint32_t binding, const VkDescriptorType& type, const VkShaderStageFlagBits& shader_stage) {

            VkDescriptorSetLayoutBinding layout_binding{};
            layout_binding.binding = binding;
            layout_binding.descriptorCount = 1;
            layout_binding.descriptorType = type;
            layout_binding.stageFlags = shader_stage;

            return layout_binding;
        }

        VkDescriptorSetLayoutCreateInfo DescriptorSetLayout::createDescriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& bindings) {

            VkDescriptorSetLayoutCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            info.pNext = nullptr;
            info.bindingCount = bindings.size();
            info.pBindings = bindings.data();

            return info;
        }


    } // vulkan 

} // undicht