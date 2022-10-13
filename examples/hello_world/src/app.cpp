#include "app.h"
#include "debug.h"
#include "file_tools.h"
#include "core/vulkan/formats.h"

#include "images/image_file.h"

using namespace undicht;

const std::vector<float> vertices = {
    -0.5f,-0.5f, 0.0f,  0.0f, 1.0f,
    0.5f,-0.5f, 0.0f,  1.0f, 1.0f, 
    0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 
    -0.5f, 0.0f, 0.0f,  0.0f, 0.0f
};

const std::vector<int> indices = {
    0, 1, 2, 2, 3, 0
};

const std::vector<float> color = {
    1.0f, 0.0f, 0.0f,
};

const float brightness = 0.1f;

void HelloWorldApp::init() {

    UND_LOG << "Init HelloWorldApp\n";

    undicht::Engine::init();

    // init sync objects
    _render_finished_fence.init(_gpu.getDevice(), true);
    _swap_image_ready.init(_gpu.getDevice());
    _render_finished_semaphore.init(_gpu.getDevice());

    // init command buffers
    _draw_command.init(_gpu.getDevice(), _gpu.getGraphicsCmdPool());

    // init the shader
    _shader.addVertexModule(_gpu.getDevice(), UND_ENGINE_SOURCE_DIR + "graphics/src/shader/bin/triangle.vert.spv");
    _shader.addFragmentModule(_gpu.getDevice(), UND_ENGINE_SOURCE_DIR + "graphics/src/shader/bin/triangle.frag.spv");
    _shader.init(_gpu.getDevice());

    // init descriptor pool
    _descriptor_pool.init(_gpu.getDevice(), 10);

    // init the pipeline
    _descriptor_set_layout.setBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    _descriptor_set_layout.setBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    _descriptor_set_layout.init(_gpu.getDevice());

    _pipeline.setViewport(_swap_chain.getExtent());
    _pipeline.setShaderStages(_shader.getShaderModules(), _shader.getShaderStages());
    _pipeline.addVertexBinding(0, 5 * sizeof(float)); // per vertex data
    _pipeline.addVertexAttribute(0, 0, 0, vulkan::translate(UND_VEC3F)); // position
    _pipeline.addVertexAttribute(0, 1, 3 * sizeof(float), vulkan::translate(UND_VEC2F)); // uv
    _pipeline.setBlending(0, false);
    _pipeline.setInputAssembly();
    _pipeline.setRasterizationState(false);
    _pipeline.setShaderInput(_descriptor_set_layout.getLayout());
    _pipeline.init(_gpu.getDevice(), _default_render_pass.getRenderPass());

    // init the renderer
    _sampler.setMinFilter(VK_FILTER_LINEAR);
    _sampler.setMaxFilter(VK_FILTER_LINEAR);
    _sampler.init(_gpu.getDevice());

    _uniform_buffer.init(_gpu, BufferLayout({UND_VEC3F, UND_FLOAT32})); // color, brightness
    _uniform_buffer.setAttribute(0, color.data(), 3 * sizeof(float));
    _uniform_buffer.setAttribute(1, &brightness, sizeof(brightness));

    _descriptor_set.init(_gpu.getDevice(), _descriptor_pool.getDescriptorPool(), _descriptor_set_layout.getLayout());
    _descriptor_set.bindUniformBuffer(0, _uniform_buffer.getBuffer());

    // init the scene
    _vertex_buffer.init(_gpu);
    _vertex_buffer.setVertexData(vertices.data(), vertices.size() * sizeof(float), 0);
    _vertex_buffer.setIndexData(indices.data(), indices.size() * sizeof(int), 0);

    loadTexture(UND_ENGINE_SOURCE_DIR + "examples/hello_world/res/Tux.jpg", _texture);
    _descriptor_set.bindImage(1, _texture.getImage().getImageView(), _texture.getLayout(), _sampler.getSampler());

}

void HelloWorldApp::mainLoop() {

    if(_main_window.isMinimized()) return;

    // waiting for the previous rendering to finish
    _render_finished_fence.waitForProcessToFinish(true, 1000000000); // 1 sec

    // acquiring an image to render to
    int swap_image_id = _swap_chain.acquireNextSwapImage(_swap_image_ready.getAsSignal());

    // record draw commands
    VkClearValue clear_value{0.3f, 0.1f, 0.7f, 1.0f};
    _draw_command.resetCommandBuffer();
    _draw_command.beginCommandBuffer(true);
    _draw_command.beginRenderPass(_default_render_pass.getRenderPass(),_default_framebuffer.at(swap_image_id).getFramebuffer(), _swap_chain.getExtent(), clear_value);
    _draw_command.bindGraphicsPipeline(_pipeline.getPipeline());
    _draw_command.bindVertexBuffer(_vertex_buffer.getVertexBuffer().getBuffer(), 0);
    _draw_command.bindIndexBuffer(_vertex_buffer.getIndexBuffer().getBuffer());
    _draw_command.bindDescriptorSet(_descriptor_set.getDescriptorSet(), _pipeline.getPipelineLayout());
    _draw_command.draw(6, true); // one triangle has 3 vertices, drawing 2 triangles
    _draw_command.endRenderPass();
    _draw_command.endCommandBuffer();

    // submit the draw command
    _gpu.submitOnGraphicsQueue(_draw_command.getCommandBuffer(), _render_finished_fence.getFence(), {_swap_image_ready.getAsWaitOn()}, {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}, {_render_finished_semaphore.getAsSignal()});

    // present the image
    _gpu.presentOnPresentQueue(_swap_chain.getSwapchain(), swap_image_id, {_render_finished_semaphore.getAsWaitOn()});

}

void HelloWorldApp::cleanUp() {

    UND_LOG << "CleanUp HelloWorldApp\n";

    // waiting for processes to finish
    _gpu.waitForProcessesToFinish();

    // destroy the scene
    _vertex_buffer.cleanUp();
    _texture.cleanUp();

    // destroy the renderer
    _uniform_buffer.cleanUp();
    _sampler.cleanUp();

    // destroy the pipeline
    _pipeline.cleanUp();
    _descriptor_set_layout.cleanUp();

    // destroy the descriptor pool
    _descriptor_pool.cleanUp();

    // destroy the shader
    _shader.cleanUp();

    // destroying the sync objects
    _render_finished_fence.cleanUp();
    _swap_image_ready.cleanUp();
    _render_finished_semaphore.cleanUp();

    // destroy command buffers
    _draw_command.cleanUp();

    undicht::Engine::cleanUp();

}

void HelloWorldApp::onWindowResize() {

    // will recreate the swap chain
    undicht::Engine::onWindowResize();

    _pipeline.cleanUp();
    _pipeline.setViewport(_swap_chain.getExtent());
    _pipeline.init(_gpu.getDevice(), _default_render_pass.getRenderPass());

}

void HelloWorldApp::loadTexture(const std::string& file_name, undicht::vulkan::Texture& loadTo) {

    undicht::tools::ImageData data;
    undicht::tools::ImageFile(file_name, data);

    loadTo.setExtent(data._width, data._height, 1);

    if(data._nr_channels == 3)
        loadTo.setFormat(undicht::vulkan::translate(UND_R8G8B8));
    else
        loadTo.setFormat(undicht::vulkan::translate(UND_R8G8B8A8));

    loadTo.init(_gpu);
    loadTo.setData(data._pixels.data(), data._pixels.size());

}