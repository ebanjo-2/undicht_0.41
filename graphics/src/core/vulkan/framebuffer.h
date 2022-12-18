#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "vector"
#include "vulkan/vulkan.h"
#include "renderpass.h"

namespace undicht {

    namespace vulkan {

        class Framebuffer {

        protected:

            VkDevice _device_handle;

            VkFramebuffer _framebuffer;
            std::vector<VkImageView> _attachments;

            VkExtent2D _extent;

        public:

            /** attachments should be added before the framebuffer is initialized
             * the attachment type (color, depth/stencil attachment) is specified by the render_pass*/
            void setAttachment(int id, const VkImageView& image_view);

            void init(const VkDevice& device, const RenderPass& render_pass, VkExtent2D extent);
            void cleanUp();

            const VkFramebuffer& getFramebuffer() const;
            const VkExtent2D& getExtent() const;

        protected:
            // creating framebuffer related structs

            VkFramebufferCreateInfo static createFramebufferCreateInfo(VkRenderPass render_pass, const std::vector<VkImageView>& attachments, VkExtent2D extent);

        };

    } // vulkan

} // undicht

#endif // FRAMEBUFFER_H