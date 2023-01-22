#ifndef SHADOW_RENDERER_H
#define SHADOW_RENDERER_H

#include "renderer/vulkan/renderer.h"
#include "renderer/vulkan/uniform_buffer.h"
#include "entities/lights/direct_light.h"
#include "renderer/world_buffer.h"

namespace cell {

    class ShadowRenderer {
        // draws geometry to a shadow map (depth buffer)
      protected:

        undicht::vulkan::LogicalDevice _device_handle;

        undicht::vulkan::Renderer _renderer;
        undicht::vulkan::UniformBuffer _local_ubo;
        std::vector<undicht::vulkan::UniformBuffer> _per_chunk_uniform_buffer;
        int _last_used_chunk_ubo = -1;

      public:

        void init(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass);
        void cleanUp();

        void onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);

        void beginFrame(const DirectLight& light, undicht::vulkan::CommandBuffer& cmd); // only one direct light for now
        void draw(const WorldBuffer& world, undicht::vulkan::CommandBuffer& cmd); // draw to shadow map
    
      protected:
        // private renderer functions 

        // creates the given number of (unused) per chunk ubos 
        void createPerChunkUBOs(uint32_t num);
    };

} // cell

#endif // SHADOW_RENDERER_H