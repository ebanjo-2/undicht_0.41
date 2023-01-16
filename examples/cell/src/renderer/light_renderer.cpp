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
        setDescriptorSetLayout({
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // global uniform buffer
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // local uniform buffer
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // tile map
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, // depth texture input
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, // material texture input
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, // normal texture input
        });
        setVertexInputLayout(LIGHT_VERTEX_LAYOUT, POINT_LIGHT_LAYOUT);
        setDepthStencilTest(false, false, VK_COMPARE_OP_GREATER);
        setRasterizer(true, true);
        setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        setBlending(0, true, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE); // hdr buffer, just add the light intensities
        Renderer::init(viewport, render_pass, subpass);

        // Renderer
        _sampler.setMinFilter(VK_FILTER_NEAREST);
        _sampler.setMaxFilter(VK_FILTER_NEAREST);
        _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _sampler.init(gpu.getDevice());

        // local uniform buffer
        // contains the tile map's tile size 
        _ubo.init(gpu, BufferLayout({UND_VEC2F}));
    }

    void LightRenderer::cleanUp() {
        
        _ubo.cleanUp();
        _sampler.cleanUp();
        Renderer::cleanUp();
    }

    void LightRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {
        
        Renderer::resizeViewport(viewport);
    }


    void LightRenderer::draw(const LightBuffer& lights, const MaterialAtlas& materials, const undicht::vulkan::UniformBuffer& global_ubo, undicht::vulkan::CommandBuffer& cmd, VkImageView depth, VkImageView material, VkImageView normal){

        cmd.bindGraphicsPipeline(_pipeline.getPipeline());

        cmd.bindVertexBuffer(lights.getPointLightBuffer().getVertexBuffer().getBuffer(), 0);
        cmd.bindVertexBuffer(lights.getPointLightBuffer().getInstanceBuffer().getBuffer(), 1);

        // storing the material tile maps dimensions in the global ubo
        float tile_map_unit[2];
        tile_map_unit[0] = 1.0f / MaterialAtlas::TILE_MAP_COLS; // width of a tile (in ndc)
        tile_map_unit[1] = 1.0f / MaterialAtlas::TILE_MAP_ROWS; // height of a tile (in ndc)
        _ubo.setAttribute(0, tile_map_unit, 2 * sizeof(float));

        undicht::vulkan::DescriptorSet& descriptor_set = _descriptor_cache.accquire();
        descriptor_set.bindUniformBuffer(0, global_ubo.getBuffer());
        descriptor_set.bindUniformBuffer(1, _ubo.getBuffer());
        descriptor_set.bindImage(2, materials.getTileMap().getImage().getImageView(), materials.getTileMap().getLayout(), _sampler.getSampler());
        descriptor_set.bindInputAttachment(3, depth);
        descriptor_set.bindInputAttachment(4, material);
        descriptor_set.bindInputAttachment(5, normal);

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