#include "renderer/master_renderer.h"
#include "core/vulkan/formats.h"
#include "debug.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui/vulkan/imgui_api.h"

namespace cell {

    using namespace undicht;
    using namespace vulkan;

    void MasterRenderer::init(const VkInstance& instance, GLFWwindow* window, const undicht::vulkan::LogicalDevice& device, undicht::vulkan::SwapChain& swap_chain, bool enable_imgui) {
        
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

        _enable_imgui = enable_imgui;

        if(_enable_imgui)
            undicht::vulkan::ImGuiAPI::init(instance, device, swap_chain, window);


    }

    void MasterRenderer::cleanUp() {

        if(_enable_imgui)
            undicht::vulkan::ImGuiAPI::cleanUp();
        
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

        // end draw command
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

    void MasterRenderer::beginShadowPass(const Light& global_shadow_source, const glm::vec3& shadow_target) {

        _current_pass = SHADOW_PASS;

        // calculate the shadow matrices
        glm::mat4 shadow_view = glm::lookAt(shadow_target - 100.0f * global_shadow_source.getDirection(), shadow_target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 shadow_proj = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, -200.0f, 200.0f);

        // loading the global shadow source
        _global_uniform_buffer.setAttribute(6, glm::value_ptr(shadow_view), 16 * sizeof(float));
        _global_uniform_buffer.setAttribute(7, glm::value_ptr(shadow_proj), 16 * sizeof(float));

        undicht::vulkan::Framebuffer& frame_buffer = _shadow_map_target.getFramebuffer(_swap_image_id);
        VkClearValue depth_clear_value{1.0f, 0};

        _draw_cmd.beginRenderPass(_shadow_map_target.getRenderPass().getRenderPass(), frame_buffer.getFramebuffer(), _shadow_map_target.getExtent(), {depth_clear_value});
        _shadow_renderer.beginFrame(_draw_cmd, _global_descriptor_set);

    }

    void MasterRenderer::drawToShadowMap(const CellBuffer& world) {
        
        _shadow_renderer.draw(world, _draw_cmd);
    }

    void MasterRenderer::endShadowPass() {

        // end shadow pass
        if(_current_pass == SHADOW_PASS)
            _draw_cmd.endRenderPass();
        
    }

    void MasterRenderer::beginMainRenderPass() {

        _current_pass = MAIN_PASS;

        // clearing the framebuffer
        undicht::vulkan::Framebuffer& frame_buffer = _main_render_target.getFramebuffer(_swap_image_id);
        VkClearValue visible_clear_value{0.01f, 0.01f, 0.01f, 1.0f};
        VkClearValue depth_clear_value{1.0f, 0};
        VkClearValue light_clear_value{0.0f, 0.0f, 0.0f, 0.0f};
        VkClearValue albedo_roughness_clear_value{0, 0, 0, 0};
        VkClearValue normal_metalness_clear_value{0.0f, 0.0f, 0.0f, 0.0f};
        VkClearValue position_rel_cam_clear_value{0.0f, 0.0f, 0.0f, 0.0f};
        VkClearValue shadow_pos_clear_value{0.0f, 0.0f, 0.0f, 0.0f};

        std::vector<VkClearValue> clear_values = {
            visible_clear_value,
            depth_clear_value,
            light_clear_value,
            albedo_roughness_clear_value,
            normal_metalness_clear_value,
            position_rel_cam_clear_value,
            shadow_pos_clear_value
        };

        // begin main pass
        _draw_cmd.beginRenderPass(_main_render_target.getRenderPass().getRenderPass(), frame_buffer.getFramebuffer(), _viewport, clear_values);

    }

    void MasterRenderer::beginGeometrySubPass(const MaterialAtlas& materials) {

        _world_renderer.beginFrame(materials, _global_descriptor_set, _draw_cmd);
    }

    void MasterRenderer::drawWorld(const CellBuffer& world) {

        _world_renderer.draw(world, _draw_cmd);
    }

    void MasterRenderer::beginLightSubPass() {

        _draw_cmd.nextSubPass(VK_SUBPASS_CONTENTS_INLINE);

        const VkImageView& albedo_roughness = _main_render_target.getAttachment(_swap_image_id, 3);
        const VkImageView& normal_metalness = _main_render_target.getAttachment(_swap_image_id, 4);
        const VkImageView& position_rel_cam = _main_render_target.getAttachment(_swap_image_id, 5);
        const VkImageView& shadow_map_pos = _main_render_target.getAttachment(_swap_image_id, 6);

        _light_renderer.beginFrame(_global_descriptor_set, _draw_cmd, albedo_roughness, normal_metalness, position_rel_cam, shadow_map_pos);

    }

    void MasterRenderer::drawLights(const LightBuffer& lights) {

        _light_renderer.draw(lights, _draw_cmd);
    }

    void MasterRenderer::drawLight(const Light& light) {
        // draw a directional light

        const VkImageView& shadow_map = _shadow_map_target.getAttachment(_swap_image_id, 0);
        const VkImageLayout shadow_map_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        _light_renderer.draw(light, _draw_cmd, shadow_map, shadow_map_layout, _SHADOW_MAP_WIDTH, _SHADOW_MAP_HEIGHT);
    }

    void MasterRenderer::drawAmbientLight(const Environment& env) {

        _light_renderer.draw(env, _draw_cmd);
    }


    void MasterRenderer::beginFinalSubPass() {

        _draw_cmd.nextSubPass(VK_SUBPASS_CONTENTS_INLINE);

        const VkImageView& light_hdr = _main_render_target.getAttachment(_swap_image_id, 2);

        _final_renderer.beginFrame(_draw_cmd, light_hdr);
    }

    void MasterRenderer::drawFinal(float exposure) {

        _final_renderer.draw(_draw_cmd, exposure);
    }

    void MasterRenderer::endMainRenderPass() {

        // ending the main render pass
        if(_current_pass == MAIN_PASS)
            _draw_cmd.endRenderPass();

    }


    //////////////////////////////////////////////// imgui pass ////////////////////////////////////////////////

    void MasterRenderer::beginImguiRenderPass() {

        _current_pass = IMGUI_PASS;
        undicht::vulkan::ImGuiAPI::newFrame();
    }

    void MasterRenderer::drawImGui() {

        undicht::vulkan::ImGuiAPI::render(_swap_image_id, _draw_cmd);
    }

    void MasterRenderer::endImguiRenderPass() {

        // ending the imgui pass
        if(_current_pass == IMGUI_PASS)
            undicht::vulkan::ImGuiAPI::endFrame();

    }

    ////////////////////////////////////////////// other functions //////////////////////////////////////////////

    void MasterRenderer::onSwapChainResize(undicht::vulkan::SwapChain& swap_chain) {
        
        _viewport = {(uint32_t)swap_chain.getExtent().width, (uint32_t)swap_chain.getExtent().height};

        float inv_viewport[] = {1.0f / _viewport.width, 1.0f / _viewport.height};
        _global_uniform_buffer.setAttribute(4, &_viewport, sizeof(_viewport));
        _global_uniform_buffer.setAttribute(5, inv_viewport, 2 * sizeof(float));

        _main_render_target.resize(_viewport, &swap_chain);

        _world_renderer.onViewportResize(_device_handle, _viewport, _main_render_target.getRenderPass());
        _light_renderer.onViewportResize(_device_handle, _viewport, _main_render_target.getRenderPass());
        _final_renderer.onViewportResize(_device_handle, _viewport, _main_render_target.getRenderPass());

        undicht::vulkan::ImGuiAPI::onViewportResize(swap_chain);
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
        const VkFormat LIGHT_BUFFER_FORMAT = translate(UND_VEC4F16); // using 2 byte floats to store a higher range (hdr)
        const VkFormat ALBEDO_ROUGHNESS_FORMAT = translate(UND_R8G8B8A8);
        const VkFormat NORMAL_METALNESS_FORMAT = translate(UND_VEC4F16);
        const VkFormat POSITION_REL_CAM_FORMAT = translate(UND_VEC4F16);
        const VkFormat SHADOW_MAP_POS_FORMAT = translate(UND_VEC4F16); // pos + depth on shadow map

        VkImageLayout swap_image_output_layout;
        if(_enable_imgui) { // the imgui render pass will transition it to the present layout
            swap_image_output_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else {
            swap_image_output_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }

        _main_render_target.setDeviceHandle(device, swap_chain.getSwapImageCount());
        _main_render_target.addVisibleAttachment(swap_chain, true, true, swap_image_output_layout); // 0
        _main_render_target.addAttachment(DEPTH_BUFFER_FORMAT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // 1
        _main_render_target.addAttachment(LIGHT_BUFFER_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 2
        _main_render_target.addAttachment(ALBEDO_ROUGHNESS_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 3
        _main_render_target.addAttachment(NORMAL_METALNESS_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 4
        _main_render_target.addAttachment(POSITION_REL_CAM_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 5
        _main_render_target.addAttachment(SHADOW_MAP_POS_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, true, false); // 6

        // geometry subpass
        _main_render_target.addSubPass(
            {1, 3, 4, 5, 6},
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
        );
        // lighting subpass
        _main_render_target.addSubPass(
            {1, 2}, // uses the depth buffer as an "output", but doesnt write to it (used by point lights)
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}, 
            {3, 4, 5, 6},
            {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}
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
            {2},
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