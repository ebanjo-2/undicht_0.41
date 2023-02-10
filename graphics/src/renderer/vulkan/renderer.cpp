#include "renderer/vulkan/renderer.h"
#include "debug.h"

namespace undicht {

    namespace vulkan {

        const BufferLayout SCREEN_QUAD_VERTEX_LAYOUT({UND_VEC2F, UND_VEC2F});

        const std::vector<float> SCREEN_QUAD_VERTICES = {
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
        };

        const BufferLayout SKY_BOX_VERTEX_LAYOUT({UND_VEC3F});
        // taken from https://learnopengl.com/code_viewer.php?code=advanced/cubemaps_skybox_data
        const std::vector<float> SKY_BOX_VERTICES = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
        };

        /////////////////////////////////// first thing to initialize!!!! ///////////////////////////////////

        void Renderer::setDeviceHandle(const undicht::vulkan::LogicalDevice& gpu) {

            _device_handle = gpu;
        }

        //////////////////////// settings that have(!) to be set before the pipeline is initialized /////////////////

        void Renderer::setShaders(const std::string& vertex_shader, const std::string& fragment_shader) {

            _shader.addVertexModule(_device_handle.getDevice(), vertex_shader);
            _shader.addFragmentModule(_device_handle.getDevice(), fragment_shader);
            _shader.init(_device_handle.getDevice());
        }

        void Renderer::setDescriptorSetLayout(const std::vector<VkDescriptorType>& binding_types, uint32_t slot, uint32_t descriptor_pool_size) {
            /** @brief tell the renderer to use an internal descriptor set layout at the specified slot
             * the renderer will create the layout based on the specified binding_types,
             * will create a descriptor pool for the layout (if descriptor_pool_size > 0)
             * and will also handle the cleanup of the resources 
            */

            DescriptorSetLayout layout;

            for(int i = 0; i < binding_types.size(); i++)
                layout.setBinding(i, binding_types.at(i));

            layout.init(_device_handle.getDevice());
            
            setDescriptorSetLayout(layout, slot, descriptor_pool_size);
            _extern_layouts.at(slot) = false; // a layout that was created by the renderer and which has to be cleaned Up by the renderer
        }
                    
        void Renderer::setDescriptorSetLayout(const undicht::vulkan::DescriptorSetLayout& layout, uint32_t slot, uint32_t descriptor_pool_size) {
            /** @brief tell the renderer to use an extern descriptor set layout at the specified slot
             * i.e. for global descriptors, like a global ubo containing the main camera matrices
             * will create a descriptor pool for the layout (if descriptor_pool_size > 0)
             * will handle the cleanup of the descriptor pool, but not of the external layout
            */

            if(slot <= _descriptor_set_layouts.size()) {
                _descriptor_set_layouts.resize(slot + 1);
                _descriptor_set_pool_sizes.resize(slot + 1);
                _extern_layouts.resize(slot + 1, true);
                _descriptor_sets.resize(slot + 1, nullptr);
            }

            if(_extern_layouts.at(slot) == false) {
                _descriptor_set_layouts.at(slot).cleanUp();
            }

            _extern_layouts.at(slot) = true;
            _descriptor_set_layouts.at(slot) = layout;
            _descriptor_set_pool_sizes.at(slot) = descriptor_pool_size;
            _pipeline.setShaderInput(layout.getLayout(), slot);
        }

        void Renderer::setVertexInputLayout(const undicht::BufferLayout& vertex_layout, const undicht::BufferLayout& instance_layout) {
            
            _pipeline.setVertexBinding(0, 0, vertex_layout);

            if(instance_layout.getTotalSize())
                _pipeline.setVertexBinding(1, vertex_layout.m_types.size(), instance_layout);

        }

        void Renderer::setDepthStencilTest(bool enable_depth_test, bool write_depth_values, VkCompareOp compare_op) {
            
            _pipeline.setDepthStencilState(enable_depth_test, write_depth_values, compare_op);
        }

        void Renderer::setRasterizer(bool enable_culling, bool cull_ccw_faces, bool wire_frame) {

            _pipeline.setRasterizationState(enable_culling, cull_ccw_faces, wire_frame);
        }

        void Renderer::setInputAssembly(VkPrimitiveTopology topology) {

            _pipeline.setInputAssembly(topology);
        }

        void Renderer::setBlending(uint32_t attachment, bool enable_blending, VkBlendOp color_blend_op, VkBlendFactor src_color_factor, VkBlendFactor dst_color_factor, VkBlendOp alpha_blend_op, VkBlendFactor src_alpha_factor, VkBlendFactor dst_alpha_factor) {
            // set the blending for output at binding #attachment
            _pipeline.setBlending(attachment, enable_blending, color_blend_op, src_color_factor, dst_color_factor, alpha_blend_op, src_alpha_factor, dst_alpha_factor);
        }

        //////////////////////////////////////////// init / cleanUp //////////////////////////////////////////////

        void Renderer::init(VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t sub_pass) {
            
            // storing the handles
            _render_pass_handle = render_pass;
            _sub_pass = sub_pass;

            // init the descriptor set cache
            _descriptor_cache.init(_device_handle, _descriptor_set_layouts, _descriptor_set_pool_sizes);

            // init the pipeline
            _pipeline.setViewport(viewport);
            _pipeline.setShaderStages(_shader.getShaderModules(), _shader.getShaderStages());
            _pipeline.init(_device_handle.getDevice(), render_pass.getRenderPass(), _sub_pass);

        }

        void Renderer::cleanUp() {
            
            _pipeline.cleanUp();
            _descriptor_cache.cleanUp();

            for(int i = 0; i < _descriptor_set_layouts.size(); i++) {
                if(!_extern_layouts.at(i))
                    _descriptor_set_layouts.at(i).cleanUp();

            }

            _shader.cleanUp();
        }

        ////////////////////////// settings that can be changed after the pipeline is initialized /////////////////

        void Renderer::resizeViewport(VkExtent2D viewport) {

            _pipeline.cleanUp();
            _pipeline.setViewport(viewport);
            _pipeline.init(_device_handle.getDevice(), _render_pass_handle.getRenderPass(), _sub_pass);
        }

        ////////////////////////////////////////// binding descriptors //////////////////////////////////////////

        void Renderer::resetDescriptorCache(uint32_t slot) {

            _descriptor_cache.reset({slot});
        }

        void Renderer::accquireDescriptorSet(uint32_t slot) {
            
            _descriptor_sets.at(slot) = &_descriptor_cache.accquire(slot);
        }

        void Renderer::bindUniformBuffer(uint32_t descriptor_set_slot, uint32_t binding, const Buffer& buffer) {
            
            if(!_descriptor_sets.at(descriptor_set_slot)) {
                UND_ERROR << "failed to bind descriptor, please accquire a descriptor set first\n";
                return;
            }

            _descriptor_sets.at(descriptor_set_slot)->bindUniformBuffer(binding, buffer);
        }

        void Renderer::bindImage(uint32_t descriptor_set_slot, uint32_t binding, const VkImageView& image_view, const VkImageLayout& layout, const VkSampler& sampler) {
            
            if(!_descriptor_sets.at(descriptor_set_slot)) {
                UND_ERROR << "failed to bind descriptor, please accquire a descriptor set first\n";
                return;
            }

            _descriptor_sets.at(descriptor_set_slot)->bindImage(binding, image_view, layout, sampler);
        }

        void Renderer::bindInputAttachment(uint32_t descriptor_set_slot, uint32_t binding, const VkImageView& image_view) {
            
            if(!_descriptor_sets.at(descriptor_set_slot)) {
                UND_ERROR << "failed to bind descriptor, please accquire a descriptor set first\n";
                return;
            }

            _descriptor_sets.at(descriptor_set_slot)->bindInputAttachment(binding, image_view);
        }
        

        //////////////////////////////////////////////// drawing ////////////////////////////////////////////////

        void Renderer::bindPipeline(undicht::vulkan::CommandBuffer& cmd) {

            cmd.bindGraphicsPipeline(_pipeline.getPipeline());
        }

        void Renderer::bindDescriptorSet(undicht::vulkan::CommandBuffer& cmd, uint32_t slot) {

            cmd.bindDescriptorSet(_descriptor_sets.at(slot)->getDescriptorSet(), _pipeline.getPipelineLayout(), slot);
        }

        void Renderer::bindDescriptorSet(undicht::vulkan::CommandBuffer& cmd, const undicht::vulkan::DescriptorSet& descriptor_set, uint32_t slot) {
            // bind an external descriptor set

            cmd.bindDescriptorSet(descriptor_set.getDescriptorSet(), _pipeline.getPipelineLayout(), slot);
        }

        void Renderer::bindVertexBuffer(undicht::vulkan::CommandBuffer& cmd, const undicht::vulkan::VertexBuffer& vbo, bool bind_index_buffer, bool bind_instance_buffer) {

            cmd.bindVertexBuffer(vbo.getVertexBuffer().getBuffer(), 0);

            if(bind_index_buffer)
                cmd.bindIndexBuffer(vbo.getIndexBuffer().getBuffer());

            if(bind_instance_buffer)
                cmd.bindVertexBuffer(vbo.getInstanceBuffer().getBuffer(), 1);

        }
        
        void Renderer::draw(undicht::vulkan::CommandBuffer& cmd, uint32_t vertex_count, bool draw_indexed, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {

            cmd.draw(vertex_count, draw_indexed, instance_count, first_vertex, first_instance);
        }

        //////////////////////////////////////////////// getters ////////////////////////////////////////////////

        const undicht::vulkan::DescriptorSetLayout& Renderer::getDescriptorSetLayout(uint32_t slot) const {
        
            return _descriptor_set_layouts.at(slot);
        }

        VkViewport Renderer::getViewport() const {

            return _pipeline.getViewport();
        }

    } // namespace vulkan

} // namespace undicht