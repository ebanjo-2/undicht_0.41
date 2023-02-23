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
#include "renderer/vulkan/renderer.h"
#include "3D/camera/perspective_camera_3d.h"
#include "materials/material_atlas.h"

#include "world/cells/cell_buffer.h"

namespace cell {

    class WorldRenderer {

      protected:

        undicht::vulkan::LogicalDevice _device_handle;

        // renderer
        undicht::vulkan::Sampler _tile_map_sampler;
        undicht::vulkan::Renderer _renderer;
        undicht::vulkan::UniformBuffer _local_uniform_buffer;
        std::vector<undicht::vulkan::UniformBuffer> _per_chunk_uniform_buffer;
        int _last_used_chunk_ubo = -1;

      public:

        void init(const undicht::vulkan::LogicalDevice& gpu, const undicht::vulkan::DescriptorSetLayout& global_descriptor_layout, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass);
        void cleanUp();

        void onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);
        
        void beginFrame(const MaterialAtlas& materials, const undicht::vulkan::DescriptorSet& global_descriptor_set, undicht::vulkan::CommandBuffer& cmd);
        void draw(const CellBuffer& world, undicht::vulkan::CommandBuffer& cmd);

      protected:
        // private renderer functions 

        // creates the given number of (unused) per chunk ubos 
        void createPerChunkUBOs(uint32_t num);
    };

} // namespace cell

#endif // WORLD_RENDERER_H