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

#include "renderer/world_buffer.h"

namespace cell {

    class WorldRenderer {

      protected:

        // handles to other objects
        undicht::vulkan::RenderPass _render_pass_handle;

        // Shader
        undicht::vulkan::Shader _shader;

        // Pipeline
        undicht::vulkan::DescriptorSetLayout _descriptor_set_layout;
        undicht::vulkan::Pipeline _pipeline;

        // renderer
        undicht::vulkan::Sampler _sampler;
        undicht::vulkan::UniformBuffer _uniform_buffer;

      public:

        void init(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);
        void cleanUp();

        void onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);

        void loadCamera(undicht::tools::PerspectiveCamera3D& camera);
        void draw(const WorldBuffer& world, undicht::vulkan::CommandBuffer& cmd, undicht::vulkan::DescriptorSetCache& descriptor_set_cache);

        const undicht::vulkan::DescriptorSetLayout& getDescriptorSetLayout() const;

      protected:
        // private renderer functions 

        void static setVertexBinding(uint32_t id, uint32_t location_offset, const undicht::BufferLayout& layout, undicht::vulkan::Pipeline& pipeline);

    };

} // namespace cell

#endif // WORLD_RENDERER_H