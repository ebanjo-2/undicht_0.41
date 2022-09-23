#include "command_buffer.h"

namespace undicht {

    namespace vulkan {

        void CommandBuffer::init(const VkDevice& device, const VkCommandPool& command_pool) {

            // allocating the command buffer from the command pool
            VkCommandBufferAllocateInfo allocate_info = createCommandBufferAllocateInfo(command_pool, 1);
            vkAllocateCommandBuffers(device, &allocate_info, &_cmd_buffer);

        }

        void CommandBuffer::cleanUp() {

            // the command buffer gets destroyed with the command pool

        }

        const VkCommandBuffer& CommandBuffer::getCommandBuffer() const {

            return _cmd_buffer;
        }

        void CommandBuffer::resetCommandBuffer() {

            vkResetCommandBuffer(_cmd_buffer, 0);
        }

        void CommandBuffer::beginCommandBuffer(bool single_use) {

            VkCommandBufferBeginInfo info = createCommandBufferBeginInfo(single_use);
            vkBeginCommandBuffer(_cmd_buffer, &info);

        }

        void CommandBuffer::endCommandBuffer() {

            vkEndCommandBuffer(_cmd_buffer);

        }

        void CommandBuffer::beginRenderPass(const VkRenderPass& render_pass, const VkFramebuffer& frame_buffer, VkExtent2D extent, const VkClearValue& clear_value) {

            VkRenderPassBeginInfo info = createRenderPassBeginInfo(render_pass, frame_buffer, extent, clear_value);
            vkCmdBeginRenderPass(_cmd_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);

        }

        void CommandBuffer::endRenderPass() {

            vkCmdEndRenderPass(_cmd_buffer);
        }

        void CommandBuffer::bindGraphicsPipeline(const VkPipeline& pipeline) {

            vkCmdBindPipeline(_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        }

        void CommandBuffer::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {

            vkCmdDraw(_cmd_buffer, vertex_count, instance_count, first_vertex, first_instance);
        }


        /////////////////////////////// creating command buffer related structs ///////////////////////////////

        VkCommandBufferAllocateInfo CommandBuffer::createCommandBufferAllocateInfo(const VkCommandPool& command_pool, uint32_t count) {

            VkCommandBufferAllocateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.pNext = nullptr;
            info.commandPool = command_pool;
            info.commandBufferCount = count;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            return info;
        }

        VkCommandBufferBeginInfo CommandBuffer::createCommandBufferBeginInfo(bool single_use) {

            VkCommandBufferBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.pNext = nullptr;
            info.pInheritanceInfo = nullptr;

            if(single_use)
                info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            return info;
        }

        VkRenderPassBeginInfo CommandBuffer::createRenderPassBeginInfo(const VkRenderPass& render_pass, const VkFramebuffer& frame_buffer, VkExtent2D extent, const VkClearValue& clear_value) {

            VkRenderPassBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.pNext = nullptr;
            info.renderPass = render_pass;
            info.renderArea.offset.x = 0;
            info.renderArea.offset.y = 0;
            info.renderArea.extent = extent;
            info.framebuffer = frame_buffer;
            info.clearValueCount = 1;
            info.pClearValues = &clear_value;

            return info;
        }


    } // vulkan

} // undicht