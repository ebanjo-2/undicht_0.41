#include "pipeline.h"
#include "debug.h"
#include "core/vulkan/formats.h"

namespace undicht {

    namespace vulkan {

        void Pipeline::setShaderStages(const std::vector<VkShaderModule> &modules, const std::vector<VkShaderStageFlagBits>& stages) {

            for(int i = 0; i < modules.size(); i++) {

                const VkShaderModule& module = modules[i];
                const VkShaderStageFlagBits& stage = stages[i];
                _shader_stages.push_back(createPipelineShaderStageCreateInfo(stage, module));
            }

        }

        void Pipeline::addVertexBinding(uint32_t binding, uint32_t total_stride) {
            /** @param binding: binding 0 is reserved for per vertex data, binding 1 for per Instance data */

            if(binding == 0)
                _vertex_input_bindings.push_back(createVertexInputBindingDescription(binding, total_stride, VK_VERTEX_INPUT_RATE_VERTEX));

            if(binding == 1)
                _vertex_input_bindings.push_back(createVertexInputBindingDescription(binding, total_stride, VK_VERTEX_INPUT_RATE_INSTANCE));

        }


        void Pipeline::addVertexAttribute(uint32_t binding, uint32_t location, uint32_t offset, VkFormat format) {
        /** @param binding: binding 0 is reserved for per vertex data, binding 1 for per Instance data
         * @param location: the location under which the attribute is accessed in the shader
         * @param offset: combined size of the vertex attributes in front of this one */

            _vertex_attributes.push_back(createVertexInputAttributeDescription(binding, location, offset, format));

        }


        void Pipeline::setInputAssembly(VkPrimitiveTopology topology) {

            _input_assembly_state = createPipelineInputAssembleStateCreateInfo(topology);

        }

        void Pipeline::setViewport(const VkExtent2D &extent) {

            _viewport = createViewport(extent);
            _scissor = createScissor(extent);
            _viewport_state = createPipelineViewportStateCreateInfo(_viewport, _scissor);

        }

        void Pipeline::setRasterizationState(bool enable_culling, bool cull_ccw_faces, bool wire_frame) {

            VkPolygonMode polygon_mode = wire_frame?VK_POLYGON_MODE_LINE:VK_POLYGON_MODE_FILL;
            VkFrontFace front_face = cull_ccw_faces?VK_FRONT_FACE_CLOCKWISE:VK_FRONT_FACE_COUNTER_CLOCKWISE; // instead of declaring what faces to cull (cw or ccw) the front face gets declared
            _rasterization_state = createPipelineRasterizationStateCreateInfo(polygon_mode, enable_culling, front_face);

        }

        /* void Pipeline::setMultisampleState(uint32_t samples) {

        }*/

        void Pipeline::setBlending(uint32_t attachment, bool enable_blending, VkBlendOp color_blend_op, VkBlendFactor src_color_factor, VkBlendFactor dst_color_factor, VkBlendOp alpha_blend_op, VkBlendFactor src_alpha_factor, VkBlendFactor dst_alpha_factor) {
            // the blend state for every attachment needs to be set in the order the attachments are in (yes, laziness)

            if(attachment >= _blend_attachments.size()) _blend_attachments.resize(attachment + 1);

            _blend_attachments.at(attachment) = createPipelineColorBlendAttachmentState(enable_blending, color_blend_op, src_color_factor, dst_color_factor, alpha_blend_op, src_alpha_factor, dst_alpha_factor);
        }

        void Pipeline::setShaderInput(const VkDescriptorSetLayout& layout, uint32_t slot) {

            if(slot >= _descriptor_set_layouts.size())
                _descriptor_set_layouts.resize(slot + 1);

            _descriptor_set_layouts.at(slot) = layout;
        }

        void Pipeline::setDepthStencilState(bool enable_depth_test, bool write_depth_values, VkCompareOp compare_op) {

            _depth_stencil_state = createPipelineDepthStencilStateCreateInfo(enable_depth_test, write_depth_values, compare_op);
        }


        void Pipeline::init(const VkDevice& device, VkRenderPass render_pass, uint32_t subpass) {

            _device_handle = device;

            // creating structs that describe combined states
            _vertex_input_state = createPipelineVertexInputStateCreateInfo(_vertex_input_bindings, _vertex_attributes);
            _color_blend_state = createPipelineColorBlendStateCreateInfo(_blend_attachments);
            _multisample_state = createPipelineMultisampleStateCreateInfo();

            // creating the pipeline layout
            VkPipelineLayoutCreateInfo layout_info = createPipelineLayoutCreateInfo(_descriptor_set_layouts);
            vkCreatePipelineLayout(device, &layout_info, {}, &_layout);

            // joining all pipeline info structs
            VkGraphicsPipelineCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            info.pNext = nullptr;

            info.stageCount = _shader_stages.size();
            info.pStages = _shader_stages.data();
            info.pVertexInputState = &_vertex_input_state;
            info.pInputAssemblyState = &_input_assembly_state;
            info.pViewportState = &_viewport_state;
            info.pRasterizationState = &_rasterization_state;
            info.pMultisampleState = &_multisample_state;
            info.pColorBlendState = &_color_blend_state;
            info.pDepthStencilState = &_depth_stencil_state;
            info.layout = _layout;
            info.renderPass = render_pass;
            info.subpass = subpass;
            info.basePipelineHandle = VK_NULL_HANDLE;
            info.layout = _layout;

            if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, {}, &_pipeline) != VK_SUCCESS)
                UND_ERROR << "failed to create graphics pipeline\n";

        }

        void Pipeline::cleanUp() {

            vkDestroyPipelineLayout(_device_handle, _layout, {});
            vkDestroyPipeline(_device_handle, _pipeline, {});
        }

        const VkViewport& Pipeline::getViewport() const {

            return _viewport;
        }

        const VkPipeline& Pipeline::getPipeline() const {

            return _pipeline;
        }

        const VkPipelineLayout& Pipeline::getPipelineLayout() const {
            
            return _layout;
        }

        const VkDescriptorSetLayout& Pipeline::getDescriptorSetLayout(uint32_t slot) const {

            if(slot >= _descriptor_set_layouts.size()) {
                UND_ERROR << "failed to return descriptor set layout " << slot << "\n";
                return VK_NULL_HANDLE;
            }

            return _descriptor_set_layouts.at(slot);
        }


            // a function that allows to set the entire layout of a vertex binding
        void Pipeline::setVertexBinding(uint32_t id, uint32_t location_offset, const undicht::BufferLayout& layout) {

            uint32_t total_size = layout.getTotalSize();
            addVertexBinding(id, total_size);

            uint32_t current_offset = 0;
            uint32_t attribute_id = location_offset;
            for(const undicht::FixedType& t : layout.m_types) {

                addVertexAttribute(id, attribute_id, current_offset, vulkan::translate(t)); // position

                current_offset += t.getSize();
                attribute_id += 1;
            }
        }

        /////////////////////////////////// creating pipeline related structs ///////////////////////////////////

        VkPipelineShaderStageCreateInfo Pipeline::createPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule module) {
            // information about a shader stage (i.e. the vertex shader stage)

            VkPipelineShaderStageCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            info.pNext = nullptr;
            info.stage = stage;
            info.module = module;
            info.pName = "main"; // entry point
            return info;
        }

        VkVertexInputBindingDescription Pipeline::createVertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate input_rate) {
            /** describes a buffer that provides vertex data
            * @param stride: total size of the per vertex data
            * @param input_rate: the data can be updated per vertex or per instance */

            VkVertexInputBindingDescription info{};
            info.binding = binding;
            info.inputRate = input_rate;
            info.stride = stride;

            return info;
        }

        VkVertexInputAttributeDescription Pipeline::createVertexInputAttributeDescription(uint32_t binding, uint32_t location, uint32_t offset, VkFormat format) {
            /** describes a single attribute of a vertex */

            VkVertexInputAttributeDescription info{};
            info.binding = binding;
            info.format = format;
            info.location = location;
            info.offset = offset;

            return info;
        }

        VkPipelineVertexInputStateCreateInfo Pipeline::createPipelineVertexInputStateCreateInfo(const std::vector<VkVertexInputBindingDescription>& vertex_input_bindings, const std::vector<VkVertexInputAttributeDescription>& vertex_input_attributes) {
            // combined information about how the vertex data gets acquired

            VkPipelineVertexInputStateCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            info.pNext = nullptr;
            info.pVertexBindingDescriptions = vertex_input_bindings.data();
            info.vertexBindingDescriptionCount = vertex_input_bindings.size();
            info.pVertexAttributeDescriptions = vertex_input_attributes.data();
            info.vertexAttributeDescriptionCount = vertex_input_attributes.size();

            return info;
        }

        VkPipelineInputAssemblyStateCreateInfo Pipeline::createPipelineInputAssembleStateCreateInfo(VkPrimitiveTopology topology) {
            // info about what how the vertices get assembled info primitives
            // like triangles, points, lines, ...

            VkPipelineInputAssemblyStateCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            info.pNext = nullptr;
            info.topology = topology;
            info.primitiveRestartEnable = VK_FALSE;

            return info;
        }

        VkPipelineRasterizationStateCreateInfo Pipeline::createPipelineRasterizationStateCreateInfo(VkPolygonMode polygon_mode, bool enable_culling, VkFrontFace front_face) {
            // info about the rasterization stage
            // use VK_POLYGON_MODE_LINE for wireframe mode

            VkPipelineRasterizationStateCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            info.pNext = nullptr;
            info.depthClampEnable = VK_FALSE;
            info.rasterizerDiscardEnable = VK_FALSE;
            info.polygonMode = polygon_mode;
            info.lineWidth = 1.0f;
            info.cullMode = enable_culling?VK_CULL_MODE_BACK_BIT:VK_CULL_MODE_NONE;
            info.frontFace = front_face;
            info.depthBiasEnable = VK_FALSE;
            info.depthBiasConstantFactor = 0.0f;
            info.depthBiasClamp = 0.0f;
            info.depthBiasSlopeFactor = 0.0f;

            return info;
        }

        VkPipelineMultisampleStateCreateInfo Pipeline::createPipelineMultisampleStateCreateInfo() {
            // for now: no multisampling

            VkPipelineMultisampleStateCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            info.pNext = nullptr;
            info.sampleShadingEnable = VK_FALSE;
            info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            info.minSampleShading = 1.0f;
            info.pSampleMask = nullptr;
            info.alphaToCoverageEnable = VK_FALSE;
            info.alphaToOneEnable = VK_FALSE;

            return info;
        }

        VkPipelineColorBlendAttachmentState Pipeline::createPipelineColorBlendAttachmentState(bool enable_blending, VkBlendOp color_blend_op, VkBlendFactor src_color_factor, VkBlendFactor dst_color_factor, VkBlendOp alpha_blend_op, VkBlendFactor src_alpha_factor, VkBlendFactor dst_alpha_factor) {
            // info about how to blend into an attachment

            VkPipelineColorBlendAttachmentState blend_state{};
            blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            blend_state.blendEnable = enable_blending;
            blend_state.colorBlendOp = color_blend_op;
            blend_state.alphaBlendOp = alpha_blend_op;
            blend_state.srcColorBlendFactor = src_color_factor;
            blend_state.dstColorBlendFactor = dst_color_factor;
            blend_state.srcAlphaBlendFactor = src_alpha_factor;
            blend_state.dstAlphaBlendFactor = dst_alpha_factor;

            return blend_state;
        }

        VkPipelineColorBlendStateCreateInfo Pipeline::createPipelineColorBlendStateCreateInfo(const std::vector<VkPipelineColorBlendAttachmentState>& blend_attachments) {
            // combines the blend operations for all attachments

            VkPipelineColorBlendStateCreateInfo color_blending{};
            color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blending.pNext = nullptr;
            color_blending.logicOpEnable = VK_FALSE;
            color_blending.logicOp = VK_LOGIC_OP_COPY;
            color_blending.attachmentCount = blend_attachments.size();
            color_blending.pAttachments = blend_attachments.data();

            return color_blending;
        }

        VkPipelineDepthStencilStateCreateInfo Pipeline::createPipelineDepthStencilStateCreateInfo(bool depth_test, bool write_depth_values, VkCompareOp compare_op) {
            
            VkPipelineDepthStencilStateCreateInfo depth_stencil{};
            depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil.depthTestEnable = depth_test;
            depth_stencil.depthWriteEnable = write_depth_values;
            depth_stencil.depthCompareOp = compare_op;
            depth_stencil.depthBoundsTestEnable = VK_FALSE;
            depth_stencil.stencilTestEnable = VK_FALSE;

            return depth_stencil;
        }

        VkViewport Pipeline::createViewport(const VkExtent2D& extent) {

            VkViewport viewport{};
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = extent.width;
            viewport.height = extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            return viewport;
        }

        VkRect2D Pipeline::createScissor(const VkExtent2D& extent) {

            VkRect2D scissor{};
            scissor.extent = extent;
            scissor.offset.x = 0;
            scissor.offset.y = 0;

            return scissor;
        }


        VkPipelineViewportStateCreateInfo Pipeline::createPipelineViewportStateCreateInfo(const VkViewport& viewport, const VkRect2D& scissor) {

            VkPipelineViewportStateCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            info.pNext = nullptr;
            info.viewportCount = 1;
            info.pViewports = &viewport;
            info.scissorCount = 1;
            info.pScissors = &scissor;

            return info;
        }

        /*VkDescriptorSetLayoutBinding Pipeline::createDescriptorSetLayoutBinding(VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags) {
            // describes a binding for an ubo or texture into a shader

            VkDescriptorSetLayoutBinding binding{};
            binding.descriptorCount = 1;
            binding.descriptorType = descriptor_type;
            binding.stageFlags = stage_flags;

            return binding;
        }*/

        /*VkDescriptorSetLayoutCreateInfo Pipeline::createDescriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
            // describes the binding points of the shaders for ubos and textures

            VkDescriptorSetLayoutCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;
            info.bindingCount = bindings.size();
            info.pBindings = bindings.data();

            return info;
        }*/

        VkPipelineLayoutCreateInfo Pipeline::createPipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout>& layouts) {

            VkPipelineLayoutCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;
            info.pushConstantRangeCount = 0;
            info.pPushConstantRanges = nullptr;
            info.setLayoutCount = layouts.size();
            info.pSetLayouts = layouts.data();

            /*if(layout != VK_NULL_HANDLE) {
                info.setLayoutCount = 1;
                info.pSetLayouts = &layout;
            }*/

            return info;
        }

    } // vulkan

} // undicht