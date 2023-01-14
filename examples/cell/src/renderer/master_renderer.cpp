#include "renderer/master_renderer.h"
#include "core/vulkan/formats.h"
#include "debug.h"

namespace cell {

    using namespace undicht;
    using namespace vulkan;

    void MasterRenderer::init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain) {
        
        // storing handles
        _device_handle = device;
        _viewport = {(uint32_t)swap_chain.getExtent().width, (uint32_t)swap_chain.getExtent().height};
        
        _swap_image_ready.init(device.getDevice());
        _draw_cmd.init(device.getDevice(), device.getGraphicsCmdPool());

        // signal objects
        _render_finished_fence.init(device.getDevice(), false);
        _render_finished_semaphore.init(device.getDevice());

        // global uniform buffer (contains global uniform data, such as the players camera)
        _global_uniform_buffer.init(device, BufferLayout({
            UND_MAT4F, // view
            UND_MAT4F, // proj
            UND_MAT4F, // inverse view
            UND_MAT4F, // inverse projection
        }));

        initMainRenderTarget(device, swap_chain);

        _world_renderer.init(device, _viewport, _main_render_target.getRenderPass(), 0);
        _light_renderer.init(device, _viewport, _main_render_target.getRenderPass(), 1);
        _final_renderer.init(device, _viewport, _main_render_target.getRenderPass(), 2);
    }

    void MasterRenderer::cleanUp() {
        
        _world_renderer.cleanUp();
        _light_renderer.cleanUp();
        _final_renderer.cleanUp();

        _global_uniform_buffer.cleanUp();

        _render_finished_fence.cleanUp();
        _render_finished_semaphore.cleanUp();

        _draw_cmd.cleanUp();
        _swap_image_ready.cleanUp();

        _main_render_target.cleanUp();
    }

    bool MasterRenderer::beginFrame(undicht::vulkan::SwapChain& swap_chain) {

        // waiting for the previous frame to finish
        if(_swap_image_id != -1) 
            _render_finished_fence.waitForProcessToFinish();
        
        // acquiring an image to render to
        _swap_image_id = swap_chain.acquireNextSwapImage(_swap_image_ready.getAsSignal());

        if(_swap_image_id == -1)
            return false;

        // clearing the framebuffer
        undicht::vulkan::Framebuffer& frame_buffer = _main_render_target.getFramebuffer(_swap_image_id);
        VkClearValue visible_clear_value{0.01f, 0.01f, 0.01f, 1.0f};
        VkClearValue depth_clear_value{1.0f, 0};
        VkClearValue position_clear_value{0.0f, 0.0f, 0.0f, 0.0f};
        VkClearValue normal_clear_value{0.0f, 0.0f, 0.0f, 0.0f};
        VkClearValue color_specular_clear_value{0.0f, 0.0f, 0.0f, 0.0f};
        VkClearValue light_clear_value{0.1f, 0.1f, 0.1f, 0.0f};

        std::vector<VkClearValue> clear_values = {
            visible_clear_value, 
            depth_clear_value, 
            position_clear_value, 
            normal_clear_value, 
            color_specular_clear_value, 
            light_clear_value,
        };

        // beginning the render pass
        _draw_cmd.resetCommandBuffer();
        _draw_cmd.beginCommandBuffer(true);
        _draw_cmd.beginRenderPass(_main_render_target.getRenderPass().getRenderPass(), frame_buffer.getFramebuffer(), _viewport, clear_values);

        return true;
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

        const glm::mat4& view = cam.getViewMatrix();
        const glm::mat4& proj = cam.getCameraProjectionMatrix();

        glm::mat4 inv_view = glm::inverse(view);
        glm::mat4 inv_proj = glm::inverse(proj);

        _global_uniform_buffer.setAttribute(0, glm::value_ptr(view), 16 * sizeof(float));
        _global_uniform_buffer.setAttribute(1, glm::value_ptr(proj), 16 * sizeof(float));
        _global_uniform_buffer.setAttribute(2, glm::value_ptr(inv_view), 16 * sizeof(float));
        _global_uniform_buffer.setAttribute(3, glm::value_ptr(inv_proj), 16 * sizeof(float));

    }

    void MasterRenderer::beginGeometryStage() {

        _world_renderer.beginFrame();
    }

    void MasterRenderer::drawWorld(const WorldBuffer& world, const MaterialAtlas& materials) {

        _world_renderer.draw(world, materials, _global_uniform_buffer, _draw_cmd);
    }

    void MasterRenderer::beginLightStage() {

        _draw_cmd.nextSubPass(VK_SUBPASS_CONTENTS_INLINE);
        _light_renderer.beginFrame();
    }

    void MasterRenderer::drawLights(const LightBuffer& lights) {

        const VkImageView& position = _main_render_target.getAttachment(_swap_image_id, 2);
        const VkImageView& normal = _main_render_target.getAttachment(_swap_image_id, 3);
        const VkImageView& color_specular = _main_render_target.getAttachment(_swap_image_id, 4);
        _light_renderer.draw(lights, _global_uniform_buffer, _draw_cmd, position, normal, color_specular);
    }

    void MasterRenderer::beginFinalStage() {

        _draw_cmd.nextSubPass(VK_SUBPASS_CONTENTS_INLINE);
        _final_renderer.beginFrame();
    }

    void MasterRenderer::drawFinal(const MaterialAtlas& materials, float exposure, float gamma) {

        const VkImageView& color_specular = _main_render_target.getAttachment(_swap_image_id, 4);
        const VkImageView& light = _main_render_target.getAttachment(_swap_image_id, 5);

        _final_renderer.draw(_global_uniform_buffer, _draw_cmd, exposure, gamma, color_specular, light);
    }

    void MasterRenderer::onSwapChainResize(undicht::vulkan::SwapChain& swap_chain) {
        
        _viewport = {(uint32_t)swap_chain.getExtent().width, (uint32_t)swap_chain.getExtent().height};

        _main_render_target.resize(_viewport, &swap_chain);

        _world_renderer.onViewportResize(_device_handle, _viewport, _main_render_target.getRenderPass());
        _light_renderer.onViewportResize(_device_handle, _viewport, _main_render_target.getRenderPass());
        _final_renderer.onViewportResize(_device_handle, _viewport, _main_render_target.getRenderPass());
    }

    ///////////////////////////////////////// private functions /////////////////////////////////////////
    
    void MasterRenderer::initMainRenderTarget(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain) {
        // using https://learnopengl.com/Advanced-Lighting/Deferred-Shading as a reference for setting up the geometry buffer

        const VkFormat DEPTH_BUFFER_FORMAT = translate(UND_DEPTH32F);
        const VkFormat POSITION_BUFFER_FORMAT = translate(UND_VEC4F16); // position relative to the camera
        const VkFormat NORMAL_BUFFER_FORMAT = translate(UND_VEC4F16); // normal
        const VkFormat COLOR_SPECULAR_BUFFER_FORMAT = translate(UND_R8G8B8A8); // albedo color + specular
        const VkFormat LIGHT_BUFFER_FORMAT = translate(UND_VEC4F16); // using 2 byte floats to store a higher range (hdr) 

        _main_render_target.setDeviceHandle(device, swap_chain.getSwapImageCount());
        _main_render_target.addVisibleAttachment(swap_chain, true, true); // 0
        _main_render_target.addAttachment(DEPTH_BUFFER_FORMAT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // 1
        _main_render_target.addAttachment(POSITION_BUFFER_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 2
        _main_render_target.addAttachment(NORMAL_BUFFER_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 3
        _main_render_target.addAttachment(COLOR_SPECULAR_BUFFER_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 4
        _main_render_target.addAttachment(LIGHT_BUFFER_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 5

        // geometry subpass
        _main_render_target.addSubPass(
            {1, 2, 3, 4},
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
        );
        // lighting subpass
        _main_render_target.addSubPass(
            {1, 5}, // uses the depth buffer as an "output", but doesnt write to it
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}, 
            {2, 3, 4},
            {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}
        );
        // lighting subpass waits for the geometry subpass at the fragment shader stage
        _main_render_target.addSubPassDependency(
            0,
            1,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT
        );
        // final subpass
        _main_render_target.addSubPass(
            {0}, // outputs only to the visible attachment
            {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}, 
            {4, 5}, // combines light and color 
            {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}
        );
        // final subpass waits for the lighting subpass at the fragment shader stage
        _main_render_target.addSubPassDependency(
            1, 
            2, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 
            VK_ACCESS_SHADER_READ_BIT
        );

        _main_render_target.init(swap_chain.getExtent(), &swap_chain);
    }

} // cell

/* future performance improvements:
* - put the information of all chunks within an ubo containing arrays
*   -> only one buffer write 
*
*/ 