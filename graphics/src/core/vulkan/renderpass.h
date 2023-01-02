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

            struct SubPass {
                std::vector<uint32_t> _input_attachments;
                std::vector<VkImageLayout> _input_layouts;
                std::vector<uint32_t> _output_attachments;
                std::vector<VkImageLayout> _output_layouts;
                VkPipelineBindPoint _bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
            };

        protected:

            VkDevice _device_handle;

            VkRenderPass _render_pass = VK_NULL_HANDLE;
            std::vector<VkAttachmentDescription> _attachment_descriptions; // all attachments used by the render-pass
            
            std::vector<SubPass> _sub_passes;
            std::vector<VkSubpassDependency> _sub_pass_dependencies;

        public:

            /** declares a single attachment (i.e. an image that stores color information)
             * @return the index with which the attachment can be accessed (counting up from 0)*/
            int addAttachment(VkFormat format, VkImageLayout final_layout, uint32_t samples = 1, bool clear_before_rendering = true, bool store_result = true);

            /** declares the output for a single rendering step
             * before the rendering begins, the attachments will be transitioned to the layout declared in attachment_layouts */
            int addSubPass(const std::vector<uint32_t>& output_attachments, const std::vector<VkImageLayout>& output_layouts, const std::vector<uint32_t>& input_attachments = {}, const std::vector<VkImageLayout>& input_layouts = {}, VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS);

            // thanks for the explanation: https://www.reddit.com/r/vulkan/comments/s80reu/subpass_dependencies_what_are_those_and_why_do_i/
            /// @brief tells the gpu to wait executing a subpass until another has finished (only needed if more than one subpass is used during rendering)
            /// @param src_subpass the subpass that should be waited on
            /// @param dst_subpass the subpass that should wait
            /// @param src_stage the stage that has to finish in the src_subpass
            /// @param dst_stage the stage the dst_subpass should wait at
            /// @param src_access memory access types used by src_subpass
            /// @param dst_access memory access types used by dst_subpass
            void addSubPassDependency(uint32_t src_subpass, uint32_t dst_subpass, const VkPipelineStageFlags& src_stage, const VkPipelineStageFlags& dst_stage, const VkAccessFlags& src_access, const VkAccessFlags& dst_access);

            /** once all output attachments and sub-passes are declared, call init */
            void init(const VkDevice& device);
            void cleanUp();

            const VkRenderPass& getRenderPass() const;
            int getAttachmentCount() const;

        protected:
            // creating render-pass related structs

            VkAttachmentDescription static createAttachmentDescription(VkFormat format, VkImageLayout final_layout, uint32_t samples, bool clear_before_rendering, bool store_result);
            VkAttachmentReference static createAttachmentReference(int attachment, VkImageLayout layout);
            VkSubpassDescription static createSubPassDescription(const std::vector<VkAttachmentReference>& color_attachments, const VkAttachmentReference& depth_attachment, const std::vector<VkAttachmentReference>& input_attachments, VkPipelineBindPoint bind_point);
            VkSubpassDependency static createSubPassDependency(uint32_t src_sub_pass, uint32_t dst_sub_pass, const VkPipelineStageFlags& src_stage, const VkPipelineStageFlags& dst_stage, const VkAccessFlags& src_access, const VkAccessFlags& dst_access);
            VkRenderPassCreateInfo static createRenderPassCreateInfo(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies);

        };

    } // vulkan

} // undicht

#endif // RENDER_PASS_H