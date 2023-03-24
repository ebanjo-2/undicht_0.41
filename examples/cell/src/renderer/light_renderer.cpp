#define _USE_MATH_DEFINES // for M_PI
#include <math.h>

#include "light_renderer.h"
#include "file_tools.h"
#include "debug.h"
#include "core/vulkan/formats.h"
#include <random>
#include "glm/glm.hpp"
#include "IBL/ibl.h"
#include "array"
#include "renderer/vulkan/immediate_command.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    void LightRenderer::init(const undicht::vulkan::LogicalDevice& gpu, const undicht::vulkan::DescriptorSetLayout& global_descriptor_layout, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass, uint32_t num_frames) {
        
        // init the screen quad
        _screen_quad.init(gpu);
        {
            ImmediateCommand cmd(gpu);
            _screen_quad.setVertexData(SCREEN_QUAD_VERTICES.data(), SCREEN_QUAD_VERTICES.size() * sizeof(float), 0, cmd);
        }

        // init the sky box
        _sky_box.init(gpu);
        {
            ImmediateCommand cmd(gpu);
            _sky_box.setVertexData(SKY_BOX_VERTICES.data(), SKY_BOX_VERTICES.size() * sizeof(float), 0, cmd);
        }

        // descriptor layout for renderer local descriptors
        _local_descriptor_layout.setBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // local uniform buffer
        _local_descriptor_layout.setBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // shadow map offsets
        _local_descriptor_layout.setBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT); // albedo roughness input
        _local_descriptor_layout.setBinding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT); // normal metalness input
        _local_descriptor_layout.setBinding(4, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT); // position rel cam input
        _local_descriptor_layout.setBinding(5, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT); // shadow map pos input
        _local_descriptor_layout.init(gpu.getDevice());

        _descriptor_cache.init(gpu, {global_descriptor_layout, _local_descriptor_layout}, {0, 1}, num_frames);

        // setting up the _point_light_renderer
        _point_light_renderer.setDeviceHandle(gpu);
        _point_light_renderer.setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/point_light.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/point_light.frag.spv");
        _point_light_renderer.setDescriptorSetLayout(global_descriptor_layout, 0, 0); // global descriptors
        _point_light_renderer.setDescriptorSetLayout(_local_descriptor_layout, 1, 0); // renderer specific descriptors
        _point_light_renderer.setVertexInputLayout(LIGHT_VERTEX_LAYOUT, POINT_LIGHT_LAYOUT);
        _point_light_renderer.setDepthStencilTest(true, false, VK_COMPARE_OP_GREATER);
        _point_light_renderer.setRasterizer(false, true);
        _point_light_renderer.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        // hdr buffer, just add the light intensities
        _point_light_renderer.setBlending(0, true, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE); 
        _point_light_renderer.init(viewport, render_pass, subpass, num_frames);

        // setting up the direct light renderer
        _direct_light_renderer.setDeviceHandle(gpu);
        _direct_light_renderer.setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/direct_light.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/direct_light.frag.spv");
        _direct_light_renderer.setDescriptorSetLayout(global_descriptor_layout, 0, 0); // global descriptors
        _direct_light_renderer.setDescriptorSetLayout(_local_descriptor_layout, 1, 0); // renderer specific descriptors
        _direct_light_renderer.setDescriptorSetLayout({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER /*shadow map*/}, 2, 1); // light specific descriptors
        _direct_light_renderer.setVertexInputLayout(SCREEN_QUAD_VERTEX_LAYOUT);
        _direct_light_renderer.setDepthStencilTest(false, false);
        _direct_light_renderer.setRasterizer(false);
        _direct_light_renderer.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
         // hdr buffer, just add the light intensities (multiplied by the shadow effect determined by the world renderer)
        _direct_light_renderer.setBlending(0, true, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE);
        _direct_light_renderer.init(viewport, render_pass, subpass, num_frames);

        // setting up the ambient light renderer
        _ambient_light_renderer.setDeviceHandle(gpu);
        _ambient_light_renderer.setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/ambient_light.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/ambient_light.frag.spv");
        _ambient_light_renderer.setDescriptorSetLayout(global_descriptor_layout, 0, 0); // global descriptors
        _ambient_light_renderer.setDescriptorSetLayout(_local_descriptor_layout, 1, 0); // renderer specific descriptors
        _ambient_light_renderer.setDescriptorSetLayout({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER/*environment sky box*/, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER/*irradiance*/, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER/*specular reflections*/, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER/*BRDF*/}, 2, 1); // light specific descriptors
        _ambient_light_renderer.setVertexInputLayout(SKY_BOX_VERTEX_LAYOUT);
        _ambient_light_renderer.setDepthStencilTest(false, false);
        _ambient_light_renderer.setRasterizer(false);
        _ambient_light_renderer.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
         // hdr buffer, just add the light intensities (multiplied by the shadow effect determined by the world renderer)
        _ambient_light_renderer.setBlending(0, true, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE);
        _ambient_light_renderer.init(viewport, render_pass, subpass, num_frames);

        // Sampler
        _offset_sampler.setMinFilter(VK_FILTER_NEAREST);
        _offset_sampler.setMaxFilter(VK_FILTER_NEAREST);
        _offset_sampler.init(gpu.getDevice());

        _shadow_map_sampler.setMinFilter(VK_FILTER_NEAREST);
        _shadow_map_sampler.setMaxFilter(VK_FILTER_NEAREST);
        _shadow_map_sampler.setRepeatMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
        _shadow_map_sampler.setBorderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE); // to make sure that areas outside the shadow map are not covered in shadow
        _shadow_map_sampler.init(gpu.getDevice());

        _cube_map_sampler.setMinFilter(VK_FILTER_LINEAR);
        _cube_map_sampler.setMaxFilter(VK_FILTER_LINEAR);
        _cube_map_sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR);
        _cube_map_sampler.setRepeatMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        _cube_map_sampler.init(gpu.getDevice());

        _brdf_map_sampler.setMinFilter(VK_FILTER_LINEAR);
        _brdf_map_sampler.setMaxFilter(VK_FILTER_LINEAR);
        _brdf_map_sampler.setRepeatMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        _brdf_map_sampler.init(gpu.getDevice());

        // create a temporary transfer command buffer
        vulkan::CommandBuffer transfer_cmd;
        transfer_cmd.init(gpu.getDevice(), gpu.getGraphicsCmdPool());
        transfer_cmd.beginCommandBuffer(true);

        // create texture with random offsets for shadow sampling
        int num_filter_samples = _shadow_sampler_filter_size * _shadow_sampler_filter_size;
        std::vector<float> shadow_sampler_offsets;
        createShadowSampleOffsetTexture(_shadow_sampler_window_size, _shadow_sampler_filter_size, shadow_sampler_offsets);
        _shadow_sampler_offsets.setExtent(num_filter_samples / 2, _shadow_sampler_window_size, _shadow_sampler_window_size);
        _shadow_sampler_offsets.setFormat(translate(UND_VEC4F));
        _shadow_sampler_offsets.init(gpu);
        _shadow_sampler_offsets.setData(transfer_cmd, (const char*)shadow_sampler_offsets.data(), shadow_sampler_offsets.size() * sizeof(float));

        _brdf_map.init(gpu, transfer_cmd);

        // destroy temporary cmd buffer
        transfer_cmd.endCommandBuffer();
        gpu.submitOnGraphicsQueue(transfer_cmd.getCommandBuffer());
        gpu.waitGraphicsQueueIdle();
        transfer_cmd.cleanUp();

        // local uniform buffer
        // tile map tile size
        // shadow map offset textures size
        // shadow offset filter size;
        _local_ubo.init(gpu, BufferLayout({UND_VEC2F, UND_VEC2I, UND_VEC2I}));

        // light color
        // light direction
        // shadow map unit
        _direct_light_ubo.init(gpu, BufferLayout({UND_VEC3F, UND_VEC3F, UND_VEC2F}));

    }

    void LightRenderer::cleanUp() {
        
        _screen_quad.cleanUp();
        _sky_box.cleanUp();
        _local_ubo.cleanUp();
        _direct_light_ubo.cleanUp();
        _local_descriptor_layout.cleanUp();
        _descriptor_cache.cleanUp();
        _offset_sampler.cleanUp();
        _shadow_map_sampler.cleanUp();
        _cube_map_sampler.cleanUp();
        _brdf_map_sampler.cleanUp();
        _shadow_sampler_offsets.cleanUp();
        _brdf_map.cleanUp();
        _point_light_renderer.cleanUp();
        _direct_light_renderer.cleanUp();
        _ambient_light_renderer.cleanUp();
    }

    void LightRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {
        
        _point_light_renderer.resizeViewport(viewport);
        _direct_light_renderer.resizeViewport(viewport);
        _ambient_light_renderer.resizeViewport(viewport);
    }

    void LightRenderer::beginFrame(const undicht::vulkan::DescriptorSet& global_descriptor_set, undicht::vulkan::CommandBuffer& cmd, VkImageView albedo_rough, VkImageView normal_metal, VkImageView position_rel_cam, VkImageView shadow_map_pos, uint32_t frame_id){
        
        _point_light_renderer.beginFrame(frame_id);
        _direct_light_renderer.beginFrame(frame_id);
        _ambient_light_renderer.beginFrame(frame_id);

        _descriptor_cache.reset({1}, frame_id);
        _local_descriptor_set = _descriptor_cache.accquire(1, frame_id);

        _direct_light_renderer.resetDescriptorCache(2);
        _ambient_light_renderer.resetDescriptorCache(2);

        // storing the material tile maps dimensions in the local ubo
        float tile_map_unit[2];
        tile_map_unit[0] = 1.0f / MaterialAtlas::TILE_MAP_COLS; // width of a tile (in ndc)
        tile_map_unit[1] = 1.0f / MaterialAtlas::TILE_MAP_ROWS; // height of a tile (in ndc)
        _local_ubo.setAttribute(0, tile_map_unit, 2 * sizeof(float));

        int shadow_offset_texture_size[2];
        shadow_offset_texture_size[0] = _shadow_sampler_window_size;
        shadow_offset_texture_size[1] = _shadow_sampler_window_size;
        _local_ubo.setAttribute(1, shadow_offset_texture_size, 2 * sizeof(int));

        int shadow_offset_filter_size[2];
        shadow_offset_filter_size[0] = _shadow_sampler_filter_size;
        shadow_offset_filter_size[1] = _shadow_sampler_filter_size;
        _local_ubo.setAttribute(2, shadow_offset_filter_size, 2 * sizeof(int));

        // updating + binding the local descriptor set
        _local_descriptor_set.bindUniformBuffer(0, _local_ubo.getBuffer());
        //_local_descriptor_set.bindImage(1, materials.getTileMap().getImage().getImageView(), materials.getTileMap().getLayout(), _tile_map_sampler.getSampler());
        _local_descriptor_set.bindImage(1, _shadow_sampler_offsets.getImage().getImageView(), _shadow_sampler_offsets.getLayout(), _offset_sampler.getSampler());
        _local_descriptor_set.bindInputAttachment(2, albedo_rough);
        _local_descriptor_set.bindInputAttachment(3, normal_metal);
        _local_descriptor_set.bindInputAttachment(4, position_rel_cam);
        _local_descriptor_set.bindInputAttachment(5, shadow_map_pos);
        _local_descriptor_set.update();

        // binding the global descriptor set
        _point_light_renderer.bindDescriptorSet(cmd, global_descriptor_set, 0);
        _point_light_renderer.bindDescriptorSet(cmd, _local_descriptor_set, 1);
        _direct_light_renderer.bindDescriptorSet(cmd, global_descriptor_set, 0); // this shouldnt do anything, and if it did, it wouldnt be good
        _direct_light_renderer.bindDescriptorSet(cmd, _local_descriptor_set, 1); 
        _ambient_light_renderer.bindDescriptorSet(cmd, global_descriptor_set, 0); // this shouldnt do anything, and if it did, it wouldnt be good
        _ambient_light_renderer.bindDescriptorSet(cmd, _local_descriptor_set, 1);
    }

    void LightRenderer::draw(const LightBuffer& lights, undicht::vulkan::CommandBuffer& cmd){

        _point_light_renderer.bindPipeline(cmd);
        _point_light_renderer.bindVertexBuffer(cmd, lights.getBuffer(), false, true);

        uint32_t light_byte_size = POINT_LIGHT_LAYOUT.getTotalSize();
        for(const LightBuffer::BufferEntry& entry : lights.getDrawAreas()) {

            // draw command
            uint32_t light_count = entry.byte_size / light_byte_size;
            uint32_t current_offset = entry.offset / light_byte_size;
            _point_light_renderer.draw(cmd, lights.getPointLightModelVertexCount(), false, light_count, current_offset);
        }
    }

    void LightRenderer::draw(const Light& light, undicht::vulkan::CommandBuffer& cmd, const VkImageView& shadow_map, const VkImageLayout& shadow_map_layout, uint32_t shadow_map_width, uint32_t shadow_map_height) {
        
        // storing the lights data in the ubo
        float shadow_map_unit[2];
        shadow_map_unit[0] = 1.0f / shadow_map_width;
        shadow_map_unit[1] = 1.0f / shadow_map_height;
        _direct_light_ubo.setAttribute(0, glm::value_ptr(light.getColor()), 3 * sizeof(float));
        _direct_light_ubo.setAttribute(1, glm::value_ptr(light.getDirection()), 3 * sizeof(float));
        _direct_light_ubo.setAttribute(2, shadow_map_unit, 2 * sizeof(float));

        _direct_light_renderer.accquireDescriptorSet(2);
        _direct_light_renderer.bindUniformBuffer(2, 0, _direct_light_ubo.getBuffer());
        _direct_light_renderer.bindImage(2, 1, shadow_map, shadow_map_layout, _shadow_map_sampler.getSampler());
        _direct_light_renderer.bindDescriptorSet(cmd, 2);

        _direct_light_renderer.bindPipeline(cmd);
        _direct_light_renderer.bindVertexBuffer(cmd, _screen_quad);
        _direct_light_renderer.draw(cmd, 6);

    }

    void LightRenderer::draw(const Environment& env, undicht::vulkan::CommandBuffer& cmd) {

        _ambient_light_renderer.accquireDescriptorSet(2);
        _ambient_light_renderer.bindImage(2, 0, env.getSkyBox().getImage().getImageView(), env.getSkyBox().getLayout(), _cube_map_sampler.getSampler()); 
        _ambient_light_renderer.bindImage(2, 1, env.getIrradiance().getImage().getImageView(), env.getIrradiance().getLayout(), _cube_map_sampler.getSampler());
        _ambient_light_renderer.bindImage(2, 2, env.getSpecular().getImage().getImageView(), env.getSpecular().getLayout(), _cube_map_sampler.getSampler()); 
        _ambient_light_renderer.bindImage(2, 3, _brdf_map.getMap().getImage().getImageView(), _brdf_map.getMap().getLayout(), _brdf_map_sampler.getSampler());
        _ambient_light_renderer.bindDescriptorSet(cmd, 2);

        _ambient_light_renderer.bindPipeline(cmd);
        _ambient_light_renderer.bindVertexBuffer(cmd, _sky_box);
        _ambient_light_renderer.draw(cmd, 36);
    }


    ////////////////////////////////////////////// protected LightRenderer functions //////////////////////////////////////////////

    void LightRenderer::createShadowSampleOffsetTexture(int window_size, int filter_size, std::vector<float>& data) {
        // https://github.com/emeiri/ogldev/tree/master/tutorial41_youtube


        int buffer_size = window_size * window_size * filter_size * filter_size * 2;
        data.resize(buffer_size);

        // random number generator
        static std::default_random_engine generator;
        static std::uniform_real_distribution<float> distrib(-0.5f, 0.5f);

        int index = 0;
        
        // traversing through the window containing a filter for every fragment
        for(int tex_y = 0; tex_y < window_size; tex_y++) {
            for(int tex_x = 0; tex_x < window_size; tex_x++) {

                // traversing through the filter (2D)
                for(int v = filter_size - 1; v >= 0 ; v--) { // start at max v, so that the outer ring filter begins at index 0
                    for(int u = 0; u < filter_size; u++) {

                        float x = ((float)u + 0.5f + distrib(generator)) / (float)filter_size;
                        float y = ((float)v + 0.5f + distrib(generator)) / (float)filter_size;

                        assert(index + 1 < data.size());
                        data[index]     = glm::sqrt(y) * glm::cos(2 * M_PI * x);
                        data[index + 1] = glm::sqrt(y) * glm::sin(2 * M_PI * x);

                        index += 2;
                    }
                }

            }
        }

    }

} // cell