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
#include "renderer/vulkan/render_target.h"

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
        undicht::vulkan::DescriptorSetLayout _global_descriptor_layout; // to bind the global uniform buffer
        undicht::vulkan::DescriptorSetCache _global_descriptor_cache;
        undicht::vulkan::DescriptorSet _global_descriptor_set;
        undicht::vulkan::CommandBuffer _draw_cmd;
        undicht::vulkan::Fence _render_finished_fence;
        undicht::vulkan::Semaphore _render_finished_semaphore;
        undicht::vulkan::UniformBuffer _global_uniform_buffer;

        // main stage
        undicht::vulkan::RenderTarget _main_render_target;
        WorldRenderer _world_renderer;
        LightRenderer _light_renderer;
        FinalRenderer _final_renderer;

      public:

        void init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain);
        void cleanUp();

        /// @brief to be called before rendering something
        /// @return returns false if the frame should be skipped because the swap chain is out of date
        bool beginFrame(undicht::vulkan::SwapChain& swap_chain);
        void endFrame(undicht::vulkan::SwapChain& swap_chain);

        void loadPlayerCamera(undicht::tools::PerspectiveCamera3D& cam);

        void beginGeometryStage(const MaterialAtlas& materials);
        void drawWorld(const WorldBuffer& world);

        void beginLightStage(const MaterialAtlas& materials);
        void drawLights(const LightBuffer& lights);

        void beginFinalStage(float exposure = 1.0f, float gamma = 2.2f);
        void drawFinal();

        void onSwapChainResize(undicht::vulkan::SwapChain& swap_chain);
      
      protected:
        // private functions

        void initMainRenderTarget(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain);

    };

} // namespace cell

#endif // CELL_MASTER_RENDERER_H