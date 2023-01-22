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
        
        // descriptor layout for renderer local descriptors
        _local_descriptor_layout.setBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // local uniform buffer
        _local_descriptor_layout.setBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // tile map
        _local_descriptor_layout.setBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT); // material texture input
        _local_descriptor_layout.setBinding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT); // normal texture input
        _local_descriptor_layout.init(gpu.getDevice());

        _descriptor_cache.init(gpu, {global_descriptor_layout, _local_descriptor_layout}, {0, 1});

        // setting up the _point_light_renderer
        _point_light_renderer.setDeviceHandle(gpu);
        _point_light_renderer.setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/point_light.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/point_light.frag.spv");
        _point_light_renderer.setDescriptorSetLayout(global_descriptor_layout, 0, 0); // global descriptors
        _point_light_renderer.setDescriptorSetLayout(_local_descriptor_layout, 1, 0); // renderer specific descriptors
        _point_light_renderer.setVertexInputLayout(LIGHT_VERTEX_LAYOUT, POINT_LIGHT_LAYOUT);
        _point_light_renderer.setDepthStencilTest(true, false, VK_COMPARE_OP_GREATER);
        _point_light_renderer.setRasterizer(true, true);
        _point_light_renderer.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        _point_light_renderer.setBlending(0, true, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE); // hdr buffer, just add the light intensities
        _point_light_renderer.init(viewport, render_pass, subpass);

        // setting up the direct light renderer
        _direct_light_renderer.setDeviceHandle(gpu);
        _direct_light_renderer.setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/direct_light.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/direct_light.frag.spv");
        _direct_light_renderer.setDescriptorSetLayout(global_descriptor_layout, 0, 0); // global descriptors
        _direct_light_renderer.setDescriptorSetLayout(_local_descriptor_layout, 1, 0); // renderer specific descriptors
        _direct_light_renderer.setDescriptorSetLayout({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}, 2, 1); // light specific descriptors
        _direct_light_renderer.setVertexInputLayout(SCREEN_QUAD_VERTEX_LAYOUT);
        _direct_light_renderer.setDepthStencilTest(false, false);
        _direct_light_renderer.setRasterizer(false);
        _direct_light_renderer.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        _direct_light_renderer.setBlending(0, true, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE); // hdr buffer, just add the light intensities
        _direct_light_renderer.init(viewport, render_pass, subpass);

        // Sampler
        _sampler.setMinFilter(VK_FILTER_NEAREST);
        _sampler.setMaxFilter(VK_FILTER_NEAREST);
        _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _sampler.init(gpu.getDevice());

        // local uniform buffer
        // contains the tile map's tile size 
        _local_ubo.init(gpu, BufferLayout({UND_VEC2F}));

        // per light ubo (only used by the direct lights at the moment)
        // light color
        // light direction
        // shadow view matrix
        // shadow proj matrix
        _light_ubo.init(gpu, BufferLayout({UND_VEC3F, UND_VEC3F, UND_MAT4F, UND_MAT4F}));
    }

    void LightRenderer::cleanUp() {
        
        _screen_quad.cleanUp();
        _local_ubo.cleanUp();
        _light_ubo.cleanUp();
        _local_descriptor_layout.cleanUp();
        _descriptor_cache.cleanUp();
        _sampler.cleanUp();
        _point_light_renderer.cleanUp();
        _direct_light_renderer.cleanUp();
    }

    void LightRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {
        
        _point_light_renderer.resizeViewport(viewport);
        _direct_light_renderer.resizeViewport(viewport);
    }

    void LightRenderer::beginFrame(const MaterialAtlas& materials, const undicht::vulkan::DescriptorSet& global_descriptor_set, undicht::vulkan::CommandBuffer& cmd, VkImageView material, VkImageView normal){
        
        _descriptor_cache.reset({1});
        _local_descriptor_set = _descriptor_cache.accquire(1);

        _direct_light_renderer.resetDescriptorCache(2);

        // storing the material tile maps dimensions in the local ubo
        float tile_map_unit[2];
        tile_map_unit[0] = 1.0f / MaterialAtlas::TILE_MAP_COLS; // width of a tile (in ndc)
        tile_map_unit[1] = 1.0f / MaterialAtlas::TILE_MAP_ROWS; // height of a tile (in ndc)
        _local_ubo.setAttribute(0, tile_map_unit, 2 * sizeof(float));

        // updating + binding the local descriptor set
        _local_descriptor_set.bindUniformBuffer(0, _local_ubo.getBuffer());
        _local_descriptor_set.bindImage(1, materials.getTileMap().getImage().getImageView(), materials.getTileMap().getLayout(), _sampler.getSampler());
        _local_descriptor_set.bindInputAttachment(2, material);
        _local_descriptor_set.bindInputAttachment(3, normal);

        // binding the global descriptor set
        _point_light_renderer.bindDescriptorSet(cmd, global_descriptor_set, 0);
        _point_light_renderer.bindDescriptorSet(cmd, _local_descriptor_set, 1);
        _direct_light_renderer.bindDescriptorSet(cmd, global_descriptor_set, 0); // this shouldnt do anything, and if it did, it wouldnt be good
        _direct_light_renderer.bindDescriptorSet(cmd, _local_descriptor_set, 1); 

    }

    void LightRenderer::draw(const LightBuffer& lights, undicht::vulkan::CommandBuffer& cmd){

        _point_light_renderer.bindPipeline(cmd);
        _point_light_renderer.bindVertexBuffer(cmd, lights.getPointLightBuffer(), false, true);
        _point_light_renderer.draw(cmd, lights.getPointLightModelVertexCount(), false, lights.getPointLightCount());

    }

    void LightRenderer::draw(const DirectLight& light, const VkImageView& shadow_map, const VkImageLayout& shadow_map_layout, undicht::vulkan::CommandBuffer& cmd) {
        
        // storing the lights data in the ubo
        _light_ubo.setAttribute(0, glm::value_ptr(light.getColor()), 3 * sizeof(float));
        _light_ubo.setAttribute(1, glm::value_ptr(light.getDirection()), 3 * sizeof(float));
        _light_ubo.setAttribute(2, glm::value_ptr(light.getShadowView()), 16 * sizeof(float));
        _light_ubo.setAttribute(3, glm::value_ptr(light.getShadowProj()), 16 * sizeof(float));

        _direct_light_renderer.accquireDescriptorSet(2);
        _direct_light_renderer.bindUniformBuffer(2, 0, _light_ubo.getBuffer());
        _direct_light_renderer.bindImage(2, 1, shadow_map, shadow_map_layout, _sampler.getSampler());
        _direct_light_renderer.bindDescriptorSet(cmd, 2);

        _direct_light_renderer.bindPipeline(cmd);
        _direct_light_renderer.bindVertexBuffer(cmd, _screen_quad);
        _direct_light_renderer.draw(cmd, 6);

    }

} // cell