#ifndef RENDERER_H
#define RENDERER_H

#include "string"

#include "core/vulkan/logical_device.h"
#include "core/vulkan/renderpass.h"
#include "core/vulkan/descriptor_set_layout.h"
#include "core/vulkan/pipeline.h"
#include "core/vulkan/shader.h"

#include "renderer/vulkan/descriptor_set_cache.h"

namespace undicht {

    namespace vulkan {

        class Renderer {
        /// @brief class that combines a pipeline, a shader and a descriptor cache
        /// other Renderers can inherit some common functions from this one
        protected:
        
            // handles to other objects
            undicht::vulkan::LogicalDevice _device_handle;
            undicht::vulkan::RenderPass _render_pass_handle;

            // Shader
            undicht::vulkan::Shader _shader;

            // Pipeline
            uint32_t _sub_pass = 0;
            undicht::vulkan::DescriptorSetLayout _descriptor_set_layout;
            undicht::vulkan::DescriptorSetCache _descriptor_cache;
            undicht::vulkan::Pipeline _pipeline;

        public:

            // first thing to initialize!!!!
            void setDeviceHandle(const undicht::vulkan::LogicalDevice& gpu);

            // settings that have(!) to be set before the pipeline is initialized
            virtual void setShaders(const std::string& vertex_shader, const std::string& fragment_shader);
            virtual void setDescriptorSetLayout(const std::vector<VkDescriptorType>& binding_types);
            virtual void setVertexInputLayout(const undicht::BufferLayout& vertex_layout, const undicht::BufferLayout& instance_layout = {});
            virtual void setDepthStencilTest(bool enable_depth_test = true, bool write_depth_values = true);
            virtual void setRasterizer(bool enable_culling, bool cull_ccw_faces = false, bool wire_frame = false);
            virtual void setInputAssembly(VkPrimitiveTopology topology);

            virtual void setBlending(uint32_t attachment, bool enable_blending, VkBlendOp color_blend_op = {}, VkBlendOp alpha_blend_op = {});

            // init / cleanUp
            virtual void init(VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t sub_pass = 0);
            virtual void cleanUp();

            // settings that can be changed after the pipeline is initialized
            virtual void resizeViewport(VkExtent2D viewport);

        };

    } // namespace vulkan

} // namespace undicht

#endif // RENDERER_H