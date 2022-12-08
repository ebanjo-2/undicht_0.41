#ifndef WORLD_RENDERER_H
#define WORLD_RENDERER_H

#include "core/vulkan/shader.h"
#include "core/vulkan/descriptor_set_layout.h"
#include "core/vulkan/pipeline.h"
#include "core/vulkan/sampler.h"
#include "core/vulkan/swap_chain.h"
#include "core/vulkan/renderpass.h"
#include "renderer/vulkan/uniform_buffer.h"
#include "3D/camera/perspective_camera_3d.h"

namespace cell {

    class WorldRenderer {

      protected:
        // Shader
        undicht::vulkan::Shader _shader;

        // Pipeline
        undicht::vulkan::DescriptorSetLayout _descriptor_set_layout;
        undicht::vulkan::Pipeline _pipeline;

        // renderer
        undicht::vulkan::Sampler _sampler;
        undicht::vulkan::UniformBuffer _uniform_buffer;

      public:

        void init(const undicht::vulkan::LogicalDevice& gpu, const undicht::vulkan::SwapChain& swap_chain, const undicht::vulkan::RenderPass& render_pass);
        void cleanUp();

        void loadCamera(undicht::tools::PerspectiveCamera3D& camera);

      protected:
        // private renderer functions 

        void static setVertexBinding(uint32_t id, uint32_t location_offset, const undicht::BufferLayout& layout, undicht::vulkan::Pipeline& pipeline);

    };

} // namespace cell

#endif // WORLD_RENDERER_H