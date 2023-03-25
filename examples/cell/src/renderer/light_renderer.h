#ifndef LIGHT_RENDERER_H
#define LIGHT_RENDERER_H

#include "core/vulkan/shader.h"
#include "core/vulkan/descriptor_set_layout.h"
#include "core/vulkan/pipeline.h"
#include "core/vulkan/sampler.h"
#include "core/vulkan/swap_chain.h"
#include "core/vulkan/renderpass.h"
#include "core/vulkan/framebuffer.h"
#include "core/vulkan/descriptor_set.h"
#include "renderer/vulkan/descriptor_set_cache.h"
#include "renderer/vulkan/uniform_buffer.h"
#include "renderer/vulkan/renderer.h"
#include "3D/camera/perspective_camera_3d.h"
#include "materials/material_atlas.h"
#include "world/lights/light_buffer.h"
#include "brdf_integration_map.h"
#include "environment/environment.h"

namespace cell {

    class LightRenderer {

      protected:
      
        undicht::vulkan::VertexBuffer _screen_quad;
        undicht::vulkan::VertexBuffer _sky_box; // for using cube maps

        // renderer
        undicht::vulkan::Renderer _point_light_renderer;
        undicht::vulkan::Renderer _direct_light_renderer;
        undicht::vulkan::Renderer _ambient_light_renderer;
        undicht::vulkan::Sampler _offset_sampler;
        undicht::vulkan::Sampler _shadow_map_sampler;
        undicht::vulkan::Sampler _cube_map_sampler;
        undicht::vulkan::Sampler _brdf_map_sampler;
        undicht::vulkan::DescriptorSetCache _descriptor_cache;

        // shadow map sampling
        const int _shadow_sampler_window_size = 16; // size of the random sample offset texture
        const int _shadow_sampler_filter_size = 4; // num of samples per ring and num of rings (only even numbers work)
        undicht::vulkan::Texture _shadow_sampler_offsets;

        // image based lighting
        BRDFIntegrationMap _brdf_map;

        undicht::vulkan::UniformBuffer _local_ubo;
        undicht::vulkan::DescriptorSetLayout _local_descriptor_layout;
        undicht::vulkan::DescriptorSet _local_descriptor_set;

        undicht::vulkan::UniformBuffer _direct_light_ubo;

      public:

        void init(const undicht::vulkan::LogicalDevice& gpu, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf, const undicht::vulkan::DescriptorSetLayout& global_descriptor_layout, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass, uint32_t num_frames);
        void cleanUp();

        void onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);

        //void loadEnvironment(const std::string& file_name);

        void beginFrame(const undicht::vulkan::DescriptorSet& global_descriptor_set, undicht::vulkan::CommandBuffer& cmd, VkImageView albedo_rough, VkImageView normal_metal, VkImageView position_rel_cam, VkImageView shadow_map_pos, uint32_t frame_id);
        void draw(const LightBuffer& lights, undicht::vulkan::CommandBuffer& cmd);
        void draw(const Light& light, undicht::vulkan::CommandBuffer& cmd, const VkImageView& shadow_map, const VkImageLayout& shadow_map_layout, uint32_t shadow_map_width, uint32_t shadow_map_height); // direct light only
        void draw(const Environment& env, undicht::vulkan::CommandBuffer& cmd); // ambient light / light from the environment map

      protected:
        // protected LightRenderer functions

        void createShadowSampleOffsetTexture(int window_size, int filter_size, std::vector<float>& data);

    };

} // namespace cell

#endif // LIGHT_RENDERER_H