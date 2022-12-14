#include "renderer/master_renderer.h"
#include "core/vulkan/formats.h"

namespace cell {

    using namespace undicht;
    using namespace vulkan;

    const VkFormat DEPTH_BUFFER_FORMAT = VK_FORMAT_D32_SFLOAT;
    const VkFormat GEOM_BUFFER_FORMAT = VK_FORMAT_R8G8B8A8_UINT; // 2 uint8s for the material + 1 for the face id, 1 unused
    const VkFormat LIGHT_BUFFER_FORMAT = VK_FORMAT_R8G8B8A8_SRGB; 

    void MasterRenderer::init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain) {
        
        // storing handles
        _device_handle = device;

        initGlobalObjects(device, swap_chain);

        _world_renderer.init(device, _viewport, _render_pass, 0);
        _light_renderer.init(device, _viewport, _render_pass, 1);
        _final_renderer.init(device, _viewport, _render_pass, 2);
    }

    void MasterRenderer::cleanUp() {
        
        _world_renderer.cleanUp();
        _light_renderer.cleanUp();
        _final_renderer.cleanUp();
        cleanUpGlobalObjects();
    }

    bool MasterRenderer::beginFrame(undicht::vulkan::SwapChain& swap_chain) {

        // waiting for the previous frame to finish
        if(_swap_image_id != -1) 
            _render_finished_fence.waitForProcessToFinish();
        
        // acquiring an image to render to
        _swap_image_id = swap_chain.acquireNextSwapImage(_swap_image_ready.getAsSignal());

        // clearing the framebuffer
        undicht::vulkan::Framebuffer& frame_buffer = _frame_buffers.at(_swap_image_id);
        VkClearValue color_clear_value{0.01f, 0.01f, 0.01f, 1.0f};
        VkClearValue depth_clear_value{1.0f, 0};
        VkClearValue geom_clear_value{3, 0, 0, 0};
        VkClearValue light_clear_value{0.0f, 0.0f, 0.0f, 0.0f};


        // beginning the render pass
        _draw_cmd.resetCommandBuffer();
        _draw_cmd.beginCommandBuffer(true);
        _draw_cmd.beginRenderPass(_render_pass.getRenderPass(), frame_buffer.getFramebuffer(), _viewport, {color_clear_value, depth_clear_value, geom_clear_value, light_clear_value});

        return _swap_image_id != -1;
    }

    void MasterRenderer::endFrame(undicht::vulkan::SwapChain& swap_chain) {

        // ending the render pass
        _draw_cmd.endRenderPass();
        _draw_cmd.endCommandBuffer();

        // submit the draw command
        _device_handle.submitOnGraphicsQueue(_draw_cmd.getCommandBuffer(), _render_finished_fence.getFence(), {_swap_image_ready.getAsWaitOn()}, {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}, {_render_finished_semaphore.getAsSignal()});

        // submitting the image to be presented
        _device_handle.presentOnPresentQueue(swap_chain.getSwapchain(), _swap_image_id, {_render_finished_semaphore.getAsWaitOn()});
    }

    void MasterRenderer::loadPlayerCamera(undicht::tools::PerspectiveCamera3D& cam) {

        _world_renderer.loadCamera(cam);
        _light_renderer.loadCamera(cam);
        _final_renderer.loadCamera(cam);
    }

    void MasterRenderer::beginGeometryStage() {

        _world_renderer.beginFrame();
    }

    void MasterRenderer::drawWorld(const WorldBuffer& world, const MaterialAtlas& materials) {

        _world_renderer.draw(world, materials, _draw_cmd);
    }

    void MasterRenderer::beginLightStage() {

        _draw_cmd.nextSubPass(VK_SUBPASS_CONTENTS_INLINE);
        _light_renderer.beginFrame();
    }

    void MasterRenderer::drawLights(const LightBuffer& lights) {

        _light_renderer.draw(lights, _draw_cmd);
    }

    void MasterRenderer::beginFinalStage() {

        _draw_cmd.nextSubPass(VK_SUBPASS_CONTENTS_INLINE);
        _final_renderer.beginFrame();
    }

    void MasterRenderer::drawFinal(const MaterialAtlas& materials) {

        _final_renderer.draw(materials, _draw_cmd, _geom_buffers.at(_swap_image_id), _depth_buffers.at(_swap_image_id), _light_buffers.at(_swap_image_id));
    }

    void MasterRenderer::onSwapChainResize(undicht::vulkan::SwapChain& swap_chain) {
        
        _viewport = {(uint32_t)swap_chain.getExtent().width, (uint32_t)swap_chain.getExtent().height};

        resizeFrameBuffers(swap_chain);

        _world_renderer.onViewportResize(_device_handle, _viewport, _render_pass);
        _light_renderer.onViewportResize(_device_handle, _viewport, _render_pass);
        _final_renderer.onViewportResize(_device_handle, _viewport, _render_pass);

    }

    ///////////////////////////////////////// private functions /////////////////////////////////////////

    void MasterRenderer::initGlobalObjects(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain) {

        _viewport = {(uint32_t)swap_chain.getExtent().width, (uint32_t)swap_chain.getExtent().height};

        _swap_image_ready.init(device.getDevice());
        _draw_cmd.init(device.getDevice(), device.getGraphicsCmdPool());

        // init the render pass + its sub passess
        _render_pass.addAttachment(swap_chain.getSwapImageFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR); // final color image
        _render_pass.addAttachment(DEPTH_BUFFER_FORMAT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // internal depth buffer
        _render_pass.addAttachment(GEOM_BUFFER_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // internal color texture
        _render_pass.addAttachment(LIGHT_BUFFER_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // internal light map

        // geometry subpass
        _render_pass.addSubPass({1, 2}, {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

        // lighting subpass
        _render_pass.addSubPass({1, 3}, {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}, {2}, {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        _render_pass.addSubPassDependency(0, 1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

        // final subpass
        _render_pass.addSubPass({0}, {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}, {1, 2, 3}, {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        _render_pass.addSubPassDependency(1, 2, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

        _render_pass.init(device.getDevice());

        // init the framebuffers + its attachments
        initFrameBuffers(swap_chain);

        // signal objects
        _render_finished_fence.init(device.getDevice(), false);
        _render_finished_semaphore.init(device.getDevice());

    }

    void MasterRenderer::initFrameBuffers(undicht::vulkan::SwapChain& swap_chain) {

        _frame_buffers.resize(swap_chain.getSwapImageCount());
        _depth_buffers.resize(swap_chain.getSwapImageCount());
        _light_buffers.resize(swap_chain.getSwapImageCount());
        _geom_buffers.resize(swap_chain.getSwapImageCount());

        for (int i = 0; i < swap_chain.getSwapImageCount(); i++) {
            _depth_buffers.at(i).init(_device_handle.getDevice());
            _geom_buffers.at(i).init(_device_handle.getDevice());
            _light_buffers.at(i).init(_device_handle.getDevice());
        }

        resizeFrameBuffers(swap_chain);

    }

    void MasterRenderer::resizeFrameBuffers(undicht::vulkan::SwapChain& swap_chain) {

        for (int i = 0; i < swap_chain.getSwapImageCount(); i++) {
            _depth_buffers.at(i).allocate(_device_handle, swap_chain.getExtent().width, swap_chain.getExtent().height, 1, 1, 1, DEPTH_BUFFER_FORMAT);
            _geom_buffers.at(i).allocate(_device_handle, swap_chain.getExtent().width, swap_chain.getExtent().height, 1, 1, 1, GEOM_BUFFER_FORMAT);
            _light_buffers.at(i).allocate(_device_handle, swap_chain.getExtent().width, swap_chain.getExtent().height, 1, 1, 1, LIGHT_BUFFER_FORMAT);

            _frame_buffers.at(i).cleanUp();
            _frame_buffers.at(i).setAttachment(0, swap_chain.getSwapImageView(i));
            _frame_buffers.at(i).setAttachment(1, _depth_buffers.at(i).getImageView());
            _frame_buffers.at(i).setAttachment(2, _geom_buffers.at(i).getImageView());
            _frame_buffers.at(i).setAttachment(3, _light_buffers.at(i).getImageView());
            _frame_buffers.at(i).init(_device_handle.getDevice(), _render_pass, swap_chain.getExtent());
        }

    }

    void MasterRenderer::cleanUpGlobalObjects() {

        _render_finished_fence.cleanUp();
        _render_finished_semaphore.cleanUp();

        _draw_cmd.cleanUp();
        _swap_image_ready.cleanUp();

        _render_pass.cleanUp();

        for(undicht::vulkan::Framebuffer &fbo : _frame_buffers)
            fbo.cleanUp();

        for(undicht::vulkan::Image &depth : _depth_buffers)
            depth.cleanUp();

        for(undicht::vulkan::Image &light : _light_buffers)
            light.cleanUp();

        for(undicht::vulkan::Image &geom : _geom_buffers)
            geom.cleanUp();

    }

} // cell