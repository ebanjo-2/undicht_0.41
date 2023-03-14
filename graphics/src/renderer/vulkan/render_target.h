#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include "vector"

#include "core/vulkan/renderpass.h"
#include "core/vulkan/framebuffer.h"
#include "core/vulkan/image.h"
#include "core/vulkan/swap_chain.h"

namespace undicht {

    namespace vulkan {

        class RenderTarget {
            /// @brief a class that combines a renderpass, a framebuffer and its attachments
          protected:
            // internal struct

            struct FrameData {
                Framebuffer _frame_buffer;
                VkImageView _visible_attachment = VK_NULL_HANDLE;
                uint32_t _visible_attachment_id = -1;
                std::vector<undicht::vulkan::Image> _attachments;
                std::vector<VkFormat> _attachment_formats;
                std::vector<uint32_t> _attachment_ids;
            };

          protected:

            undicht::vulkan::LogicalDevice _device_handle;

            RenderPass _render_pass;
            std::vector<FrameData> _frame_data;
            VkExtent2D _extent;

          public:

            // first thing to initialize!!!!
            void setDeviceHandle(const undicht::vulkan::LogicalDevice& gpu, uint32_t num_frames);

            // settings that have(!) to be set before the render target is initialized
            int addAttachment(VkFormat format, VkImageLayout final_layout, uint32_t samples = 1, bool clear_before_rendering = true, bool store_result = true);
            int addVisibleAttachment(const SwapChain& swap_chain, bool clear_before_rendering = true, bool store_result = true, VkImageLayout output_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            int addSubPass(const std::vector<uint32_t>& output_attachments, const std::vector<VkImageLayout>& output_layouts, const std::vector<uint32_t>& input_attachments = {}, const std::vector<VkImageLayout>& input_layouts = {}, VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS);
            void addSubPassDependency(uint32_t src_subpass, uint32_t dst_subpass, const VkPipelineStageFlags& src_stage, const VkPipelineStageFlags& dst_stage, const VkAccessFlags& src_access, const VkAccessFlags& dst_access);

            // init / cleanUp
            // the swap_chain only needs to be provided if the rendertarget contains visible swap images
            void init(const VkExtent2D &extent, const SwapChain* swap_chain = nullptr);
            void cleanUp();

            // settings that can be changed after the render target is initialized
            void resize(const VkExtent2D& extent, const SwapChain* swap_chain = nullptr);

            // getters
            const RenderPass& getRenderPass() const;
            Framebuffer& getFramebuffer(uint32_t frame);
            const VkImageView& getAttachment(uint32_t frame, uint32_t attachment) const;
            const VkImage& getLocalAttachment(uint32_t frame, uint32_t attachment) const; // cant return extern attachments
            VkExtent2D getExtent() const;

        };

    } // namespace vulkan

} // namespace undicht

#endif // RENDER_TARGET