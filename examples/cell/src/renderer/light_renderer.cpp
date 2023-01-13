#include "light_renderer.h"
#include "file_tools.h"
#include "debug.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    void LightRenderer::init(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass) {
        
        // setting up the renderer
        setDeviceHandle(gpu);
        setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/light.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/light.frag.spv");
        setDescriptorSetLayout({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
        setVertexInputLayout(LIGHT_VERTEX_LAYOUT, LIGHT_LAYOUT);
        setDepthStencilTest(true, false);
        setRasterizer(true);
        setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        setBlending(0, false);
        Renderer::init(viewport, render_pass, subpass);

        // Renderer
        _sampler.setMinFilter(VK_FILTER_NEAREST);
        _sampler.setMaxFilter(VK_FILTER_NEAREST);
        _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _sampler.init(gpu.getDevice());

    }

    void LightRenderer::cleanUp() {

        _sampler.cleanUp();
        Renderer::cleanUp();
    }

    void LightRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {
        
        Renderer::resizeViewport(viewport);
    }


    void LightRenderer::draw(const LightBuffer& lights, const undicht::vulkan::UniformBuffer& global_ubo, undicht::vulkan::CommandBuffer& cmd){

        cmd.bindGraphicsPipeline(_pipeline.getPipeline());

        cmd.bindVertexBuffer(lights.getPointLightBuffer().getVertexBuffer().getBuffer(), 0);
        cmd.bindVertexBuffer(lights.getPointLightBuffer().getInstanceBuffer().getBuffer(), 1);

        undicht::vulkan::DescriptorSet& descriptor_set = _descriptor_cache.accquire();
        descriptor_set.bindUniformBuffer(0, global_ubo.getBuffer());

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