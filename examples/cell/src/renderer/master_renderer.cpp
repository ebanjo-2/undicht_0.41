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
            UND_MAT4F, // 0 view
            UND_MAT4F, // 1 proj
            UND_MAT4F, // 2 inverse view
            UND_MAT4F, // 3 inverse projection
            UND_VEC2F, // 4 viewport size
            UND_VEC2F, // 5 inverse viewport size
            UND_MAT4F, // 6 shadow view matrix
            UND_MAT4F, // 7 shadow proj matrix
        }));

        // global descriptor set
        _global_descriptor_layout.setBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // global ubo
        _global_descriptor_layout.init(device.getDevice());
        _global_descriptor_cache.init(device, {_global_descriptor_layout}, {1});

        initShadowRenderTarget(device, {_SHADOW_MAP_WIDTH, _SHADOW_MAP_HEIGHT}, swap_chain.getSwapImageCount());
        initMainRenderTarget(device, swap_chain);

        _shadow_renderer.init(device, {_SHADOW_MAP_WIDTH, _SHADOW_MAP_HEIGHT}, _shadow_map_target.getRenderPass(), 0);
        _world_renderer.init(device, _global_descriptor_layout, _viewport, _main_render_target.getRenderPass(), 0);
        _light_renderer.init(device, _global_descriptor_layout, _viewport, _main_render_target.getRenderPass(), 1);
        _final_renderer.init(device, _viewport, _main_render_target.getRenderPass(), 2);
    }

    void MasterRenderer::cleanUp() {
        
        _shadow_renderer.cleanUp();
        _world_renderer.cleanUp();
        _light_renderer.cleanUp();
        _final_renderer.cleanUp();

        _global_descriptor_cache.cleanUp();
        _global_descriptor_layout.cleanUp();
        _global_uniform_buffer.cleanUp();

        _render_finished_fence.cleanUp();
        _render_finished_semaphore.cleanUp();

        _draw_cmd.cleanUp();
        _swap_image_ready.cleanUp();

        _shadow_map_target.cleanUp();
        _main_render_target.cleanUp();
    }

    bool MasterRenderer::beginFrame(undicht::vulkan::SwapChain& swap_chain) {

        _global_descriptor_cache.reset({0});
        _global_descriptor_set = _global_descriptor_cache.accquire(0);

        // waiting for the previous frame to finish
        if(_swap_image_id != -1) 
            _render_finished_fence.waitForProcessToFinish();
        
        // acquiring an image to render to
        _swap_image_id = swap_chain.acquireNextSwapImage(_swap_image_ready.getAsSignal());

        if(_swap_image_id == -1)
            return false;

        // beginning the render pass
        _draw_cmd.resetCommandBuffer();
        _draw_cmd.beginCommandBuffer(true);

        // bind global ubo in case no shadow pass is started, in which case the ubo will be rebound
        _global_descriptor_set.bindUniformBuffer(0, _global_uniform_buffer.getBuffer());

        return true;
    }

    void MasterRenderer::endFrame(undicht::vulkan::SwapChain& swap_chain) {

        _current_pass = NO_PASS;

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

    void MasterRenderer::loadEnvironment(const std::string& file_name) {

        _light_renderer.loadEnvironment(file_name);
    }

    void MasterRenderer::beginShadowPass(const DirectLight& global_shadow_source) {

        _current_pass = SHADOW_PASS;

        // loading the global shadow source
        _global_uniform_buffer.setAttribute(6, glm::value_ptr(global_shadow_source.getShadowView()), 16 * sizeof(float));
        _global_uniform_buffer.setAttribute(7, glm::value_ptr(global_shadow_source.getShadowProj()), 16 * sizeof(float));

        undicht::vulkan::Framebuffer& frame_buffer = _shadow_map_target.getFramebuffer(_swap_image_id);
        VkClearValue depth_clear_value{1.0f, 0};

        _draw_cmd.beginRenderPass(_shadow_map_target.getRenderPass().getRenderPass(), frame_buffer.getFramebuffer(), _shadow_map_target.getExtent(), {depth_clear_value});
        _shadow_renderer.beginFrame(global_shadow_source, _draw_cmd, _global_descriptor_set);

    }

    void MasterRenderer::drawToShadowMap(const WorldBuffer& world) {
        
        _shadow_renderer.draw(world, _draw_cmd);
    }

    void MasterRenderer::beginMainRenderPass() {

        // end shadow pass
        if(_current_pass == SHADOW_PASS)
            _draw_cmd.endRenderPass();

        _current_pass = MAIN_PASS;

        // clearing the framebuffer
        undicht::vulkan::Framebuffer& frame_buffer = _main_render_target.getFramebuffer(_swap_image_id);
        VkClearValue visible_clear_value{0.01f, 0.01f, 0.01f, 1.0f};
        VkClearValue depth_clear_value{1.0f, 0};
        VkClearValue material_clear_value{255, 255, 0, 0};
        VkClearValue normal_clear_value{0.0f, 0.0f, 0.0f, 0.0f};
        VkClearValue light_clear_value{0.0f, 0.0f, 0.0f, 0.0f};
        VkClearValue shadow_pos_clear_value{0.0f, 0.0f, 0.0f, 0.0f};

        std::vector<VkClearValue> clear_values = {
            visible_clear_value,
            depth_clear_value,
            material_clear_value,
            normal_clear_value,
            light_clear_value,
            shadow_pos_clear_value
        };

        // begin main pass
        _draw_cmd.beginRenderPass(_main_render_target.getRenderPass().getRenderPass(), frame_buffer.getFramebuffer(), _viewport, clear_values);

    }

    void MasterRenderer::beginGeometrySubPass(const MaterialAtlas& materials) {

        _world_renderer.beginFrame(materials, _global_descriptor_set, _draw_cmd);
    }

    void MasterRenderer::drawWorld(const WorldBuffer& world) {

        _world_renderer.draw(world, _draw_cmd);
    }

    void MasterRenderer::beginLightSubPass(const MaterialAtlas& materials) {

        _draw_cmd.nextSubPass(VK_SUBPASS_CONTENTS_INLINE);

        const VkImageView& material = _main_render_target.getAttachment(_swap_image_id, 2);
        const VkImageView& normal = _main_render_target.getAttachment(_swap_image_id, 3);
        const VkImageView& shadow_map_pos = _main_render_target.getAttachment(_swap_image_id, 5);

        _light_renderer.beginFrame(materials, _global_descriptor_set, _draw_cmd, material, normal, shadow_map_pos);

    }

    void MasterRenderer::drawLights(const LightBuffer& lights) {

        _light_renderer.draw(lights, _draw_cmd);
    }

    void MasterRenderer::drawLight(const DirectLight& light) {

        const VkImageView& shadow_map = _shadow_map_target.getAttachment(_swap_image_id, 0);
        const VkImageLayout shadow_map_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        _light_renderer.draw(light, _draw_cmd, shadow_map, shadow_map_layout, _SHADOW_MAP_WIDTH, _SHADOW_MAP_HEIGHT);
    }

    void MasterRenderer::drawAmbientLight(const glm::vec3& ambient_light) {

        _light_renderer.draw(ambient_light, _draw_cmd);
    }


    void MasterRenderer::beginFinalSubPass() {

        _draw_cmd.nextSubPass(VK_SUBPASS_CONTENTS_INLINE);

        const VkImageView& light_hdr = _main_render_target.getAttachment(_swap_image_id, 4);

        _final_renderer.beginFrame(_draw_cmd, light_hdr);
    }

    void MasterRenderer::drawFinal(float exposure) {

        _final_renderer.draw(_draw_cmd, exposure);
    }

    void MasterRenderer::onSwapChainResize(undicht::vulkan::SwapChain& swap_chain) {
        
        _viewport = {(uint32_t)swap_chain.getExtent().width, (uint32_t)swap_chain.getExtent().height};

        float inv_viewport[] = {1.0f / _viewport.width, 1.0f / _viewport.height};
        _global_uniform_buffer.setAttribute(4, &_viewport, sizeof(_viewport));
        _global_uniform_buffer.setAttribute(5, inv_viewport, 2 * sizeof(float));

        _main_render_target.resize(_viewport, &swap_chain);

        _world_renderer.onViewportResize(_device_handle, _viewport, _main_render_target.getRenderPass());
        _light_renderer.onViewportResize(_device_handle, _viewport, _main_render_target.getRenderPass());
        _final_renderer.onViewportResize(_device_handle, _viewport, _main_render_target.getRenderPass());
    }

    ///////////////////////////////////////// private functions /////////////////////////////////////////

    void MasterRenderer::initShadowRenderTarget(const undicht::vulkan::LogicalDevice& device, VkExtent2D extent, uint32_t num_frames) {

        const VkFormat SHADOW_MAP_FORMAT = translate(UND_DEPTH32F);

        _shadow_map_target.setDeviceHandle(device, num_frames);
        _shadow_map_target.addAttachment(SHADOW_MAP_FORMAT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 1, true, true); // 0
        // Attachment will be transitioned to shader read at render pass end
        // thanks to https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp

        _shadow_map_target.addSubPass({0}, {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});

        // Use subpass dependencies for layout transitions
        // from the depth stencil layout to a layout that is readable in a shader
        _shadow_map_target.addSubPassDependency(
            VK_SUBPASS_EXTERNAL, 0,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
        );
        _shadow_map_target.addSubPassDependency(
            0, VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT
        );

        _shadow_map_target.init(extent);
    }

    void MasterRenderer::initMainRenderTarget(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain) {
        // using https://learnopengl.com/Advanced-Lighting/Deferred-Shading as a reference for setting up the geometry buffer

        const VkFormat DEPTH_BUFFER_FORMAT = translate(UND_DEPTH32F);
        const VkFormat MATERIAL_BUFFER_FORMAT = translate(UND_R8G8B8A8); // material + cell_uv
        const VkFormat NORMAL_BUFFER_FORMAT = translate(UND_VEC4F); // normal (32 bit floats needed for the depth stored in the alpha)
        const VkFormat LIGHT_BUFFER_FORMAT = translate(UND_VEC4F16); // using 2 byte floats to store a higher range (hdr)
        const VkFormat SHADOW_MAP_POS_FORMAT = translate(UND_VEC4F16); // pos + depth on shadow map

        _main_render_target.setDeviceHandle(device, swap_chain.getSwapImageCount());
        _main_render_target.addVisibleAttachment(swap_chain, true, true); // 0
        _main_render_target.addAttachment(DEPTH_BUFFER_FORMAT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // 1
        _main_render_target.addAttachment(MATERIAL_BUFFER_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 2
        _main_render_target.addAttachment(NORMAL_BUFFER_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 3
        _main_render_target.addAttachment(LIGHT_BUFFER_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 4
        _main_render_target.addAttachment(SHADOW_MAP_POS_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 5

        // geometry subpass
        _main_render_target.addSubPass(
            {1, 2, 3, 5},
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
        );
        // lighting subpass
        _main_render_target.addSubPass(
            {1, 4}, // uses the depth buffer as an "output", but doesnt write to it
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}, 
            {2, 3, 5},
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
            {4},
            {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}
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