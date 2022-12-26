#include "renderer/master_renderer.h"

namespace cell {

    void MasterRenderer::init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain) {
        
        // storing handles
        _device_handle = device;

        initGlobalObjects(device, swap_chain);
        initGeometryStage(device);
    }

    void MasterRenderer::cleanUp() {
        
        cleanUpGeometryStage();
        cleanUpGlobalObjects();
    }

    bool MasterRenderer::beginFrame(undicht::vulkan::SwapChain& swap_chain) {

        // waiting for the previous frame to finish
        if(_swap_image_id != -1) {
            undicht::vulkan::Fence& last_frame_finished = _geometry_stage_finished_fence; // might change with more stages
            last_frame_finished.waitForProcessToFinish();
        }
        
        // acquiring an image to render to
        _swap_image_id = swap_chain.acquireNextSwapImage(_swap_image_ready.getAsSignal());

        return _swap_image_id != -1;
    }

    void MasterRenderer::endFrame(undicht::vulkan::SwapChain& swap_chain) {

        // submitting the image to be presented
        undicht::vulkan::Semaphore& last_render_stage_finished = _geometry_stage_finished_semaphore; // might change with more stages
        _device_handle.presentOnPresentQueue(swap_chain.getSwapchain(), _swap_image_id, {last_render_stage_finished.getAsWaitOn()});
    }

    void MasterRenderer::loadPlayerCamera(undicht::tools::PerspectiveCamera3D& cam) {

        _world_renderer.loadCamera(cam);

    }


    void MasterRenderer::beginGeometryStage() {

        undicht::vulkan::Framebuffer& frame_buffer = _frame_buffers.at(_swap_image_id);


        // clearing the image
        VkClearValue color_clear_value{0.01f, 0.01f, 0.01f, 1.0f};
        VkClearValue depth_clear_value{1.0f, 0};

        // beginning the render pass
        _draw_cmd.resetCommandBuffer();
        _draw_cmd.beginCommandBuffer(true);
        _draw_cmd.beginRenderPass(_render_pass.getRenderPass(), frame_buffer.getFramebuffer(), _viewport, {color_clear_value, depth_clear_value});

        _world_renderer.beginFrame();
    }

    void MasterRenderer::drawWorld(const WorldBuffer& world, const MaterialAtlas& materials) {

        _world_renderer.draw(world, materials, _draw_cmd);
    }

    void MasterRenderer::endGeometryStage() {
        
        // ending the render pass
        _draw_cmd.endRenderPass();
        _draw_cmd.endCommandBuffer();

        // submit the draw command
        _device_handle.submitOnGraphicsQueue(_draw_cmd.getCommandBuffer(), _geometry_stage_finished_fence.getFence(), {_swap_image_ready.getAsWaitOn()}, {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}, {_geometry_stage_finished_semaphore.getAsSignal()});

    }

    void MasterRenderer::onSwapChainResize(undicht::vulkan::SwapChain& swap_chain) {
        
        _viewport = {(uint32_t)swap_chain.getExtent().width, (uint32_t)swap_chain.getExtent().height};

        for (int i = 0; i < swap_chain.getSwapImageCount(); i++) {
            
            // resize the framebuffers attachments
            _depth_buffers.at(i).allocate(_device_handle, _viewport.width, _viewport.height, 1, 1, 1, VK_FORMAT_D32_SFLOAT);

            // reattaching the attachments
            _frame_buffers.at(i).cleanUp();
            _frame_buffers.at(i).setAttachment(0, swap_chain.getSwapImageView(i));
            _frame_buffers.at(i).setAttachment(1, _depth_buffers.at(i).getImageView());
            _frame_buffers.at(i).init(_device_handle.getDevice(), _render_pass, swap_chain.getExtent());
        }

        _world_renderer.onViewportResize(_device_handle, _viewport, _render_pass);
    }

    ///////////////////////////////////////// private functions /////////////////////////////////////////

    void MasterRenderer::initGlobalObjects(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain) {

        _viewport = {(uint32_t)swap_chain.getExtent().width, (uint32_t)swap_chain.getExtent().height};

        _swap_image_ready.init(device.getDevice());
        _draw_cmd.init(device.getDevice(), device.getGraphicsCmdPool());

        // init the render pass + its sub passess
        _render_pass.addOutputAttachment(swap_chain.getSwapImageFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        _render_pass.addOutputAttachment(VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        // geometry subpass

        // lighting subpass

        // final subpass
        _render_pass.addSubPass({0, 1}, {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});

        _render_pass.init(device.getDevice());

        // init the framebuffers + its attachments
        _frame_buffers.resize(swap_chain.getSwapImageCount());
        _depth_buffers.resize(swap_chain.getSwapImageCount());

        for (int i = 0; i < swap_chain.getSwapImageCount(); i++) {
            // init the depth buffer
            _depth_buffers.at(i).init(device.getDevice());
            _depth_buffers.at(i).allocate(device, _viewport.width, _viewport.height, 1, 1, 1, VK_FORMAT_D32_SFLOAT);

            // init the framebuffer
            _frame_buffers.at(i).setAttachment(0, swap_chain.getSwapImageView(i));
            _frame_buffers.at(i).setAttachment(1, _depth_buffers.at(i).getImageView());
            _frame_buffers.at(i).init(device.getDevice(), _render_pass, _viewport);
        }

    }

    void MasterRenderer::cleanUpGlobalObjects() {

        _draw_cmd.cleanUp();
        _swap_image_ready.cleanUp();

        _render_pass.cleanUp();

        for(undicht::vulkan::Framebuffer &fbo : _frame_buffers)
            fbo.cleanUp();

        for(undicht::vulkan::Image &depth : _depth_buffers)
            depth.cleanUp();

    }

    void MasterRenderer::initGeometryStage(const undicht::vulkan::LogicalDevice& device) {

        _world_renderer.init(device, _viewport, _render_pass);
        _geometry_stage_finished_fence.init(device.getDevice(), false);
        _geometry_stage_finished_semaphore.init(device.getDevice());
    }

    void MasterRenderer::cleanUpGeometryStage() {

        _world_renderer.cleanUp();
        _geometry_stage_finished_fence.cleanUp();
        _geometry_stage_finished_semaphore.cleanUp();

    }

} // cell