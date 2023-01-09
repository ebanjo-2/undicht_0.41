#include "light_renderer.h"
#include "file_tools.h"
#include "debug.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    void LightRenderer::init(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass) {
        
        _device_handle = gpu;
        _render_pass_handle = render_pass;
        _subpass = subpass;

        // Shader
        _shader.addVertexModule(gpu.getDevice(), getFilePath(UND_CODE_SRC_FILE) + "shader/bin/light.vert.spv");
        _shader.addFragmentModule(gpu.getDevice(), getFilePath(UND_CODE_SRC_FILE) + "shader/bin/light.frag.spv");
        _shader.init(gpu.getDevice());

        // Pipeline
        _descriptor_set_layout.setBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // global data
        _descriptor_set_layout.init(gpu.getDevice());

        _descriptor_cache.init(gpu, _descriptor_set_layout);

        _pipeline.setViewport(viewport);
        _pipeline.setShaderStages(_shader.getShaderModules(), _shader.getShaderStages());
        _pipeline.setBlending(0, false);
        _pipeline.setDepthStencilState(true, false);
        _pipeline.setInputAssembly();
        _pipeline.setRasterizationState(true, false, false);
        _pipeline.setShaderInput(_descriptor_set_layout.getLayout());

        _pipeline.setVertexBinding(0, 0, LIGHT_VERTEX_LAYOUT); // per vertex data
        _pipeline.setVertexBinding(1, LIGHT_VERTEX_LAYOUT.m_types.size(), LIGHT_LAYOUT); // per light/instance data

        _pipeline.init(gpu.getDevice(), render_pass.getRenderPass(), subpass);

        // Renderer
        _sampler.setMinFilter(VK_FILTER_NEAREST);
        _sampler.setMaxFilter(VK_FILTER_NEAREST);
        _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _sampler.init(gpu.getDevice());

        // uniform buffer layout:
        // mat4 proj
        // mat4 view
        _global_uniform_buffer.init(gpu, BufferLayout({UND_MAT4F, UND_MAT4F}));
    }

    void LightRenderer::cleanUp() {

        _global_uniform_buffer.cleanUp();
        _sampler.cleanUp();
        _pipeline.cleanUp();
        _descriptor_cache.cleanUp();
        _descriptor_set_layout.cleanUp();
        _shader.cleanUp();

    }

    void LightRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {
        
        _pipeline.cleanUp();
        _pipeline.setViewport(viewport);
        _pipeline.init(gpu.getDevice(), render_pass.getRenderPass(), _subpass);
    }

    void LightRenderer::loadCamera(undicht::tools::PerspectiveCamera3D& camera) {

        _global_uniform_buffer.setAttribute(0, glm::value_ptr(camera.getCameraProjectionMatrix()), 16 * sizeof(float));
        _global_uniform_buffer.setAttribute(1, glm::value_ptr(camera.getViewMatrix()), 16 * sizeof(float));
    }

    void LightRenderer::draw(const LightBuffer& lights, undicht::vulkan::CommandBuffer& cmd){

        cmd.bindGraphicsPipeline(_pipeline.getPipeline());

        cmd.bindVertexBuffer(lights.getPointLightBuffer().getVertexBuffer().getBuffer(), 0);
        cmd.bindVertexBuffer(lights.getPointLightBuffer().getInstanceBuffer().getBuffer(), 1);

        undicht::vulkan::DescriptorSet& descriptor_set = _descriptor_cache.accquire();
        descriptor_set.bindUniformBuffer(0, _global_uniform_buffer.getBuffer());

        // bind the descriptor set
        cmd.bindDescriptorSet(descriptor_set.getDescriptorSet(), _pipeline.getPipelineLayout());

        // draw command
        cmd.draw(lights.getPointLightModelVertexCount(), false, lights.getPointLightCount()); // 2 triangles make up the screen quad, so 6 vertices in total

    }

    void LightRenderer::beginFrame(){

        _descriptor_cache.reset();
    }

    const undicht::vulkan::DescriptorSetLayout& LightRenderer::getDescriptorSetLayout() const {
        
        return _descriptor_set_layout;
    }

} // cell