#ifndef HELLO_WORLD_APP_H
#define HELLO_WORLD_APP_H

#include "engine.h"
#include "core/vulkan/semaphore.h"
#include "core/vulkan/fence.h"
#include "core/vulkan/command_buffer.h"
#include "core/vulkan/shader.h"
#include "core/vulkan/pipeline.h"

class HelloWorldApp :public undicht::Engine {

protected:

    uint32_t _current_frame = 0;

    // sync objects (one for every swap image)
    std::vector<undicht::vulkan::Fence> _render_finished_fences;
    std::vector<undicht::vulkan::Semaphore> _swap_image_ready;
    std::vector<undicht::vulkan::Semaphore> _render_finished_semaphores;

    // commands
    std::vector<undicht::vulkan::CommandBuffer> _draw_commands;

    // Shader
    undicht::vulkan::Shader _shader;

    // Pipeline
    undicht::vulkan::Pipeline _pipeline;

public:

    void init();

    void mainLoop();

    void cleanUp();

protected:

    void onWindowResize();
};

#endif // HELLO_WORLD_APP_H