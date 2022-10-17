#include "app.h"
#include "debug.h"
#include "file_tools.h"
#include "core/vulkan/formats.h"
#include "array"

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

void HelloWorldApp::init() {

    UND_LOG << "Init HelloWorldApp\n";

    undicht::Engine::init();

    // init the shader
    _shader.addVertexModule(_gpu.getDevice(), UND_ENGINE_SOURCE_DIR + "graphics/src/shader/bin/triangle.vert.spv");
    _shader.addFragmentModule(_gpu.getDevice(), UND_ENGINE_SOURCE_DIR + "graphics/src/shader/bin/triangle.frag.spv");
    _shader.init(_gpu.getDevice());

    // init the pipeline
    _descriptor_set_layout.setBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    _descriptor_set_layout.setBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    _descriptor_set_layout.init(_gpu.getDevice());

    _pipeline.setViewport(_swap_chain.getExtent());
    _pipeline.setShaderStages(_shader.getShaderModules(), _shader.getShaderStages());
    _pipeline.addVertexBinding(0, 8 * sizeof(float)); // per vertex data
    _pipeline.addVertexAttribute(0, 0, 0, vulkan::translate(UND_VEC3F)); // position
    _pipeline.addVertexAttribute(0, 1, 3 * sizeof(float), vulkan::translate(UND_VEC2F)); // uv
    _pipeline.addVertexAttribute(0, 2, 5 * sizeof(float), vulkan::translate(UND_VEC3F)); // normal
    _pipeline.setBlending(0, false);
    _pipeline.setDepthStencilState(true, true);
    _pipeline.setInputAssembly();
    _pipeline.setRasterizationState(true, true, false);
    _pipeline.setShaderInput(_descriptor_set_layout.getLayout());
    _pipeline.init(_gpu.getDevice(), _default_render_pass.getRenderPass());

    // init frame objects
    _frames.resize(_swap_chain.getSwapImageCount());
    for(int i = 0; i < _frames.size(); i++) {
        _frames[i].init(_gpu, {_descriptor_set_layout});
    } 

    // init the renderer
    _sampler.setMinFilter(VK_FILTER_LINEAR);
    _sampler.setMaxFilter(VK_FILTER_LINEAR);
    _sampler.init(_gpu.getDevice());

    _uniform_buffer.init(_gpu, BufferLayout({UND_MAT4F, UND_MAT4F}));

    // init the scene
    loadModel(UND_ENGINE_SOURCE_DIR + "examples/hello_world/res/sponza_collada/sponza.dae", _model);

    _camera.setViewRange(1.0f, 10000.0f);
    _camera.setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
    _camera.setAxesRotation({0,0,90});
    
}

void HelloWorldApp::mainLoop() {

    if(_main_window.isMinimized()) return;

    // beginning the next frame
    _current_frame = (_current_frame + 1) % _frames.size();
    _frames.at(_current_frame).begin();

    // acquiring an image to render to
    int swap_image_id = _swap_chain.acquireNextSwapImage(_frames.at(_current_frame)._swap_image_ready.getAsSignal());

    // moving the camera
    _camera.addTranslation(_camera.getViewDirection());
    _uniform_buffer.setAttribute(0, glm::value_ptr(_camera.getProjectionMatrix()), 16 * sizeof(float));
    _uniform_buffer.setAttribute(1, glm::value_ptr(_camera.getViewMatrix()), 16 * sizeof(float));

    // record draw commands
    VkClearValue color_clear_value{0.3f, 0.1f, 0.7f, 1.0f};
    VkClearValue depth_clear_value{1.0f, 0};
    _frames[_current_frame]._draw_command.resetCommandBuffer();
    _frames[_current_frame]._draw_command.beginCommandBuffer(true);
    _frames[_current_frame]._draw_command.beginRenderPass(_default_render_pass.getRenderPass(),_default_framebuffer.at(swap_image_id).getFramebuffer(), _swap_chain.getExtent(), {color_clear_value, depth_clear_value});
    _frames[_current_frame]._draw_command.bindGraphicsPipeline(_pipeline.getPipeline());
    
    for(int i = 0; i < _model._vertex_buffers.size(); i++) {
        if(!_model._vertex_count.at(i)) continue;
        undicht::vulkan::DescriptorSet& descriptor_set = _frames[_current_frame]._descriptor_set_caches[0].accquire();
        descriptor_set.bindUniformBuffer(0, _uniform_buffer.getBuffer());
        descriptor_set.bindImage(1, _model._textures[_model._texture_ids[i]].getImage().getImageView(), _model._textures[_model._texture_ids[i]].getLayout(), _sampler.getSampler());

        _frames[_current_frame]._draw_command.bindVertexBuffer(_model._vertex_buffers.at(i).getVertexBuffer().getBuffer(), 0);
        _frames[_current_frame]._draw_command.bindDescriptorSet(descriptor_set.getDescriptorSet(), _pipeline.getPipelineLayout());
        _frames[_current_frame]._draw_command.draw(_model._vertex_count.at(i), false);
    }

    _frames[_current_frame]._draw_command.endRenderPass();
    _frames[_current_frame]._draw_command.endCommandBuffer();

    // submit the draw command
    _gpu.submitOnGraphicsQueue(_frames[_current_frame]._draw_command.getCommandBuffer(), _frames.at(_current_frame)._render_finished_fence.getFence(), {_frames.at(_current_frame)._swap_image_ready.getAsWaitOn()}, {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}, {_frames.at(_current_frame)._render_finished_semaphore.getAsSignal()});

    // present the image
    _gpu.presentOnPresentQueue(_swap_chain.getSwapchain(), swap_image_id, {_frames.at(_current_frame)._render_finished_semaphore.getAsWaitOn()});

}

void HelloWorldApp::cleanUp() {

    UND_LOG << "CleanUp HelloWorldApp\n";

    // waiting for processes to finish
    _gpu.waitForProcessesToFinish();

    // destroy the scene
    _model.cleanUp();

    // destroy the renderer
    _uniform_buffer.cleanUp();
    _sampler.cleanUp();

    // destroy frame objects
    for(Frame& f : _frames) f.cleanUp();

    // destroy the pipeline
    _pipeline.cleanUp();
    _descriptor_set_layout.cleanUp();

    // destroy the shader
    _shader.cleanUp();

    undicht::Engine::cleanUp();

}

void HelloWorldApp::onWindowResize() {

    // will recreate the swap chain
    undicht::Engine::onWindowResize();

    _pipeline.cleanUp();
    _pipeline.setViewport(_swap_chain.getExtent());
    _pipeline.init(_gpu.getDevice(), _default_render_pass.getRenderPass());

}

void HelloWorldApp::loadModel(const std::string& file_name, TexturedModel& loadTo) {

    if(undicht::tools::hasFileType(file_name, ".dae")) {
        UND_ERROR << "failed to load model: incorrect file type\n";
        return;
    }

    // loading the data from the file
    std::vector<undicht::tools::MeshData> meshes;
    std::vector<undicht::tools::ImageData> textures;

    undicht::tools::ColladaFile file(file_name);
    UND_LOG << "loading model file: " << file_name << "\n";
    file.loadAllMeshes(meshes);
    UND_LOG << "loaded " << meshes.size() << " meshes\n";
    file.loadAllTextures(textures);
    UND_LOG << "loaded " << textures.size() << " textures\n";

    // loading the textures
    for(undicht::tools::ImageData& data : textures) {

        undicht::vulkan::Texture t;
        loadTexture(data, t);
        _model._textures.push_back(t);

    }

    // loading the meshes
    for(undicht::tools::MeshData& data : meshes) {

        undicht::vulkan::VertexBuffer t;
        loadMesh(data, t);
        _model._vertex_buffers.push_back(t);
        _model._texture_ids.push_back(data.color_texture);
        _model._vertex_count.push_back(data.vertices.size() / 8);
    }

}


void HelloWorldApp::loadTexture(const std::string& file_name, undicht::vulkan::Texture& loadTo) {

    undicht::tools::ImageData data;
    undicht::tools::ImageFile(file_name, data);

    loadTexture(data, loadTo);
}

void HelloWorldApp::loadTexture(const undicht::tools::ImageData& data, undicht::vulkan::Texture& loadTo) {

    if(data._width && data._height) {
        loadTo.setExtent(data._width, data._height, 1);
    } else {
        loadTo.setExtent(1, 1);
    }
    
    if(data._nr_channels == 3) {
        loadTo.setFormat(undicht::vulkan::translate(UND_R8G8B8));
    } else {
        loadTo.setFormat(undicht::vulkan::translate(UND_R8G8B8A8));
    }

    loadTo.init(_gpu);

    if(data._pixels.size()) {
        loadTo.setData(data._pixels.data(), data._pixels.size());
    } else {
        std::array<char, 4> pink = {0, 0, 0, 0}; // no texture color
        loadTo.setData(pink.data(), pink.size());
    }

}

void HelloWorldApp::loadMesh(const undicht::tools::MeshData& data, undicht::vulkan::VertexBuffer& loadTo) {

    loadTo.init(_gpu);

    if(data.vertices.size())
        loadTo.setVertexData(data.vertices.data(), data.vertices.size() * sizeof(float), 0);
    
    if(data.indices.size())
        loadTo.setIndexData(data.indices.data(), data.indices.size() * sizeof(int), 0);

}
