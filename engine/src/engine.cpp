#include "engine.h"
#include "debug.h"

namespace undicht {

    Engine::Engine() : _monitor(_window_api.getMonitor(0)) {

    }

     void Engine::init() {
         /** @brief initializes core engine objects
         * @param open_window opens a main window
         * @param choose_gpu chooses a physical gpu and creates a logical instance for the engine to use it*/

         // initializing the vulkan instance
         _vk_instance.init();

         // opening a window
         _main_window = graphics::Window(_vk_instance.getInstance(), "Undicht Engine V0.41");

         // choosing a gpu
         _gpu.init(_vk_instance.chooseGPU(_main_window.getSurface()), _main_window.getSurface());

         // initializing the swap chain for the main window
         _swap_chain.init(_gpu, _main_window.getSurface(), VK_PRESENT_MODE_FIFO_KHR);

         // creating a render-pass that can be used to draw to the swap images
         _default_render_pass.addOutputAttachment(_swap_chain.getSwapImageFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
         _default_render_pass.addSubPass({0}, {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
         _default_render_pass.init(_gpu.getDevice());

         // creating a framebuffer for every swap image
         _default_framebuffer.resize(_swap_chain.getSwapImageCount());
         for(int i = 0; i < _swap_chain.getSwapImageCount(); i++) {
             _default_framebuffer.at(i).setAttachment(0, _swap_chain.getSwapImageView(i));
             _default_framebuffer.at(i).init(_gpu.getDevice(), _default_render_pass, _swap_chain.getExtent());
         }

     }

     void Engine::updateWindow() {
         /** updates the window */

         _main_window.update();

         // calling event handle functions
         if(_main_window.hasResized())
             onWindowResize();

         if(_main_window.isMinimized())
            onWindowMinimize();
     }

     void Engine::run() {
         /** @brief runs the main loop
          * stops running the main loop if the window is requested to close */

         _should_stop = false;

         while(!_main_window.shouldClose() && !_should_stop) {

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

         // destroying the swap chain framebuffers
         for(vulkan::Framebuffer& fbo : _default_framebuffer)
             fbo.cleanUp();

         // destroying the default render pass
         _default_render_pass.cleanUp();

         // destroying the swap chain
         _swap_chain.cleanUp();

         // releasing the logical device
         _gpu.cleanUp();

         // closing the main window
         _main_window.close();

         // terminating the vulkan instance
         _vk_instance.cleanUp();
     }

     ////////////////////////////////////////////////////////////////////////

    void Engine::onWindowResize() {
        // the swap chain needs to be recreated if the surface it belongs to has changed
        // i.e. if the surface was resized

        // recreating the swap chain
        _swap_chain.recreate(_main_window.getSurface());

        // reinitializing the framebuffers
        _default_framebuffer.resize(_swap_chain.getSwapImageCount());
        for(int i = 0; i < _swap_chain.getSwapImageCount(); i++) {
            _default_framebuffer.at(i).cleanUp();
            _default_framebuffer.at(i).setAttachment(0, _swap_chain.getSwapImageView(i));
            _default_framebuffer.at(i).init(_gpu.getDevice(), _default_render_pass, _swap_chain.getExtent());
        }

    }

    void Engine::onWindowMinimize() {

    }

} // undicht