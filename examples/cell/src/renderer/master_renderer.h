#ifndef CELL_MASTER_RENDERER_H
#define CELL_MASTER_RENDERER_H

#include "core/vulkan/semaphore.h"
#include "core/vulkan/fence.h"
#include "core/vulkan/logical_device.h"
#include "core/vulkan/swap_chain.h"
#include "core/vulkan/renderpass.h"

#include "renderer/world_renderer.h"

#include "3D/camera/perspective_camera_3d.h"

namespace cell {

    class MasterRenderer {

      protected:

        // handles to other objects
        undicht::vulkan::LogicalDevice _device_handle;
        undicht::vulkan::RenderPass _render_pass_handle;

        // controlling the render target
        undicht::vulkan::Semaphore _swap_image_ready;
        uint32_t _swap_image_id = 0;
        VkExtent2D _viewport;

        // geometry stage
        WorldRenderer _world_renderer;
        undicht::vulkan::DescriptorSetCache _geometry_descriptor_cache;
        undicht::vulkan::CommandBuffer _geometry_draw_cmd;
        undicht::vulkan::Fence _geometry_stage_finished_fence;
        undicht::vulkan::Semaphore _geometry_stage_finished_semaphore;


      public:

        void init(const undicht::vulkan::LogicalDevice& device, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass);
        void cleanUp();

        void beginFrame(undicht::vulkan::SwapChain& swap_chain);
        void endFrame(undicht::vulkan::SwapChain& swap_chain);

        void loadPlayerCamera(undicht::tools::PerspectiveCamera3D& cam);

        void beginGeometryStage(std::vector<undicht::vulkan::Framebuffer>& visible_frame_buffers);
        void drawWorld(const WorldBuffer& world);
        void endGeometryStage();

        void onSwapChainResize(undicht::vulkan::SwapChain& swap_chain, const undicht::vulkan::RenderPass& render_pass);

    };

} // namespace cell

#endif // CELL_MASTER_RENDERER_H