#include "light_renderer.h"
#include "file_tools.h"
#include "debug.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    void LightRenderer::init(const undicht::vulkan::LogicalDevice& gpu, const undicht::vulkan::DescriptorSetLayout& global_descriptor_layout, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass) {
        
        // init the screen quad
        _screen_quad.init(gpu);
        _screen_quad.setVertexData(SCREEN_QUAD_VERTICES.data(), SCREEN_QUAD_VERTICES.size() * sizeof(float), 0);

        // setting up the _point_light_renderer
        _point_light_renderer.setDeviceHandle(gpu);
        _point_light_renderer.setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/light.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/light.frag.spv");
        _point_light_renderer.setDescriptorSetLayout(global_descriptor_layout, 0, 0); // global uniform buffer
        _point_light_renderer.setDescriptorSetLayout({
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // local uniform buffer
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // tile map
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, // material texture input
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, // normal texture input
        }, 1);
        _point_light_renderer.setVertexInputLayout(LIGHT_VERTEX_LAYOUT, POINT_LIGHT_LAYOUT);
        _point_light_renderer.setDepthStencilTest(true, false, VK_COMPARE_OP_GREATER);
        _point_light_renderer.setRasterizer(true, true);
        _point_light_renderer.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        _point_light_renderer.setBlending(0, true, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE); // hdr buffer, just add the light intensities
        _point_light_renderer.init(viewport, render_pass, subpass);

        // Sampler
        _sampler.setMinFilter(VK_FILTER_NEAREST);
        _sampler.setMaxFilter(VK_FILTER_NEAREST);
        _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _sampler.init(gpu.getDevice());

        // local uniform buffer
        // contains the tile map's tile size 
        _local_ubo.init(gpu, BufferLayout({UND_VEC2F}));
    }

    void LightRenderer::cleanUp() {
        
        _screen_quad.cleanUp();
        _local_ubo.cleanUp();
        _sampler.cleanUp();
        _point_light_renderer.cleanUp();
    }

    void LightRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {
        
        _point_light_renderer.resizeViewport(viewport);
    }

    void LightRenderer::beginFrame(const MaterialAtlas& materials, const undicht::vulkan::DescriptorSet& global_descriptor_set, undicht::vulkan::CommandBuffer& cmd, VkImageView material, VkImageView normal){
        
        _point_light_renderer.resetDescriptorCache(1);

        // storing the material tile maps dimensions in the local ubo
        float tile_map_unit[2];
        tile_map_unit[0] = 1.0f / MaterialAtlas::TILE_MAP_COLS; // width of a tile (in ndc)
        tile_map_unit[1] = 1.0f / MaterialAtlas::TILE_MAP_ROWS; // height of a tile (in ndc)
        _local_ubo.setAttribute(0, tile_map_unit, 2 * sizeof(float));

        // updating + binding the pointlightrenderers descriptor set
        _point_light_renderer.accquireDescriptorSet(1);
        _point_light_renderer.bindDescriptor(1, 0, _local_ubo.getBuffer());
        _point_light_renderer.bindDescriptor(1, 1, materials.getTileMap().getImage().getImageView(), materials.getTileMap().getLayout(), _sampler.getSampler());
        _point_light_renderer.bindDescriptor(1, 2, material);
        _point_light_renderer.bindDescriptor(1, 3, normal);
        _point_light_renderer.bindDescriptorSet(cmd, 1);

        // binding the global descriptor set
        _point_light_renderer.bindDescriptorSet(cmd, global_descriptor_set, 0);

    }

    void LightRenderer::draw(const LightBuffer& lights, undicht::vulkan::CommandBuffer& cmd){

        _point_light_renderer.bindPipeline(cmd);
        _point_light_renderer.bindVertexBuffer(cmd, lights.getPointLightBuffer(), false, true);
        _point_light_renderer.draw(cmd, lights.getPointLightModelVertexCount(), false, lights.getPointLightCount());

    }

    void LightRenderer::draw(const DirectLight& light, undicht::vulkan::CommandBuffer& cmd) {
        

    }

} // cell