#ifndef TONK_RENDERER_H
#define TONK_RENDERER_H

#include "vector"
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "core/vulkan/command_buffer.h"
#include "core/vulkan/semaphore.h"
#include "core/vulkan/fence.h"
#include "core/vulkan/swap_chain.h"
#include "core/vulkan/framebuffer.h"
#include "core/vulkan/sampler.h"
#include "core/vulkan/pipeline.h"
#include "core/vulkan/shader.h"
#include "core/vulkan/descriptor_set_layout.h"
#include "renderer/vulkan/texture.h"
#include "renderer/vulkan/descriptor_set_cache.h"
#include "renderer/vulkan/uniform_buffer.h"

#include "imgui/vulkan/imgui_api.h"
#include "glm/glm.hpp"

#include "map/map.h"
#include "map/tile_map.h"


namespace tonk {

    class Renderer {

    protected:

        // sync objects
        undicht::vulkan::Semaphore _swap_image_ready;
        undicht::vulkan::Fence _render_finished_fence;
        undicht::vulkan::Semaphore _render_finished;

        int _swap_image_id = 0;

        // draw command
        undicht::vulkan::CommandBuffer _draw_cmd;
        undicht::vulkan::Sampler _sampler;

        // handles
        undicht::vulkan::LogicalDevice _device;
        std::vector<undicht::vulkan::Framebuffer>* _visible_framebuffers;
        VkRenderPass _render_pass;

        // map pipeline
        undicht::vulkan::Shader _map_shader;
        undicht::vulkan::DescriptorSetLayout _map_descriptor_set_layout;
        undicht::vulkan::DescriptorSetCache _map_descriptor_cache;
        undicht::vulkan::Pipeline _map_pipeline;
        undicht::vulkan::UniformBuffer _map_ubo;

    public:

        void init(VkInstance instance, const undicht::vulkan::LogicalDevice& device, std::vector<undicht::vulkan::Framebuffer>* visible_framebuffers, undicht::vulkan::SwapChain& swap_chain, VkRenderPass render_pass, VkExtent2D viewport, const GLFWwindow* window);
        void cleanUp();

        void beginFrame(undicht::vulkan::SwapChain& swap_chain);
        void drawMap(undicht::vulkan::SwapChain& swap_chain, const Map& map, const TileMap& tile_map, glm::vec2 map_center, float zoom_factor = 0.04);
        void drawImGui();
        void endFrame(undicht::vulkan::SwapChain& swap_chain);

        void onWindowResize(undicht::vulkan::SwapChain& swap_chain);

        VkDescriptorSet createImGuiTexture(const undicht::vulkan::Texture& texture);

    protected:
        // private functions

        void initMapPipeline(VkExtent2D viewport);

    };

} // tonk

#endif // TONK_RENDERER_H