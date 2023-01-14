#ifndef FINAL_RENDERER_H
#define FINAL_RENDERER_H

#include "renderer/vulkan/vertex_buffer.h"
#include "renderer/vulkan/uniform_buffer.h"
#include "3D/camera/perspective_camera_3d.h"
#include "materials/material_atlas.h"
#include "renderer/vulkan/descriptor_set_cache.h"
#include "renderer/vulkan/renderer.h"
#include "core/vulkan/renderpass.h"
#include "core/vulkan/shader.h"
#include "core/vulkan/sampler.h"
#include "core/vulkan/pipeline.h"

namespace cell {

    class FinalRenderer : public undicht::vulkan::Renderer {
    
      protected:

        undicht::vulkan::VertexBuffer _screen_quad;

        // renderer
        undicht::vulkan::Sampler _sampler;
        undicht::vulkan::UniformBuffer _ubo;

      public:

        void init(const undicht::vulkan::LogicalDevice& device, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass);
        void cleanUp();
        
        void onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);

        void draw(const undicht::vulkan::UniformBuffer& global_ubo, const MaterialAtlas& atlas, undicht::vulkan::CommandBuffer& cmd, float exposure, float gamma, VkImageView material, VkImageView light);
        void beginFrame();

    };


} // cell

#endif // FINAL_RENDERER_H