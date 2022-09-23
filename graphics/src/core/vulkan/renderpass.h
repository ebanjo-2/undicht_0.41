#ifndef RENDER_PASS_H
#define RENDER_PASS_H

#include "vector"
#include "vulkan/vulkan.h"

namespace undicht {

    namespace vulkan {

        class RenderPass {
            /** a RenderPass contains information about the targets of the rendering process
             * (the images, their formats, the state they will be in, ...)*/

        protected:

            VkDevice _device_handle;

            VkRenderPass _render_pass = VK_NULL_HANDLE;
            std::vector<VkAttachmentDescription> _attachment_descriptions; // all attachments used by the render-pass
            std::vector<VkSubpassDescription> _sub_pass_descriptions;
            std::vector<std::vector<VkAttachmentReference>> _sub_pass_color_attachments;
            std::vector<VkAttachmentReference> _sub_pass_depth_attachments;

        public:

            /** declares a single output attachment (i.e. an image that stores color information)
             * @return the index with which the attachment can be accessed (counting up from 0)*/
            int addOutputAttachment(VkFormat format, VkImageLayout final_layout, uint32_t samples = 1, bool clear_before_rendering = true, bool store_result = true);

            /** declares the output for a single rendering step (whatever that means)
             * one sub-pass is enough for most applications
             * before the rendering begins, the attachments will be transitioned to the layout declared in attachment_layouts */
            int addSubPass(const std::vector<int>& used_attachments, const std::vector<VkImageLayout>& attachment_layouts, VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS);

            /** once all output attachments and sub-passes are declared, call init */
            void init(const VkDevice& device);
            void cleanUp();

            const VkRenderPass& getRenderPass() const;
            int getAttachmentCount() const;

        protected:
            // creating render-pass related structs

            VkAttachmentDescription static createAttachmentDescription(VkFormat format, VkImageLayout final_layout, uint32_t samples, bool clear_before_rendering, bool store_result);
            VkAttachmentReference static createAttachmentReference(int attachment, VkImageLayout layout);
            VkSubpassDescription static createSubPassDescription(const std::vector<VkAttachmentReference>& color_attachments, const VkAttachmentReference& depth_attachment, VkPipelineBindPoint bind_point);
            VkRenderPassCreateInfo static createRenderPassCreateInfo(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses);

        };

    } // vulkan

} // undicht

#endif // RENDER_PASS_H