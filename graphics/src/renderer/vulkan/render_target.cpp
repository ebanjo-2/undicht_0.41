#include "render_target.h"
#include "debug.h"

namespace undicht {

    namespace vulkan {

        /////////////////////////////////// first thing to initialize!!!! ////////////////////////////////////////

        void RenderTarget::setDeviceHandle(const undicht::vulkan::LogicalDevice &gpu, uint32_t num_frames) {

            _device_handle = gpu;

            _frame_data.resize(num_frames);
        }

        //////////////////////// settings that have(!) to be set before the pipeline is initialized //////////////

        int RenderTarget::addAttachment(VkFormat format, VkImageLayout final_layout, uint32_t samples, bool clear_before_rendering, bool store_result) {
            /** @brief declares a single attachment (i.e. an image that stores color information)
             * @return the index with which the attachment can be accessed (counting up from 0) */

            if (!_frame_data.size()) {
                UND_ERROR << "Failed to add Attachment to RenderTarget, please set the number of frames to smth. bigger than 0 by calling setDeviceHandle() \n";
                return -1;
            }

            int attachment_id = _render_pass.addAttachment(format, final_layout, samples, clear_before_rendering, store_result);

            for (FrameData &f : _frame_data) {
                Image new_image;
                new_image.init(_device_handle.getDevice());
                f._attachments.push_back(new_image);
                f._attachment_formats.push_back(format);
                f._attachment_ids.push_back(attachment_id);
            }

            return attachment_id;
        }

        int RenderTarget::addVisibleAttachment(const SwapChain& swap_chain, bool clear_before_rendering, bool store_result, VkImageLayout output_layout) {
            
            if (_frame_data.size() != swap_chain.getSwapImageCount()) {
                UND_ERROR << "Failed to add visible Attachment to RenderTarget, please set the number of frames equal to the number of swap images in the swap chain \n";
                return -1;
            }

            int attachment_id = _render_pass.addAttachment(swap_chain.getSwapImageFormat(), output_layout, 1, clear_before_rendering, store_result);

            for(int i = 0; i <  _frame_data.size(); i++) {
                _frame_data.at(i)._visible_attachment_id = attachment_id;
                _frame_data.at(i)._visible_attachment = swap_chain.getSwapImageView(i);
            }

            return attachment_id;
        }


        int RenderTarget::addSubPass(const std::vector<uint32_t> &output_attachments, const std::vector<VkImageLayout> &output_layouts, const std::vector<uint32_t> &input_attachments, const std::vector<VkImageLayout> &input_layouts, VkPipelineBindPoint bind_point) {
            /** @brief declares the input + output for a single rendering step
             * before the rendering begins, the attachments will be transitioned to the layout declared in attachment_layouts */

            return _render_pass.addSubPass(output_attachments, output_layouts, input_attachments, input_layouts, bind_point);
        }

        void RenderTarget::addSubPassDependency(uint32_t src_subpass, uint32_t dst_subpass, const VkPipelineStageFlags &src_stage, const VkPipelineStageFlags &dst_stage, const VkAccessFlags &src_access, const VkAccessFlags &dst_access) {
            // thanks for the explanation: https://www.reddit.com/r/vulkan/comments/s80reu/subpass_dependencies_what_are_those_and_why_do_i/
            /// @brief tells the gpu to wait executing a subpass until another has finished (only needed if more than one subpass is used during rendering)
            /// @param src_subpass the subpass that should be waited on
            /// @param dst_subpass the subpass that should wait
            /// @param src_stage the stage that has to finish in the src_subpass
            /// @param dst_stage the stage the dst_subpass should wait at
            /// @param src_access memory access types used by src_subpass
            /// @param dst_access memory access types used by dst_subpass

            _render_pass.addSubPassDependency(src_subpass, dst_subpass, src_stage, dst_stage, src_access, dst_access);
        }

        //////////////////////////////////////////// init / cleanUp //////////////////////////////////////////////

        void RenderTarget::init(const VkExtent2D &extent, const SwapChain* swap_chain) {
            // the swap_chain only needs to be provided if the rendertarget contains visible swap images

            if(_device_handle.getDevice() == VK_NULL_HANDLE)
                UND_ERROR << "failed to init Rendertarget because the device handle wasnt set (as the first thing!!!!!)\n";

            _render_pass.init(_device_handle.getDevice());

            resize(extent, swap_chain);
        }

        void RenderTarget::cleanUp() {

            for (FrameData &f : _frame_data) {
                f._frame_buffer.cleanUp();
                for (Image &i : f._attachments)
                    i.cleanUp();
            }

            _render_pass.cleanUp();
        }

        ////////////////////////////// call before / after drawing to the render target //////////////////////////////

        void RenderTarget::beginRenderPass(CommandBuffer& draw_cmd, uint32_t frame_id, const std::vector<VkClearValue>& clear_values) {

            draw_cmd.beginRenderPass(_render_pass.getRenderPass(), getFramebuffer(frame_id).getFramebuffer(), getExtent(), clear_values);
        }

        void RenderTarget::endRenderPass(CommandBuffer& draw_cmd) {

            draw_cmd.endRenderPass();
        }

        ////////////////////////// settings that can be changed after the pipeline is initialized /////////////////

        void RenderTarget::resize(const VkExtent2D &extent, const SwapChain* swap_chain) {

            _extent = extent;

            // resizing the attachments and reattaching them to the framebuffer
            for (int i = 0; i < _frame_data.size(); i++) {
                
                FrameData& frame = _frame_data.at(i);
                Framebuffer& fbo = frame._frame_buffer;

                fbo.cleanUp();

                // reattaching the visible attachment
                if(frame._visible_attachment_id != -1) {

                    if(!swap_chain) {
                        UND_ERROR << "Failed to resize RenderTarget: there was a visible attachment attached previously, but no swapchain was provided for the resizing\n";
                        return;
                    }

                    frame._visible_attachment = swap_chain->getSwapImageView(i);
                    fbo.setAttachment(frame._visible_attachment_id, frame._visible_attachment);
                }

                // reattaching the other attachments
                for (int j = 0; j < frame._attachments.size(); j++) {
                    Image &image = frame._attachments.at(j);
                    image.allocate(_device_handle, _extent.width, _extent.height, 1, 1, 1, frame._attachment_formats.at(j));
                    fbo.setAttachment(frame._attachment_ids.at(j), image.getImageView());
                }

                fbo.init(_device_handle.getDevice(), _render_pass, _extent);
            }
        }

        //////////////////////////////////////////// getters ////////////////////////////////////////////

        const RenderPass &RenderTarget::getRenderPass() const {

            return _render_pass;
        }

        Framebuffer &RenderTarget::getFramebuffer(uint32_t frame) {

            return _frame_data.at(frame)._frame_buffer;
        }

        const VkImageView& RenderTarget::getAttachment(uint32_t frame, uint32_t attachment) const {

            if(_frame_data.at(frame)._visible_attachment_id == attachment) {
                return _frame_data.at(frame)._visible_attachment;
            } else {
                for(int i = 0; i <  _frame_data.at(frame)._attachments.size(); i++) {
                    if(_frame_data.at(frame)._attachment_ids.at(i) == attachment)
                        return _frame_data.at(frame)._attachments.at(i).getImageView();
                }
            }

            UND_ERROR << "failed to return attachment " << attachment << " for frame " << frame << ", no such attachment found\n";

            return VK_NULL_HANDLE;
        }

        const VkImage& RenderTarget::getLocalAttachment(uint32_t frame, uint32_t attachment) const {
            // cant return extern attachments

            for(int i = 0; i <  _frame_data.at(frame)._attachments.size(); i++) {

                if(_frame_data.at(frame)._attachment_ids.at(i) == attachment)
                    return _frame_data.at(frame)._attachments.at(i).getImage();

            }

            UND_ERROR << "failed to return local attachment " << attachment << " for frame " << frame << ", no such attachment found\n";
            return VK_NULL_HANDLE;
        } 


        VkExtent2D RenderTarget::getExtent() const {
            
            return _extent;
        }


    } // namespace vulkan

} // namespace undicht