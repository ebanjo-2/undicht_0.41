#ifndef WORLD_RENDERER_H
#define WORLD_RENDERER_H

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
#include "3D/camera/perspective_camera_3d.h"
#include "materials/material_atlas.h"

#include "renderer/world_buffer.h"

namespace cell {

    class WorldRenderer {

      protected:

        // handles to other objects
        undicht::vulkan::LogicalDevice _device_handle;
        undicht::vulkan::RenderPass _render_pass_handle;

        // Shader
        undicht::vulkan::Shader _shader;

        // Pipeline
        uint32_t _subpass = 0;
        undicht::vulkan::DescriptorSetLayout _descriptor_set_layout;
        undicht::vulkan::DescriptorSetCache _descriptor_cache;
        undicht::vulkan::Pipeline _pipeline;

        // renderer
        undicht::vulkan::Sampler _sampler;
        undicht::vulkan::UniformBuffer _global_uniform_buffer; // contains data like the camera matrices, player position, ...
        std::vector<undicht::vulkan::UniformBuffer> _per_chunk_uniform_buffer;
        int _last_used_chunk_ubo = -1;

      public:

        void init(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass);
        void cleanUp();

        void onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);

        void loadCamera(undicht::tools::PerspectiveCamera3D& camera);
        void draw(const WorldBuffer& world, const MaterialAtlas& materials, undicht::vulkan::CommandBuffer& cmd);
        void beginFrame();

        const undicht::vulkan::DescriptorSetLayout& getDescriptorSetLayout() const;

      protected:
        // private renderer functions 


        // creates the given number of (unused) per chunk ubos 
        void createPerChunkUBOs(uint32_t num);
    };

} // namespace cell

#endif // WORLD_RENDERER_H