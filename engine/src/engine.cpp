#include "engine.h"
#include "debug.h"

namespace undicht {

    Engine::Engine() : _monitor(_window_api.getMonitor(0)) {
    }

    void Engine::init(bool full_screen) {
        /** @brief initializes core engine objects */

        // initializing the vulkan instance
        _vk_instance.init();

        // opening a window
        _monitor = _window_api.getMonitor();
        _main_window = graphics::Window(_vk_instance.getInstance(), "Undicht Engine V0.41");

        if(full_screen)
            _main_window.setFullscreen(&_monitor);

        // choosing a gpu
        _gpu.init(_vk_instance.chooseGPU(_main_window.getSurface()), _main_window.getSurface());

    }

    void Engine::updateWindow() {
        /** updates the window */

        _main_window.update();

        // calling event handle functions
        if (_main_window.hasResized())
            onWindowResize();

        if (_main_window.isMinimized())
            onWindowMinimize();
    }

    void Engine::run() {
        /** @brief runs the main loop
         * stops running the main loop if the window is requested to close */

        _should_stop = false;

        while (!_main_window.shouldClose() && !_should_stop) {
            
            _last_frame_time = _this_frame_time;
            _this_frame_time = std::chrono::high_resolution_clock::now();

            mainLoop();
            updateWindow();
        }
    }

    void Engine::stop() {
        /** stops the main loop running */

        _should_stop = true;
    }

    void Engine::cleanUp() {
        /** clean up the core engine objects */
        // in the opposite order in which they were created

        // releasing the logical device
        _gpu.cleanUp();

        // closing the main window
        _main_window.close();

        // terminating the vulkan instance
        _vk_instance.cleanUp();
    }

    double Engine::getDeltaT() const {
        /** time in milliseconds since last frame started */

        return std::chrono::duration<double, std::milli>(_this_frame_time - _last_frame_time).count() ;
    }

    double Engine::getFPS() const {

        return 1000.0 / getDeltaT(); // delta_t is in milliseconds
    }

    double Engine::getTimeSinceEpoch() const {

        return _this_frame_time.time_since_epoch().count();
    }


} // namespace undicht