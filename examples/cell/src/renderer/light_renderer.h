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
        undicht::vulkan::Sampler _sampler;
        undicht::vulkan::UniformBuffer _local_ubo;

      public:

        void init(const undicht::vulkan::LogicalDevice& gpu, const undicht::vulkan::DescriptorSetLayout& global_descriptor_layout, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass);
        void cleanUp();

        void onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);

        void beginFrame(const MaterialAtlas& materials, const undicht::vulkan::DescriptorSet& global_descriptor_set, undicht::vulkan::CommandBuffer& cmd, VkImageView material, VkImageView normal);
        void draw(const LightBuffer& lights, undicht::vulkan::CommandBuffer& cmd);
        void draw(const DirectLight& light, undicht::vulkan::CommandBuffer& cmd);

    };

} // namespace cell

#endif // LIGHT_RENDERER_H