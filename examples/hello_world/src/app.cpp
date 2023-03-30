#include "app.h"
#include "debug.h"
#include "file_tools.h"
#include "types.h"
#include "model_loading/collada/collada_file.h"
#include "model_loading/obj/obj_file.h"
#include "array"
#include "core/vulkan/formats.h"
#include "renderer/vulkan/immediate_command.h"
#include "renderer/vulkan/transfer_buffer.h"

using namespace undicht;
using namespace tools;
using namespace vulkan;

const int NUM_FRAMES = 2;

void HelloWorldApp::init() {

    UND_LOG << "Init HelloWorldApp\n";

    undicht::Engine::init(true);
    undicht::FrameManager::init(_gpu, _main_window, false, NUM_FRAMES);

    // visible render target
    _render_target.setDeviceHandle(_gpu, getSwapChain().getSwapImageCount());
    _render_target.addVisibleAttachment(getSwapChain()); // 0
    _render_target.addAttachment(VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, true, false); // 1
    _render_target.addSubPass({0, 1}, {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
    _render_target.init(getSwapChain().getExtent(), &getSwapChain());

    // renderer
    _renderer.setDeviceHandle(_gpu);
    _renderer.setShaders(UND_ENGINE_SOURCE_DIR + "graphics/src/shader/bin/triangle.vert.spv", UND_ENGINE_SOURCE_DIR + "graphics/src/shader/bin/triangle.frag.spv");
    _renderer.setDescriptorSetLayout({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}, 0);
    _renderer.setVertexInputLayout(BufferLayout({UND_VEC3F, UND_VEC2F, UND_VEC3F}));
    _renderer.setRasterizer(true, true);
    _renderer.init(getSwapChain().getExtent(), _render_target.getRenderPass(), 0, NUM_FRAMES);

    // sampler
    _sampler.setMaxFilter(VK_FILTER_LINEAR);
    _sampler.setMinFilter(VK_FILTER_LINEAR);
    _sampler.setRepeatMode(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
    _sampler.init(_gpu.getDevice());

    // uniform buffer
    _uniform_buffer.init(_gpu, BufferLayout({UND_MAT4F, UND_MAT4F}));

    // camera
    _camera.setViewRange(1.0f, 10000.0f);
    _camera.setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
    _camera.setAxesRotation({0,0,90});

    // load model
    TransferBuffer transfer_buffer;
    transfer_buffer.init(_gpu);
    //transfer_buffer.allocateInternalBuffer(200000000);
    {
        ImmediateCommand transfer_cmd(_gpu); // just a single command to load the entire models textures
        loadModel(UND_ENGINE_SOURCE_DIR + "examples/hello_world/res/sponza/sponza.obj", _model, transfer_cmd, transfer_buffer);
    }
    transfer_buffer.cleanUp();

}

void HelloWorldApp::mainLoop() {

    // stopping after 500 frames
    _frame_counter++;
    if(_frame_counter == 50) {
        _start_time = getTimeMillesec();
    }

    if(_frame_counter >= 550) {
        long total_time = getTimeMillesec() - _start_time;
        UND_LOG << "500 frames in " << total_time << "ms, average of " << 1000.0f / (total_time / 500.0f) << " FPS\n";
        stop();
    }

    if(_main_window.isMinimized()) return;

    // move camera
    _camera.addTranslation(_camera.getViewDirection());
    _uniform_buffer.setAttribute(0, glm::value_ptr(_camera.getProjectionMatrix()), 16 * sizeof(float));
    _uniform_buffer.setAttribute(1, glm::value_ptr(_camera.getViewMatrix()), 16 * sizeof(float));

    // clear the visible render target
    VkClearValue color_clear_value{0.3f, 0.1f, 0.7f, 1.0f};
    VkClearValue depth_clear_value{1.0f, 0};
    std::vector<VkClearValue> clear_values = {color_clear_value, depth_clear_value};

    // drawing
    if(undicht::FrameManager::beginFrame()) {

        _renderer.beginFrame(getFrameID());
        _renderer.resetDescriptorCache(0);
        _renderer.bindPipeline(getDrawCmd());

        _render_target.beginRenderPass(getDrawCmd(), getSwapImageID(), clear_values);

        for(int i = 0; i < _model._vertex_buffers.size(); i++) {

            if(!_model._vertex_count.at(i)) continue;

            _renderer.accquireDescriptorSet(0);
            _renderer.bindUniformBuffer(0, 0, _uniform_buffer.getBuffer());
            _renderer.bindImage(0 , 1, _model._textures[_model._texture_ids[i]].getImage().getImageView(), _model._textures[_model._texture_ids[i]].getLayout(), _sampler.getSampler());
            _renderer.bindDescriptorSet(getDrawCmd(), 0);

            _renderer.bindVertexBuffer(getDrawCmd(), _model._vertex_buffers.at(i));

            _renderer.draw(getDrawCmd(), _model._vertex_count.at(i));
        }

        _render_target.endRenderPass(getDrawCmd());

        undicht::FrameManager::endFrame();
    }

}

void HelloWorldApp::cleanUp() {

    UND_LOG << "CleanUp HelloWorldApp\n";

    _gpu.waitForProcessesToFinish();

    _model.cleanUp();
    _uniform_buffer.cleanUp();
    _sampler.cleanUp();
    _renderer.cleanUp();
    _render_target.cleanUp();

    undicht::FrameManager::cleanUp();
    undicht::Engine::cleanUp();
}

void HelloWorldApp::onWindowResize() {

    undicht::FrameManager::onWindowResize(_main_window);
    _render_target.resize(getSwapChain().getExtent(), &getSwapChain());
    _renderer.resizeViewport(getSwapChain().getExtent());
}

void HelloWorldApp::loadModel(const std::string& file_name, TexturedModel& loadTo, undicht::vulkan::CommandBuffer& transfer_cmd, undicht::vulkan::TransferBuffer& transfer_buffer) {

    // loading the data from the file
    std::vector<undicht::tools::MeshData> meshes;
    std::vector<undicht::tools::ImageData<char>> textures;

    if(hasFileType(file_name, ".dae")) {
        undicht::tools::ColladaFile file(file_name);
        file.loadAllMeshes(meshes);
        file.loadAllTextures(textures);
    } else if (hasFileType(file_name, ".obj")) {
        undicht::tools::OBJFile file(file_name);
        file.loadAllMeshes(meshes);
        file.loadAllTextures(textures);
    } else {
        UND_ERROR << "failed to load model:" << file_name << " : incorrect file type\n";
        return;
    }

    UND_LOG << "loading model file: " << file_name << "\n";
    UND_LOG << "loaded " << textures.size() << " textures\n";
    UND_LOG << "loaded " << meshes.size() << " meshes\n";

    // loading the textures
    for(undicht::tools::ImageData<char>& data : textures) {
        //UND_LOG << "loading the textures\n";
        undicht::vulkan::Texture t;
        t.setMipMaps(true);
        loadTexture(data, t, transfer_cmd, transfer_buffer);
        _model._textures.push_back(t);

    }

    // loading the meshes
    for(undicht::tools::MeshData& data : meshes) {

        undicht::vulkan::VertexBuffer t;
        loadMesh(data, t, transfer_cmd, transfer_buffer);
        _model._vertex_buffers.push_back(t);
        _model._texture_ids.push_back(data.color_texture);
        _model._vertex_count.push_back(data.vertices.size() / 8);
    }

}


void HelloWorldApp::loadTexture(const std::string& file_name, undicht::vulkan::Texture& loadTo, undicht::vulkan::CommandBuffer& transfer_cmd, undicht::vulkan::TransferBuffer& transfer_buffer) {

    undicht::tools::ImageData<char> data;
    undicht::tools::ImageFile(file_name, data);

    loadTexture(data, loadTo, transfer_cmd, transfer_buffer);
}

void HelloWorldApp::loadTexture(const undicht::tools::ImageData<char>& data, undicht::vulkan::Texture& loadTo, undicht::vulkan::CommandBuffer& transfer_cmd, undicht::vulkan::TransferBuffer& transfer_buffer) {

    if(data.getWidth() && data.getHeight()) {
        loadTo.setExtent(data.getWidth(), data.getHeight(), 1);
    } else {
        loadTo.setExtent(1, 1);
    }
    
    if(data.getNrChannels() == 3) {
        loadTo.setFormat(undicht::vulkan::translate(UND_R8G8B8_SRGB));
    } else {
        loadTo.setFormat(undicht::vulkan::translate(UND_R8G8B8A8_SRGB));
    }

    loadTo.init(_gpu);

    if(data.getPixelDataSize()) {
        loadTo.setData(transfer_cmd, transfer_buffer, data.getPixelData(), data.getPixelDataSize());
    } else {
        std::array<char, 4> pink = {0, 0, 0, 0}; // no texture color
        loadTo.setData(transfer_cmd, transfer_buffer, pink.data(), pink.size());
    }

}

void HelloWorldApp::loadMesh(const undicht::tools::MeshData& data, undicht::vulkan::VertexBuffer& loadTo, undicht::vulkan::CommandBuffer& transfer_cmd, undicht::vulkan::TransferBuffer& transfer_buffer) {

    loadTo.init(_gpu);

    if(data.vertices.size()) {
        loadTo.allocateVertexBuffer(data.vertices.size() * sizeof(float));
        loadTo.setVertexData(data.vertices.data(), data.vertices.size() * sizeof(float), 0, transfer_cmd, transfer_buffer);
    }

    if(data.indices.size()) {
        loadTo.allocateIndexBuffer(data.indices.size() * sizeof(int));
        loadTo.setIndexData(data.indices.data(), data.indices.size() * sizeof(int), 0, transfer_cmd, transfer_buffer);
    }

}