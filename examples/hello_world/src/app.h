#ifndef HELLO_WORLD_APP_H
#define HELLO_WORLD_APP_H

#include "engine.h"
#include "frame_manager.h"
#include "renderer/vulkan/renderer.h"
#include "renderer/vulkan/render_target.h"
#include "core/vulkan/sampler.h"
#include "renderer/vulkan/texture.h"
#include "renderer/vulkan/vertex_buffer.h"
#include "images/image_data.h"
#include "model_loading/model_loader.h"
#include "renderer/vulkan/uniform_buffer.h"
#include "3D/camera/perspective_camera_3d.h"

class TexturedModel {

public:

    std::vector<undicht::vulkan::Texture> _textures;
    std::vector<undicht::vulkan::VertexBuffer> _vertex_buffers;
    std::vector<uint32_t> _texture_ids; // which texture is used by which vertex buffer
    std::vector<uint32_t> _vertex_count;

    void cleanUp() {

        for(auto& t : _textures)
            t.cleanUp();

        for(auto& t : _vertex_buffers)
            t.cleanUp();

    }
};

class HelloWorldApp : public undicht::Engine, public undicht::FrameManager {

protected:

    // Renderer
    undicht::vulkan::RenderTarget _render_target;
    undicht::vulkan::Renderer _renderer;
    undicht::vulkan::Sampler _sampler;
    undicht::vulkan::UniformBuffer _uniform_buffer;
    undicht::tools::PerspectiveCamera3D _camera;

    // scene
    TexturedModel _model;

    // measuring performance
    long _frame_counter = 0;
    long _start_time;

public:

    void init();

    void mainLoop();

    void cleanUp();

protected:

    void onWindowResize();

    void loadModel(const std::string& file_name, TexturedModel& loadTo, undicht::vulkan::CommandBuffer& transfer_cmd, undicht::vulkan::TransferBuffer& transfer_buffer);
    void loadTexture(const std::string& file_name, undicht::vulkan::Texture& loadTo, undicht::vulkan::CommandBuffer& transfer_cmd, undicht::vulkan::TransferBuffer& transfer_buffer);
    void loadTexture(const undicht::tools::ImageData<char>& data, undicht::vulkan::Texture& loadTo, undicht::vulkan::CommandBuffer& transfer_cmd, undicht::vulkan::TransferBuffer& transfer_buffer);
    void loadMesh(const undicht::tools::MeshData& data, undicht::vulkan::VertexBuffer& loadTo, undicht::vulkan::CommandBuffer& transfer_cmd, undicht::vulkan::TransferBuffer& transfer_buffer);
};

#endif // HELLO_WORLD_APP_H