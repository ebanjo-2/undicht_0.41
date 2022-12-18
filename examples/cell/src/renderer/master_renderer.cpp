#include "renderer/master_renderer.h"

namespace cell {

    void MasterRenderer::init(const undicht::vulkan::LogicalDevice& device, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {
        
        // storing handles
        _device_handle = device;
        _render_pass_handle = render_pass;
        _viewport = viewport;

        // swap image signals
        _swap_image_ready.init(device.getDevice());

        // geometry stage
        _geometry_draw_cmd.init(device.getDevice(), device.getGraphicsCmdPool());
        _world_renderer.init(device, viewport, render_pass);
        _geometry_descriptor_cache.init(device, _world_renderer.getDescriptorSetLayout());
        _geometry_stage_finished_fence.init(device.getDevice(), true);
        _geometry_stage_finished_semaphore.init(device.getDevice());

    }

    void MasterRenderer::cleanUp() {

        // other renderers

        // geometry stage
        _world_renderer.cleanUp();
        _geometry_draw_cmd.cleanUp();
        _geometry_descriptor_cache.cleanUp();
        _geometry_stage_finished_fence.cleanUp();
        _geometry_stage_finished_semaphore.cleanUp();

        // swap image signal
        _swap_image_ready.cleanUp();

    }

    void MasterRenderer::beginFrame(undicht::vulkan::SwapChain& swap_chain) {

        // waiting for the previous frame to finish
        undicht::vulkan::Fence& last_frame_finished = _geometry_stage_finished_fence; // might change with more stages
        last_frame_finished.waitForProcessToFinish();
        
        // acquiring an image to render to
        _swap_image_id = swap_chain.acquireNextSwapImage(_swap_image_ready.getAsSignal());
        
    }

    void MasterRenderer::endFrame(undicht::vulkan::SwapChain& swap_chain) {

        // submitting the image to be presented
        undicht::vulkan::Semaphore& last_render_stage_finished = _geometry_stage_finished_semaphore; // might change with more stages
        _device_handle.presentOnPresentQueue(swap_chain.getSwapchain(), _swap_image_id, {last_render_stage_finished.getAsWaitOn()});
    }

    void MasterRenderer::loadPlayerCamera(undicht::tools::PerspectiveCamera3D& cam) {

        _world_renderer.loadCamera(cam);

    }


    void MasterRenderer::beginGeometryStage(std::vector<undicht::vulkan::Framebuffer>& visible_frame_buffers) {

        undicht::vulkan::Framebuffer& frame_buffer = visible_frame_buffers.at(_swap_image_id);

        // reset geometry descriptor cache
        _geometry_descriptor_cache.reset();

        // reset world renderers ubos
        _world_renderer.resetPerChunkUBOs();

        // clearing the image
        VkClearValue color_clear_value{0.01f, 0.01f, 0.01f, 1.0f};
        VkClearValue depth_clear_value{1.0f, 0};

        // beginning the render pass
        _geometry_draw_cmd.resetCommandBuffer();
        _geometry_draw_cmd.beginCommandBuffer(true);
        _geometry_draw_cmd.beginRenderPass(_render_pass_handle.getRenderPass(), frame_buffer.getFramebuffer(), _viewport, {color_clear_value, depth_clear_value});
        
    }

    void MasterRenderer::drawWorld(const WorldBuffer& world) {

        _world_renderer.draw(world, _geometry_draw_cmd, _geometry_descriptor_cache);
    }

    void MasterRenderer::endGeometryStage() {

        // ending the render pass
        _geometry_draw_cmd.endRenderPass();
        _geometry_draw_cmd.endCommandBuffer();

        // submit the draw command
        _device_handle.submitOnGraphicsQueue(_geometry_draw_cmd.getCommandBuffer(), _geometry_stage_finished_fence.getFence(), {_swap_image_ready.getAsWaitOn()}, {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}, {_geometry_stage_finished_semaphore.getAsSignal()});

    }

    void MasterRenderer::onSwapChainResize(undicht::vulkan::SwapChain& swap_chain, const undicht::vulkan::RenderPass& render_pass) {
        
        _viewport = {(uint32_t)swap_chain.getExtent().width, (uint32_t)swap_chain.getExtent().height};

        _world_renderer.onViewportResize(_device_handle, _viewport, render_pass);
    }


} // cell