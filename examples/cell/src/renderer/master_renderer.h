#ifndef CELL_MASTER_RENDERER_H
#define CELL_MASTER_RENDERER_H

#include "frame_manager.h"

#include "core/vulkan/semaphore.h"
#include "core/vulkan/fence.h"
#include "core/vulkan/logical_device.h"
#include "core/vulkan/swap_chain.h"
#include "core/vulkan/renderpass.h"
#include "core/vulkan/command_buffer.h"

#include "renderer/world_renderer.h"
#include "renderer/light_renderer.h"
#include "renderer/final_renderer.h"
#include "renderer/shadow_renderer.h"
#include "renderer/vulkan/texture.h"
#include "renderer/vulkan/render_target.h"
#include "renderer/vulkan/transfer_buffer.h"

#include "world/lights/light.h"
#include "world/lights/light_buffer.h"

#include "world/cells/cell_buffer.h"

#include "3D/camera/perspective_camera_3d.h"

#include "window/glfw/window.h"

namespace cell {

    class MasterRenderer : public undicht::FrameManager {

      enum Pass {
        NO_PASS,
        SHADOW_PASS,
        MAIN_PASS,
        IMGUI_PASS,
      };

      protected:

        VkExtent2D _viewport;

        // handles to other objects
        undicht::vulkan::LogicalDevice _device_handle;

        // used by all stages
        undicht::vulkan::DescriptorSetLayout _global_descriptor_layout; // used to bind the global uniform buffer
        undicht::vulkan::DescriptorSetCache _global_descriptor_cache;
        undicht::vulkan::DescriptorSet _global_descriptor_set;
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

        bool _enable_imgui = true;

      public:

        void init(const VkInstance& instance, undicht::graphics::Window& window, const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf, bool enable_imgui = true);
        void cleanUp();

        /// @brief to be called before rendering something
        /// @return returns false if the frame should be skipped because the swap chain is out of date
        bool beginFrame();
        void endFrame();

        // do before starting to render
        void loadPlayerCamera(undicht::tools::PerspectiveCamera3D& cam);

        // shadow pass
        void beginShadowPass(const Light& global_shadow_source, const glm::vec3& shadow_target); // only one shadow source for now
        void drawToShadowMap(const CellBuffer& world);
        void endShadowPass();

        // main pass
        void beginMainRenderPass();
        void beginGeometrySubPass(const MaterialAtlas& materials);
        void drawWorld(const CellBuffer& world);
        void beginLightSubPass();
        void drawLights(const LightBuffer& lights);
        void drawLight(const Light& light); // draw a directional light
        void drawAmbientLight(const Environment& env);
        void beginFinalSubPass();
        void drawFinal(float exposure = 1.0f);
        void endMainRenderPass();

        // imgui pass
        void beginImguiRenderPass();
        void drawImGui();
        void endImguiRenderPass();

        // other functions
        void onWindowResize(const undicht::graphics::Window& window);

      protected:
        // private functions

        void initShadowRenderTarget(const undicht::vulkan::LogicalDevice& device, VkExtent2D extent,  uint32_t num_frames);
        void initMainRenderTarget(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain);

    };

} // namespace cell

#endif // CELL_MASTER_RENDERER_H