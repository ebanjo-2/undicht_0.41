#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include "logical_device.h"

namespace undicht {

    namespace vulkan {

        class CommandBuffer {

        protected:

            VkCommandBuffer _cmd_buffer;

        public:

            void init(const VkDevice& device, const VkCommandPool& command_pool);
            void cleanUp();

            const VkCommandBuffer& getCommandBuffer() const;

            // recording commands into the buffer
            void resetCommandBuffer();
            void beginCommandBuffer(bool single_use);
            void endCommandBuffer();

            // graphics commands
            void beginRenderPass(const VkRenderPass& render_pass, const VkFramebuffer& frame_buffer, VkExtent2D extent, const VkClearValue& clear_value = {0.0f, 0.0f, 0.0f, 1.0f});
            void endRenderPass();
            void bindGraphicsPipeline(const VkPipeline& pipeline);
            void bindVertexBuffer(const VkBuffer& buffer, uint32_t binding);
            void bindIndexBuffer(const VkBuffer& buffer);
            void draw(uint32_t vertex_count, bool draw_indexed = false, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0);
            
            // other commands
            void copy(const VkBuffer& src, const VkBuffer& dst, const VkBufferCopy& copy_region);

        protected:
            // creating command buffer related structs

            VkCommandBufferAllocateInfo static createCommandBufferAllocateInfo(const VkCommandPool& command_pool, uint32_t count = 1);
            VkCommandBufferBeginInfo static createCommandBufferBeginInfo(bool single_use);
            VkRenderPassBeginInfo static createRenderPassBeginInfo(const VkRenderPass& render_pass, const VkFramebuffer& frame_buffer, VkExtent2D extent, const VkClearValue& clear_value);
            
        };

    } // vulkan

} // undicht

#endif // COMMAND_BUFFER_H