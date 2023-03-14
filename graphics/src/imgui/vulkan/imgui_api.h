#ifndef UNDICHT_IMGUI_API_H
#define UNDICHT_IMGUI_API_H

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "core/vulkan/logical_device.h"
#include "core/vulkan/command_buffer.h"
#include "renderer/vulkan/render_target.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace undicht {

    namespace vulkan {

        class ImGuiAPI {
        protected:

            static LogicalDevice _device_handle;
            static VkDescriptorPool _descriptor_pool;
            static RenderTarget _render_target;

        public:

            void static init(const VkInstance& instance, const LogicalDevice& device, const SwapChain& swap_chain, GLFWwindow* window);
            void static cleanUp();

            void static newFrame();
            void static endFrame(); // not necessary if render was called

            void static render(uint32_t frame, CommandBuffer& draw_cmd);

            void static onViewportResize(const SwapChain& swap_chain);

        };

    } // vulkan

} // undicht

#endif // UNDICHT_IMGUI_API_H