#ifndef FINAL_RENDERER_H
#define FINAL_RENDERER_H

#include "renderer/vulkan/vertex_buffer.h"
#include "renderer/vulkan/uniform_buffer.h"
#include "3D/camera/perspective_camera_3d.h"
#include "materials/material_atlas.h"
#include "renderer/vulkan/descriptor_set_cache.h"
#include "core/vulkan/renderpass.h"
#include "core/vulkan/shader.h"
#include "core/vulkan/sampler.h"
#include "core/vulkan/pipeline.h"

namespace cell {

    class FinalRenderer {
    
      protected:

        undicht::vulkan::LogicalDevice _device_handle;
        undicht::vulkan::VertexBuffer _screen_quad;

        // Shader
        undicht::vulkan::Shader _shader;

        // Pipeline
        uint32_t _subpass = 0;
        undicht::vulkan::DescriptorSetLayout _descriptor_set_layout;
        undicht::vulkan::DescriptorSetCache _descriptor_cache;
        undicht::vulkan::Pipeline _pipeline;

        // renderer
        undicht::vulkan::Sampler _sampler;
        undicht::vulkan::UniformBuffer _global_uniform_buffer;

      public:

        void init(const undicht::vulkan::LogicalDevice& device, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass);
        void cleanUp();
        
        void onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);

        void loadCamera(undicht::tools::PerspectiveCamera3D& camera);
        void draw(const MaterialAtlas& materials, undicht::vulkan::CommandBuffer& cmd, const undicht::vulkan::Image& color_input, const undicht::vulkan::Image& depth_input);
        void beginFrame();

    };


} // cell

#endif // FINAL_RENDERER_H