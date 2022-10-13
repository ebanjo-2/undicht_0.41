#ifndef PIPE_LINE_H
#define PIPE_LINE_H

#include "vector"
#include "vulkan/vulkan.h"

namespace undicht {

    namespace vulkan {

        class Pipeline {

        protected:

            VkDevice _device_handle;

            // pipeline info structs
            std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;
            std::vector<VkVertexInputBindingDescription> _vertex_input_bindings;
            std::vector<VkVertexInputAttributeDescription> _vertex_attributes;
            VkPipelineVertexInputStateCreateInfo _vertex_input_state;
            VkPipelineInputAssemblyStateCreateInfo _input_assembly_state;
            VkViewport _viewport;
            VkRect2D _scissor;
            VkPipelineViewportStateCreateInfo _viewport_state;
            VkPipelineRasterizationStateCreateInfo _rasterization_state;
            VkPipelineMultisampleStateCreateInfo _multisample_state;
            std::vector<VkPipelineColorBlendAttachmentState> _blend_attachments;
            VkPipelineColorBlendStateCreateInfo _color_blend_state;

            VkPipeline _pipeline;
            VkPipelineLayout _layout; // contains info about the input to the shaders (ubo and texture bindings, push constants)
            VkDescriptorSetLayout _descriptor_set_layout;

        public:

            // configuring the pipeline
            void setShaderStages(const std::vector<VkShaderModule>& modules, const std::vector<VkShaderStageFlagBits>& stages);
            void addVertexBinding(uint32_t binding, uint32_t total_stride);
            void addVertexAttribute(uint32_t binding, uint32_t location, uint32_t offset, VkFormat format);
            void setInputAssembly(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            void setViewport(const VkExtent2D& extent);
            void setRasterizationState(bool enable_culling, bool cull_ccw_faces = true, bool wire_frame = false);
            // void setMultisampleState(uint32_t samples);
            void setBlending(uint32_t attachment, bool enable_blending, VkBlendOp color_blend_op = {}, VkBlendOp alpha_blend_op = {});
            void setShaderInput(const VkDescriptorSetLayout& layout);

            void init(const VkDevice& device, VkRenderPass render_pass);
            void cleanUp();

            const VkPipeline& getPipeline() const;
            const VkPipelineLayout& getPipelineLayout() const;

        protected:
            // creating pipeline related structs

            VkPipelineShaderStageCreateInfo static createPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule module);
            VkVertexInputBindingDescription static createVertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate input_rate = VK_VERTEX_INPUT_RATE_VERTEX);
            VkVertexInputAttributeDescription static createVertexInputAttributeDescription(uint32_t binding, uint32_t location, uint32_t offset, VkFormat format);
            VkPipelineVertexInputStateCreateInfo static createPipelineVertexInputStateCreateInfo(const std::vector<VkVertexInputBindingDescription>& vertex_input_bindings, const std::vector<VkVertexInputAttributeDescription>& vertex_input_attributes);
            VkPipelineInputAssemblyStateCreateInfo static createPipelineInputAssembleStateCreateInfo(VkPrimitiveTopology topology);
            VkPipelineRasterizationStateCreateInfo static createPipelineRasterizationStateCreateInfo(VkPolygonMode polygon_mode = VK_POLYGON_MODE_FILL, bool enable_culling = false, VkFrontFace front_face = VK_FRONT_FACE_CLOCKWISE);
            VkPipelineMultisampleStateCreateInfo static createPipelineMultisampleStateCreateInfo();
            VkPipelineColorBlendAttachmentState static createPipelineColorBlendAttachmentState(bool enable_blending = false, VkBlendOp color_blend_op = {}, VkBlendOp alpha_blend_op = {});
            VkPipelineColorBlendStateCreateInfo static createPipelineColorBlendStateCreateInfo(const std::vector<VkPipelineColorBlendAttachmentState>& blend_attachments);
            VkViewport static createViewport(const VkExtent2D& extent);
            VkRect2D static createScissor(const VkExtent2D& extent);
            VkPipelineViewportStateCreateInfo static createPipelineViewportStateCreateInfo(const VkViewport& viewport, const VkRect2D& scissor);
            VkDescriptorSetLayoutBinding static createDescriptorSetLayoutBinding(VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags = VK_SHADER_STAGE_ALL_GRAPHICS);
            VkDescriptorSetLayoutCreateInfo static createDescriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& bindings = {});
            VkPipelineLayoutCreateInfo static createPipelineLayoutCreateInfo(const VkDescriptorSetLayout& layout = VK_NULL_HANDLE);

        };

    } // vulkan

} // undicht

#endif // PIPE_LINE_H