#ifndef UNDICHT_ENGINE_H
#define UNDICHT_ENGINE_H

#include "vector"
#include "chrono"

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
        // initializes vulkan and a window
    protected:

        // glfw
        graphics::WindowAPI _window_api;
        graphics::Monitor _monitor;
        graphics::Window _main_window;

        // vulkan
        vulkan::Instance _vk_instance;
        vulkan::LogicalDevice _gpu;

        bool _should_stop = false;

        std::chrono::high_resolution_clock::time_point _this_frame_time;
        std::chrono::high_resolution_clock::time_point _last_frame_time;

    public:

        Engine();

        /** @brief initializes core engine objects
        * chooses a physical gpu and creates a logical instance for the engine to use it
        * creates a swap chain to render to */
        virtual void init(bool full_screen = false);

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

        /** time in milliseconds since last frame started */
        double getDeltaT() const;
        
        // 1 / delta_t
        double getFPS() const;

        double getTimeSinceEpoch() const;

    protected:
        // default event handling

        // called when the default window was resized
        virtual void onWindowResize() {};
        virtual void onWindowMinimize() {};

    };

}

#endif // UNDICHT_ENGINE_H