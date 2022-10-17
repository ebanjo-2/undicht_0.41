#ifndef UNDICHT_ENGINE_H
#define UNDICHT_ENGINE_H

#include "vector"

#include "window/glfw/window_api.h"
#include "window/glfw/monitor.h"
#include "window/glfw/window.h"

#include "core/vulkan/instance.h"
#include "core/vulkan/logical_device.h"
#include "core/vulkan/swap_chain.h"
#include "core/vulkan/renderpass.h"
#include "core/vulkan/framebuffer.h"

namespace undicht {

    class Engine {
    protected:

        graphics::WindowAPI _window_api;
        graphics::Monitor _monitor;
        graphics::Window _main_window;

        vulkan::Instance _vk_instance;
        vulkan::LogicalDevice _gpu;
        vulkan::SwapChain _swap_chain;
        vulkan::RenderPass _default_render_pass;
        
        std::vector<vulkan::Image> _depth_buffers;
        std::vector<vulkan::Framebuffer> _default_framebuffer; // one for each image of the swap chain


        bool _should_stop = false;

    public:

        Engine();

        /** @brief initializes core engine objects
        * @param open_window opens a main window
        * chooses a physical gpu and creates a logical instance for the engine to use it
        * creates a swap chain to render to */
        virtual void init();

        /** can be implemented by a child "application" class */
        virtual void mainLoop(){};

        /** updates the window */
        virtual void updateWindow();

        /** @brief runs the main loop
         * stops running the main loop if the window is requested to close */
        virtual void run();

        /** stops the main loop running */
        virtual void stop();

        /** clean up the core engine objects */
        virtual void cleanUp();

    protected:
        // default event handling

        // called when the default window was resized
        virtual void onWindowResize();

        virtual void onWindowMinimize();

    };

}

#endif // UNDICHT_ENGINE_H