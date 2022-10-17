#ifndef HELLO_WORLD_APP_H
#define HELLO_WORLD_APP_H

#include "engine.h"
#include "core/vulkan/semaphore.h"
#include "core/vulkan/fence.h"
#include "core/vulkan/command_buffer.h"
#include "core/vulkan/shader.h"
#include "core/vulkan/pipeline.h"
#include "core/vulkan/descriptor_set_layout.h"
#include "core/vulkan/descriptor_pool.h"
#include "core/vulkan/descriptor_set.h"
#include "core/vulkan/sampler.h"
#include "renderer/vulkan/vertex_buffer.h"
#include "renderer/vulkan/uniform_buffer.h"
#include "renderer/vulkan/texture.h"
#include "3D/camera/perspective_camera_3d.h"

#include "images/image_file.h"
#include "model_loading/collada/collada_file.h"

#include "frame.h"

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

class HelloWorldApp : public undicht::Engine {

protected:

    // handling multiple frames in flight
    std::vector<Frame> _frames;
    uint32_t _current_frame = 0;

    // Shader
    undicht::vulkan::Shader _shader;

    // Pipeline
    undicht::vulkan::DescriptorSetLayout _descriptor_set_layout;
    undicht::vulkan::Pipeline _pipeline;

    // renderer
    undicht::vulkan::Sampler _sampler;
    undicht::vulkan::UniformBuffer _uniform_buffer;
    undicht::tools::PerspectiveCamera3D _camera;

    // scene
    TexturedModel _model;

public:

    void init();

    void mainLoop();

    void cleanUp();

protected:

    void onWindowResize();

    void loadModel(const std::string& file_name, TexturedModel& loadTo);
    void loadTexture(const std::string& file_name, undicht::vulkan::Texture& loadTo);
    void loadTexture(const undicht::tools::ImageData& data, undicht::vulkan::Texture& loadTo);
    void loadMesh(const undicht::tools::MeshData& data, undicht::vulkan::VertexBuffer& loadTo);

};

#endif // HELLO_WORLD_APP_H