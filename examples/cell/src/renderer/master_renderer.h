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
#include "renderer/shadow_renderer.h"
#include "renderer/vulkan/texture.h"
#include "renderer/vulkan/render_target.h"

#include "renderer/light_buffer.h"
#include "entities/lights/direct_light.h"

#include "3D/camera/perspective_camera_3d.h"

namespace cell {

    class MasterRenderer {

      enum Pass {
        NO_PASS,
        SHADOW_PASS,
        MAIN_PASS
      };

      protected:

        VkExtent2D _viewport;

        // handles to other objects
        undicht::vulkan::LogicalDevice _device_handle;

        // managing frames
        undicht::vulkan::Semaphore _swap_image_ready;
        uint32_t _swap_image_id = -1;

        // used by all stages
        undicht::vulkan::DescriptorSetLayout _global_descriptor_layout; // used to bind the global uniform buffer
        undicht::vulkan::DescriptorSetCache _global_descriptor_cache;
        undicht::vulkan::DescriptorSet _global_descriptor_set;
        undicht::vulkan::CommandBuffer _draw_cmd;
        undicht::vulkan::Fence _render_finished_fence;
        undicht::vulkan::Semaphore _render_finished_semaphore;
        undicht::vulkan::UniformBuffer _global_uniform_buffer;

        // keeping track of which pass we are currently in
        Pass _current_pass = NO_PASS;

        // shadow pass
        const uint32_t _SHADOW_MAP_WIDTH = 1024;
        const uint32_t _SHADOW_MAP_HEIGHT = 1024;
        undicht::vulkan::RenderTarget _shadow_map_target;
        ShadowRenderer _shadow_renderer;

        // main pass
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

        // do before starting to render
        void loadPlayerCamera(undicht::tools::PerspectiveCamera3D& cam);
        void loadEnvironment(const std::string& file_name);

        // shadow pass
        void beginShadowPass(const DirectLight& global_shadow_source); // only one shadow source for now
        void drawToShadowMap(const WorldBuffer& world);

        // main pass
        void beginMainRenderPass();
        void beginGeometrySubPass(const MaterialAtlas& materials);
        void drawWorld(const WorldBuffer& world);
        void beginLightSubPass();
        void drawLights(const LightBuffer& lights);
        void drawLight(const DirectLight& light);
        void drawAmbientLight();
        void beginFinalSubPass();
        void drawFinal(float exposure = 1.0f);

        void onSwapChainResize(undicht::vulkan::SwapChain& swap_chain);
      
      protected:
        // private functions

        void initShadowRenderTarget(const undicht::vulkan::LogicalDevice& device, VkExtent2D extent,  uint32_t num_frames);
        void initMainRenderTarget(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain);

    };

} // namespace cell

#endif // CELL_MASTER_RENDERER_H