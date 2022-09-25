#include "app.h"
#include "debug.h"
#include "file_tools.h"


void HelloWorldApp::init() {

    UND_LOG << "Init HelloWorldApp\n";

    undicht::Engine::init();

    // init sync objects
    _render_finished_fences.resize(_swap_chain.getSwapImageCount());
    _swap_image_ready.resize(_swap_chain.getSwapImageCount());
    _render_finished_semaphores.resize(_swap_chain.getSwapImageCount());
    for(undicht::vulkan::Fence& fence : _render_finished_fences) fence.init(_gpu.getDevice(), true);
    for(undicht::vulkan::Semaphore& sem : _swap_image_ready) sem.init(_gpu.getDevice());
    for(undicht::vulkan::Semaphore& sem : _render_finished_semaphores) sem.init(_gpu.getDevice());

    // init command buffers
    _draw_commands.resize(_swap_chain.getSwapImageCount());
    for(undicht::vulkan::CommandBuffer& command : _draw_commands) command.init(_gpu.getDevice(), _gpu.getGraphicsCmdPool());

    // init the shader
    _shader.addVertexModule(_gpu.getDevice(), UND_ENGINE_SOURCE_DIR + "graphics/src/shader/bin/triangle.vert.spv");
    _shader.addFragmentModule(_gpu.getDevice(), UND_ENGINE_SOURCE_DIR + "graphics/src/shader/bin/triangle.frag.spv");
    _shader.init(_gpu.getDevice());

    // init the pipeline
    _pipeline.setViewport(_swap_chain.getExtent());
    _pipeline.setShaderStages(_shader.getShaderModules(), _shader.getShaderStages());
    _pipeline.setBlending(0, false);
    _pipeline.setInputAssembly();
    _pipeline.setRasterizationState(false);
    _pipeline.init(_gpu.getDevice(), _default_render_pass.getRenderPass());

}

void HelloWorldApp::mainLoop() {

    if(_main_window.isMinimized()) return;

    // advancing the frame id
    _current_frame = (_current_frame + 1) % _swap_chain.getSwapImageCount();

    // waiting for the previous rendering to finish
    _render_finished_fences.at(_current_frame).waitForProcessToFinish(true, 1000000000); // 1 sec

    // acquiring an image to render to
    int swap_image_id = _swap_chain.acquireNextSwapImage(_swap_image_ready.at(_current_frame).getAsSignal());

    // record draw commands
    VkClearValue clear_value{0.3f, 0.1f, 0.7f, 1.0f};
    _draw_commands.at(_current_frame).resetCommandBuffer();
    _draw_commands.at(_current_frame).beginCommandBuffer(true);
    _draw_commands.at(_current_frame).beginRenderPass(_default_render_pass.getRenderPass(),_default_framebuffer.at(swap_image_id).getFramebuffer(), _swap_chain.getExtent(), clear_value);
    _draw_commands.at(_current_frame).bindGraphicsPipeline(_pipeline.getPipeline());
    _draw_commands.at(_current_frame).draw(3); // triangle has 3 vertices
    _draw_commands.at(_current_frame).endRenderPass();
    _draw_commands.at(_current_frame).endCommandBuffer();

    // submit the draw command
    _gpu.submitOnGraphicsQueue(_draw_commands.at(_current_frame).getCommandBuffer(), _render_finished_fences.at(_current_frame).getFence(), {_swap_image_ready.at(_current_frame).getAsWaitOn()}, {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}, {_render_finished_semaphores.at(_current_frame).getAsSignal()});

    // present the image
    _gpu.presentOnPresentQueue(_swap_chain.getSwapchain(), swap_image_id, {_render_finished_semaphores.at(_current_frame).getAsWaitOn()});

}

void HelloWorldApp::cleanUp() {

    UND_LOG << "CleanUp HelloWorldApp\n";

    // waiting for processes to finish
    _gpu.waitForProcessesToFinish();

    // destroy the pipeline
    _pipeline.cleanUp();

    // destroy the shader
    _shader.cleanUp();

    // destroying the sync objects
    for(undicht::vulkan::Fence& fence : _render_finished_fences) fence.cleanUp();
    for(undicht::vulkan::Semaphore& sem : _swap_image_ready) sem.cleanUp();
    for(undicht::vulkan::Semaphore& sem : _render_finished_semaphores) sem.cleanUp();

    undicht::Engine::cleanUp();

}

void HelloWorldApp::onWindowResize() {

    // will recreate the swap chain
    undicht::Engine::onWindowResize();

    _pipeline.cleanUp();
    _pipeline.setViewport(_swap_chain.getExtent());
    _pipeline.init(_gpu.getDevice(), _default_render_pass.getRenderPass());

}