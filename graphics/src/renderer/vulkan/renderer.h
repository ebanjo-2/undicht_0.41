#ifndef RENDERER_H
#define RENDERER_H

#include "string"

#include "core/vulkan/logical_device.h"
#include "core/vulkan/renderpass.h"
#include "core/vulkan/descriptor_set_layout.h"
#include "core/vulkan/pipeline.h"
#include "core/vulkan/shader.h"
#include "core/vulkan/command_buffer.h"

#include "renderer/vulkan/descriptor_set_cache.h"
#include "renderer/vulkan/vertex_buffer.h"

namespace undicht {

    namespace vulkan {

        const extern BufferLayout SCREEN_QUAD_VERTEX_LAYOUT;
        const extern std::vector<float> SCREEN_QUAD_VERTICES;
        const extern BufferLayout SKY_BOX_VERTEX_LAYOUT;
        const extern std::vector<float> SKY_BOX_VERTICES;

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
            std::vector<bool> _extern_layouts;
            std::vector<DescriptorSetLayout> _descriptor_set_layouts;
            std::vector<uint32_t> _descriptor_set_pool_sizes;
            undicht::vulkan::DescriptorSetCache _descriptor_cache;
            undicht::vulkan::Pipeline _pipeline;

            std::vector<undicht::vulkan::DescriptorSet*> _descriptor_sets;

            uint32_t _frame_id = 0;

        public:

            // first thing to initialize!!!!
            void setDeviceHandle(const undicht::vulkan::LogicalDevice& gpu);

            // settings that have(!) to be set before the pipeline is initialized
            virtual void setShaders(const std::string& vertex_shader, const std::string& fragment_shader);
            virtual void setDescriptorSetLayout(const std::vector<VkDescriptorType>& binding_types, uint32_t slot = 0, uint32_t descriptor_pool_size = 1000);
            virtual void setDescriptorSetLayout(const undicht::vulkan::DescriptorSetLayout& layout, uint32_t slot = 0, uint32_t descriptor_pool_size = 1000);
            virtual void setVertexInputLayout(const undicht::BufferLayout& vertex_layout, const undicht::BufferLayout& instance_layout = {});
            virtual void setDepthStencilTest(bool enable_depth_test = true, bool write_depth_values = true, VkCompareOp compare_op = VK_COMPARE_OP_LESS);
            virtual void setRasterizer(bool enable_culling, bool cull_ccw_faces = false, bool wire_frame = false);
            virtual void setInputAssembly(VkPrimitiveTopology topology);

            virtual void setBlending(uint32_t attachment, bool enable_blending, VkBlendOp color_blend_op = {}, VkBlendFactor src_color_factor = {}, VkBlendFactor dst_color_factor = {}, VkBlendOp alpha_blend_op = {}, VkBlendFactor src_alpha_factor = {}, VkBlendFactor dst_alpha_factor = {});

            // init / cleanUp
            virtual void init(VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t sub_pass = 0, uint32_t num_frames = 1);
            virtual void cleanUp();

            // settings that can be changed after the pipeline is initialized
            virtual void resizeViewport(VkExtent2D viewport);

            // binding descriptors
            void resetDescriptorCache(uint32_t slot);
            void accquireDescriptorSet(uint32_t slot);
            void bindUniformBuffer(uint32_t descriptor_set_slot, uint32_t binding, const Buffer& buffer);
            void bindImage(uint32_t descriptor_set_slot, uint32_t binding, const VkImageView& image_view, const VkImageLayout& layout, const VkSampler& sampler);
            void bindInputAttachment(uint32_t descriptor_set_slot, uint32_t binding, const VkImageView& image_view);

            // drawing
            void beginFrame(uint32_t frame_id);
            void bindPipeline(undicht::vulkan::CommandBuffer& cmd);
            void bindDescriptorSet(undicht::vulkan::CommandBuffer& cmd, uint32_t slot = 0);
            void bindDescriptorSet(undicht::vulkan::CommandBuffer& cmd, const undicht::vulkan::DescriptorSet& descriptor_set, uint32_t slot = 0); // bind an external descriptor set
            void bindVertexBuffer(undicht::vulkan::CommandBuffer& cmd, const undicht::vulkan::VertexBuffer& vbo, bool bind_index_buffer = false, bool bind_instance_buffer = false);
            void draw(undicht::vulkan::CommandBuffer& cmd, uint32_t vertex_count, bool draw_indexed = false, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0);

            // getters
            const undicht::vulkan::DescriptorSetLayout& getDescriptorSetLayout(uint32_t slot = 0) const;
            VkViewport getViewport() const;

        };

    } // namespace vulkan

} // namespace undicht

#endif // RENDERER_H