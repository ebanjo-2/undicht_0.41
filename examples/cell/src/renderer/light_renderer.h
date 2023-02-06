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
#include "entities/light_buffer.h"
#include "entities/lights/direct_light.h"

namespace cell {

    class LightRenderer {

      protected:
      
        undicht::vulkan::VertexBuffer _screen_quad;

        // renderer
        undicht::vulkan::Renderer _point_light_renderer;
        undicht::vulkan::Renderer _direct_light_renderer;
        undicht::vulkan::Sampler _tile_map_sampler;
        undicht::vulkan::Sampler _shadow_map_sampler;
        undicht::vulkan::DescriptorSetCache _descriptor_cache;

        // shadow map sampling
        const int _shadow_sampler_window_size = 16; // size of the random sample offset texture
        const int _shadow_sampler_filter_size = 6; // num of samples per ring and num of rings (only even numbers work)
        undicht::vulkan::Texture _shadow_sampler_offsets;

        undicht::vulkan::UniformBuffer _local_ubo;
        undicht::vulkan::DescriptorSetLayout _local_descriptor_layout;
        undicht::vulkan::DescriptorSet _local_descriptor_set;

        undicht::vulkan::UniformBuffer _light_ubo;

      public:

        void init(const undicht::vulkan::LogicalDevice& gpu, const undicht::vulkan::DescriptorSetLayout& global_descriptor_layout, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass);
        void cleanUp();

        void onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);

        void beginFrame(const MaterialAtlas& materials, const undicht::vulkan::DescriptorSet& global_descriptor_set, undicht::vulkan::CommandBuffer& cmd, VkImageView material, VkImageView normal, VkImageView shadow_map_pos);
        void draw(const LightBuffer& lights, undicht::vulkan::CommandBuffer& cmd);
        void draw(const DirectLight& light, undicht::vulkan::CommandBuffer& cmd, const VkImageView& shadow_map, const VkImageLayout& shadow_map_layout, uint32_t shadow_map_width, uint32_t shadow_map_height);

      protected:
        // protected LightRenderer functions

        void createShadowSampleOffsetTexture(int window_size, int filter_size, std::vector<float>& data);

    };

} // namespace cell

#endif // LIGHT_RENDERER_H