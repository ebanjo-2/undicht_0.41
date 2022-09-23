#include "debug.h"
#include "framebuffer.h"

namespace undicht {

    namespace vulkan {


        void Framebuffer::setAttachment(int id, const VkImageView& image_view) {
            /** attachments should be added before the framebuffer is initialized
            * the attachment type (color, depth/stencil attachment) is specified by the render_pass*/

            if(_attachments.size() <= id)
                _attachments.resize(id + 1, {});

            _attachments.at(id) = image_view;
        }

        void Framebuffer::init(const VkDevice& device, const RenderPass& render_pass, VkExtent2D extent) {

            _device_handle = device;

            if(render_pass.getAttachmentCount() != _attachments.size()) {
                UND_ERROR << "failed to create framebuffer: added attachments dont match the ones specified by the render_pass\n";
                return;
            }

            VkFramebufferCreateInfo info = createFramebufferCreateInfo(render_pass.getRenderPass(), _attachments, extent);
            vkCreateFramebuffer(device, &info, {}, &_framebuffer);

        }

        void Framebuffer::cleanUp() {

            vkDestroyFramebuffer(_device_handle, _framebuffer, {});

        }

        const VkFramebuffer& Framebuffer::getFramebuffer() const {

            return _framebuffer;
        }

        /////////////////////////////// creating framebuffer related structs ///////////////////////////////


        VkFramebufferCreateInfo Framebuffer::createFramebufferCreateInfo(VkRenderPass render_pass, const std::vector<VkImageView>& attachments, VkExtent2D extent) {

            VkFramebufferCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            info.pNext = nullptr;
            info.renderPass = render_pass;
            info.attachmentCount = attachments.size();
            info.pAttachments = attachments.data();
            info.width = extent.width;
            info.height = extent.height;
            info.layers = 1;

            return info;
        }


    } // vulkan

} // undicht