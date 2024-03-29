#ifndef FINAL_RENDERER_H
#define FINAL_RENDERER_H

#include "renderer/vulkan/vertex_buffer.h"
#include "renderer/vulkan/uniform_buffer.h"
#include "3D/camera/perspective_camera_3d.h"
#include "materials/material_atlas.h"
#include "renderer/vulkan/descriptor_set_cache.h"
#include "renderer/vulkan/renderer.h"
#include "renderer/vulkan/transfer_buffer.h"
#include "core/vulkan/renderpass.h"
#include "core/vulkan/shader.h"
#include "core/vulkan/sampler.h"
#include "core/vulkan/pipeline.h"

namespace cell {

    class FinalRenderer {
    
      protected:

        undicht::vulkan::VertexBuffer _screen_quad;

        // renderer
        undicht::vulkan::Renderer _renderer;
        undicht::vulkan::Sampler _sampler;
        undicht::vulkan::UniformBuffer _local_ubo;

      public:

        void init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass, uint32_t num_frames);
        void cleanUp();
        
        void onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);
        
        void beginFrame(undicht::vulkan::CommandBuffer& cmd, VkImageView light_hdr, uint32_t frame_id);
        void draw(undicht::vulkan::CommandBuffer& cmd, float exposure);
        

    };


} // cell

#endif // FINAL_RENDERER_H