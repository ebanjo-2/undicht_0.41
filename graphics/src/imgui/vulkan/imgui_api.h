#ifndef UNDICHT_IMGUI_API_H
#define UNDICHT_IMGUI_API_H

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "core/vulkan/logical_device.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace undicht {

    namespace vulkan {

        class ImGuiAPI {
        protected:

            static VkDescriptorPool _descriptor_pool;

        public:

            void static init(const VkInstance& instance, const LogicalDevice& device, const VkRenderPass& render_pass, GLFWwindow* window);
            void static cleanUp(const VkDevice& device);

        };

    } // vulkan

} // undicht

#endif // UNDICHT_IMGUI_API_H