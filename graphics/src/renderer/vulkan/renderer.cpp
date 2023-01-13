#include "renderer/vulkan/renderer.h"

namespace undicht {

    namespace vulkan {

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

        void Renderer::setDescriptorSetLayout(const std::vector<VkDescriptorType>& binding_types) {
            /// @brief set the layout of the resources used by the shaders
            /// for example binding type 0 could be a uniform buffer, the binding 1 could be a image sampler

            for(int i = 0; i < binding_types.size(); i++)
                _descriptor_set_layout.setBinding(i, binding_types.at(i));

            _descriptor_set_layout.init(_device_handle.getDevice());
            _descriptor_cache.init(_device_handle, _descriptor_set_layout);
        }

        void Renderer::setVertexInputLayout(const undicht::BufferLayout& vertex_layout, const undicht::BufferLayout& instance_layout) {
            
            _pipeline.setVertexBinding(0, 0, vertex_layout);

            if(instance_layout.getTotalSize())
                _pipeline.setVertexBinding(1, vertex_layout.m_types.size(), instance_layout);

        }

        void Renderer::setDepthStencilTest(bool enable_depth_test, bool write_depth_values) {
            
            _pipeline.setDepthStencilState(enable_depth_test, write_depth_values);
        }

        void Renderer::setRasterizer(bool enable_culling, bool cull_ccw_faces, bool wire_frame) {

            _pipeline.setRasterizationState(enable_culling, cull_ccw_faces, wire_frame);
        }

        void Renderer::setInputAssembly(VkPrimitiveTopology topology) {

            _pipeline.setInputAssembly(topology);
        }

        void Renderer::setBlending(uint32_t attachment, bool enable_blending, VkBlendOp color_blend_op, VkBlendOp alpha_blend_op) {
            // set the blending for output at binding #attachment
            _pipeline.setBlending(attachment, enable_blending, color_blend_op, alpha_blend_op);
        }

        //////////////////////////////////////////// init / cleanUp //////////////////////////////////////////////

        void Renderer::init(VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t sub_pass) {
            
            // storing the handles
            _render_pass_handle = render_pass;
            _sub_pass = sub_pass;

            // init the pipeline
            _pipeline.setViewport(viewport);
            _pipeline.setShaderStages(_shader.getShaderModules(), _shader.getShaderStages());
            _pipeline.setShaderInput(_descriptor_set_layout.getLayout());
            _pipeline.init(_device_handle.getDevice(), render_pass.getRenderPass(), _sub_pass);

        }

        void Renderer::cleanUp() {
            
            _pipeline.cleanUp();
            _descriptor_cache.cleanUp();
            _descriptor_set_layout.cleanUp();
            _shader.cleanUp();
        }

        ////////////////////////// settings that can be changed after the pipeline is initialized /////////////////

        void Renderer::resizeViewport(VkExtent2D viewport) {

            _pipeline.cleanUp();
            _pipeline.setViewport(viewport);
            _pipeline.init(_device_handle.getDevice(), _render_pass_handle.getRenderPass(), _sub_pass);
        }
        
    } // namespace vulkan

} // namespace undicht