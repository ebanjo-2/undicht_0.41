#include "renderpass.h"

namespace undicht {

    namespace vulkan {

        int RenderPass::addOutputAttachment(VkFormat format, VkImageLayout final_layout, uint32_t samples, bool clear_before_rendering, bool store_result) {
            /** declares a single output attachment (i.e. an image that stores color information)*/

            _attachment_descriptions.push_back(createAttachmentDescription(format, final_layout, samples, clear_before_rendering, store_result));

            return _attachment_descriptions.size() - 1;
        }


        int RenderPass::addSubPass(const std::vector<int>& used_attachments, const std::vector<VkImageLayout>& attachment_layouts, VkPipelineBindPoint bind_point) {
            /** declares the output for a single rendering step (whatever that means)
            * one sub-pass is enough for most applications
            * before the rendering begins, the attachments will be transitioned to the layout declared in attachment_layouts */

            // creating the attachment references
            _sub_pass_color_attachments.push_back(std::vector<VkAttachmentReference>());
            _sub_pass_depth_attachments.push_back({VK_ATTACHMENT_UNUSED});

            for(int i = 0; i < used_attachments.size(); i++) {

                if(attachment_layouts[i] == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                    _sub_pass_color_attachments.back().push_back(createAttachmentReference(used_attachments[i], attachment_layouts[i]));
                else if(attachment_layouts[i] == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                    _sub_pass_depth_attachments.back() = createAttachmentReference(used_attachments[i], attachment_layouts[i]);

            }

            // creating the subpass description
            VkSubpassDescription description = createSubPassDescription(_sub_pass_color_attachments.back(), _sub_pass_depth_attachments.back(), bind_point);
            _sub_pass_descriptions.push_back(description);

            return _sub_pass_descriptions.size() - 1;
        }


        void RenderPass::init(const VkDevice& device) {

            _device_handle = device;

            VkRenderPassCreateInfo info = createRenderPassCreateInfo(_attachment_descriptions, _sub_pass_descriptions);
            vkCreateRenderPass(device, &info, {}, &_render_pass);

        }

        void RenderPass::cleanUp() {

            vkDestroyRenderPass(_device_handle, _render_pass, {});

        }

        const VkRenderPass& RenderPass::getRenderPass() const {

            return _render_pass;
        }

        int RenderPass::getAttachmentCount() const {

            return _attachment_descriptions.size();
        }

        ////////////////////////////// creating render-pass related structs //////////////////////////////

        VkAttachmentDescription RenderPass::createAttachmentDescription(VkFormat format, VkImageLayout final_layout, uint32_t samples, bool clear_before_rendering, bool store_result) {

            VkAttachmentDescription attachment{};
            attachment.format = format;
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;

            if(clear_before_rendering)
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            else
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

            if(store_result)
                attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            else
                attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;

            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachment.finalLayout = final_layout;

            return attachment;
        }

        VkAttachmentReference RenderPass::createAttachmentReference(int attachment, VkImageLayout layout) {

            VkAttachmentReference reference{};
            reference.attachment = attachment;
            reference.layout = layout;

            return reference;
        }

        VkSubpassDescription RenderPass::createSubPassDescription(const std::vector<VkAttachmentReference>& color_attachments, const VkAttachmentReference& depth_attachment, VkPipelineBindPoint bind_point) {

            // creating the sub-pass description
            VkSubpassDescription description{};
            description.pipelineBindPoint = bind_point;
            description.colorAttachmentCount = color_attachments.size();
            description.pColorAttachments = color_attachments.data();
            description.pDepthStencilAttachment = &depth_attachment;

            return description;
        }

        VkRenderPassCreateInfo RenderPass::createRenderPassCreateInfo(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses) {

            VkRenderPassCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            info.attachmentCount = attachments.size();
            info.pAttachments = attachments.data();
            info.subpassCount = subpasses.size();
            info.pSubpasses = subpasses.data();

            return info;
        }

    } // vulkan

} // undicht