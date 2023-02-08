#include "renderer.h"
#include "file_tools.h"
#include "core/vulkan/formats.h"
#include "glm/gtc/type_ptr.hpp"

using namespace undicht;
using namespace vulkan;
using namespace tools;

namespace tonk {

    void Renderer::init(VkInstance instance, const undicht::vulkan::LogicalDevice& device, std::vector<undicht::vulkan::Framebuffer>* visible_framebuffers, VkRenderPass render_pass, VkExtent2D viewport, const GLFWwindow* window) {
        
        _device = device;
        _visible_framebuffers = visible_framebuffers;
        _render_pass = render_pass;

        _swap_image_ready.init(device.getDevice());
        _render_finished.init(device.getDevice());
        _render_finished_fence.init(device.getDevice(), true);
        _draw_cmd.init(device.getDevice(), device.getGraphicsCmdPool());

        _sampler.setMaxFilter(VK_FILTER_NEAREST);
        _sampler.setMinFilter(VK_FILTER_NEAREST);
        _sampler.init(device.getDevice());

        initMapPipeline(viewport);

        ImGuiAPI::init(instance, device, render_pass, (GLFWwindow*)window);

    }

    void Renderer::cleanUp() {

        ImGuiAPI::cleanUp(_device.getDevice());

        _sampler.cleanUp();
        _draw_cmd.cleanUp();
        _swap_image_ready.cleanUp();
        _render_finished.cleanUp();
        _render_finished_fence.cleanUp();

        // map pipeline
        _map_pipeline.cleanUp();
        _map_shader.cleanUp();
        _map_descriptor_cache.cleanUp();
        _map_descriptor_set_layout.cleanUp();
        _map_ubo.cleanUp();
    }

    void Renderer::beginFrame(undicht::vulkan::SwapChain& swap_chain) {

        // reset descriptor caches
        _map_descriptor_cache.reset({0});
        
        // wait for previous frame to finish
        _render_finished_fence.waitForProcessToFinish();

        // begin a new frame
        _swap_image_id = swap_chain.acquireNextSwapImage(_swap_image_ready.getAsSignal());

        // tell imgui to start a new frame
        ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // start draw command
        VkClearValue color_clear_value{0.0f, 0.2f, 0.05f, 1.0f};
        VkClearValue depth_clear_value{1.0f, 0};

        _draw_cmd.resetCommandBuffer();
        _draw_cmd.beginCommandBuffer(true);
        _draw_cmd.beginRenderPass(_render_pass, _visible_framebuffers->at(_swap_image_id).getFramebuffer(), swap_chain.getExtent(), {color_clear_value, depth_clear_value});
        
    }

    void Renderer::drawMap(const Map& map, const TileMap& tile_map, glm::vec2 map_center, float zoom_factor) {

        _map_ubo.setAttribute(1, &zoom_factor, sizeof(zoom_factor));
        _map_ubo.setAttribute(2, glm::value_ptr(map_center), 2 * sizeof(float));

        DescriptorSet map_descriptor = _map_descriptor_cache.accquire(0);
        map_descriptor.bindImage(0, tile_map.getMap().getImage().getImageView(), tile_map.getMap().getLayout(), _sampler.getSampler());
        map_descriptor.bindUniformBuffer(1, _map_ubo.getBuffer());

        _draw_cmd.bindGraphicsPipeline(_map_pipeline.getPipeline());
        _draw_cmd.bindDescriptorSet(map_descriptor.getDescriptorSet(), _map_pipeline.getPipelineLayout());
        _draw_cmd.bindIndexBuffer(map.getVertexBuffer().getIndexBuffer().getBuffer());
        _draw_cmd.bindVertexBuffer(map.getVertexBuffer().getVertexBuffer().getBuffer(), 0);
        _draw_cmd.bindVertexBuffer(map.getVertexBuffer().getInstanceBuffer().getBuffer(), 1);
        _draw_cmd.draw(map.getVertexCount(), map.getDrawIndexed(), map.getInstanceCount());

    }

    void Renderer::drawImGui() {

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _draw_cmd.getCommandBuffer());

    }

    void Renderer::endFrame(undicht::vulkan::SwapChain& swap_chain) {

        _draw_cmd.endRenderPass();
        _draw_cmd.endCommandBuffer();

        // submit the draw command
        _device.submitOnGraphicsQueue(_draw_cmd.getCommandBuffer(), {_render_finished_fence.getFence()}, {_swap_image_ready.getAsWaitOn()}, {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}, {_render_finished.getAsSignal()});

        // present the image
        _device.presentOnPresentQueue(swap_chain.getSwapchain(), _swap_image_id, {_render_finished.getAsWaitOn()});
    }

    void Renderer::onWindowResize(VkExtent2D new_viewport) {

        // resize map pipeline viewport
        _map_pipeline.cleanUp();
        _map_pipeline.setViewport(new_viewport);
        _map_pipeline.init(_device.getDevice(), _render_pass);

        float aspect_ratio = float(new_viewport.width) / new_viewport.height;
        _map_ubo.setAttribute(0, &aspect_ratio, sizeof(aspect_ratio));

    }

    VkDescriptorSet Renderer::createImGuiTexture(const undicht::vulkan::Texture& texture) {

        return ImGui_ImplVulkan_AddTexture(_sampler.getSampler(), texture.getImage().getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    /////////////////////////////////////// private functions ///////////////////////////////////////

    void Renderer::initMapPipeline(VkExtent2D viewport) {
        
        // init the shader
        _map_shader.addVertexModule(_device.getDevice(), UND_ENGINE_SOURCE_DIR + "examples/tonk_game/src/shader/bin/map.vert.spv");
        _map_shader.addFragmentModule(_device.getDevice(), UND_ENGINE_SOURCE_DIR + "examples/tonk_game/src/shader/bin/map.frag.spv");
        _map_shader.init(_device.getDevice());

        // init descriptor set
        _map_descriptor_set_layout.setBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // tile map
        _map_descriptor_set_layout.setBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // ubo
        _map_descriptor_set_layout.init(_device.getDevice());

        // init descriptor cache
        _map_descriptor_cache.init(_device, {_map_descriptor_set_layout}, {1000});

        // init the pipeline
        _map_pipeline.setViewport(viewport);
        _map_pipeline.setShaderStages(_map_shader.getShaderModules(), _map_shader.getShaderStages());
        _map_pipeline.addVertexBinding(0, 2 * sizeof(float)); // per vertex data
        _map_pipeline.addVertexBinding(1, 6 * sizeof(float)); // per instance data
        _map_pipeline.addVertexAttribute(0, 0, 0, vulkan::translate(UND_VEC2F)); // vertex pos
        _map_pipeline.addVertexAttribute(1, 1, 0 * sizeof(float), vulkan::translate(UND_VEC2F)); // world pos
        _map_pipeline.addVertexAttribute(1, 2, 2 * sizeof(float), vulkan::translate(UND_VEC2F)); // uv0
        _map_pipeline.addVertexAttribute(1, 3, 4 * sizeof(float), vulkan::translate(UND_VEC2F)); // uv0
        _map_pipeline.setBlending(0, false);
        _map_pipeline.setDepthStencilState(false, false);
        _map_pipeline.setInputAssembly();
        _map_pipeline.setRasterizationState(false, false, false);
        _map_pipeline.setShaderInput(_map_descriptor_set_layout.getLayout());
        _map_pipeline.init(_device.getDevice(), _render_pass);

        // init uniform buffer
        _map_ubo.init(_device, BufferLayout({UND_FLOAT32, UND_FLOAT32, UND_VEC2F})); // aspect ratio, zoom, map center

        float aspect_ratio = float(viewport.width) / viewport.height;
        _map_ubo.setAttribute(0, &aspect_ratio, sizeof(aspect_ratio));

    }

} // tonk