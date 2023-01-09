#ifndef CELL_MASTER_RENDERER_H
#define CELL_MASTER_RENDERER_H

#include "core/vulkan/semaphore.h"
#include "core/vulkan/fence.h"
#include "core/vulkan/logical_device.h"
#include "core/vulkan/swap_chain.h"
#include "core/vulkan/renderpass.h"

#include "renderer/world_renderer.h"
#include "renderer/light_renderer.h"
#include "renderer/final_renderer.h"
#include "renderer/vulkan/texture.h"

#include "entities/light_buffer.h"

#include "3D/camera/perspective_camera_3d.h"

namespace cell {

    class MasterRenderer {

      protected:

        VkExtent2D _viewport;

        // handles to other objects
        undicht::vulkan::LogicalDevice _device_handle;

        // managing frames
        undicht::vulkan::Semaphore _swap_image_ready;
        uint32_t _swap_image_id = -1;

        // used by all stages
        std::vector<undicht::vulkan::Framebuffer> _frame_buffers;
        std::vector<undicht::vulkan::Image> _depth_buffers;
        std::vector<undicht::vulkan::Image> _light_buffers;
        std::vector<undicht::vulkan::Image> _geom_buffers;
        undicht::vulkan::RenderPass _render_pass; // contains sub passes for each of the stages
        undicht::vulkan::CommandBuffer _draw_cmd;
        undicht::vulkan::Fence _render_finished_fence;
        undicht::vulkan::Semaphore _render_finished_semaphore;

        // geometry stage
        WorldRenderer _world_renderer;

        // lighting stage
        LightRenderer _light_renderer;

        // final stage
        FinalRenderer _final_renderer;

      public:

        void init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain);
        void cleanUp();

        /// @brief to be called before rendering something
        /// @return returns false if the frame should be skipped because the swap chain is out of date
        bool beginFrame(undicht::vulkan::SwapChain& swap_chain);
        void endFrame(undicht::vulkan::SwapChain& swap_chain);

        void loadPlayerCamera(undicht::tools::PerspectiveCamera3D& cam);

        void beginGeometryStage();
        void drawWorld(const WorldBuffer& world, const MaterialAtlas& materials);

        void beginLightStage();
        void drawLights(const LightBuffer& lights);

        void beginFinalStage();
        void drawFinal(const MaterialAtlas& materials);

        void onSwapChainResize(undicht::vulkan::SwapChain& swap_chain);
      
      protected:
        // private functions

        void initGlobalObjects(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain);
        void initFrameBuffers(undicht::vulkan::SwapChain& swap_chain);
        void resizeFrameBuffers(undicht::vulkan::SwapChain& swap_chain);
        void cleanUpGlobalObjects();


    };

} // namespace cell

#endif // CELL_MASTER_RENDERER_H