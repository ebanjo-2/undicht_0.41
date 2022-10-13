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

class HelloWorldApp :public undicht::Engine {

protected:

    // sync objects
    undicht::vulkan::Fence _render_finished_fence;
    undicht::vulkan::Semaphore _swap_image_ready;
    undicht::vulkan::Semaphore _render_finished_semaphore;

    // commands
    undicht::vulkan::CommandBuffer _draw_command;

    // Shader
    undicht::vulkan::Shader _shader;

    // descriptor pool
    undicht::vulkan::DescriptorPool _descriptor_pool;

    // Pipeline
    undicht::vulkan::DescriptorSetLayout _descriptor_set_layout;
    undicht::vulkan::Pipeline _pipeline;

    // renderer
    undicht::vulkan::Sampler _sampler;
    undicht::vulkan::UniformBuffer _uniform_buffer;
    undicht::vulkan::DescriptorSet _descriptor_set;

    // scene
    undicht::vulkan::Texture _texture;
    undicht::vulkan::VertexBuffer _vertex_buffer;

public:

    void init();

    void mainLoop();

    void cleanUp();

protected:

    void onWindowResize();

    void loadTexture(const std::string& file_name, undicht::vulkan::Texture& loadTo);

};

#endif // HELLO_WORLD_APP_H