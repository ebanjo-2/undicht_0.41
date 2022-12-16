#include "app.h"
#include "debug.h"

namespace cell {

    void App::init() {

        undicht::Engine::init(false);

        _master_renderer.init(_gpu, _swap_chain.getExtent(), _default_render_pass);
        _world.init(_gpu);
        _player.init();

        // setting some cells for testing
        std::vector<Cell> cells = {
            Cell(0, 0, 0, 255, 1, 255, 0),
            Cell(50, 1, 50, 100, 21, 70, 0),
            Cell(65, 21, 50, 85, 50, 70, 0),
        };

        _world.loadChunk(glm::ivec3(0,0,0), cells);
        _world.loadChunk(glm::ivec3(255,0,0), cells);
        _world.loadChunk(glm::ivec3(-255,0,0), cells);
        _world.updateWorldBuffer(glm::ivec3(0,0,0));
        _world.updateWorldBuffer(glm::ivec3(255,0,0));
        _world.updateWorldBuffer(glm::ivec3(-255,0,0));

        UND_LOG << "initialized the world\n";
    }

    void App::cleanUp() {

        _gpu.waitForProcessesToFinish();

        _player.cleanUp();
        _world.cleanUp();
        _master_renderer.cleanUp();

        undicht::Engine::cleanUp();
    }

    void App::mainLoop() {
        
        // user input
        if(_main_window.isKeyPressed(GLFW_KEY_ESCAPE))
            stop();

        if(_main_window.isKeyPressed(GLFW_KEY_LEFT_ALT)) {
            _main_window.setCursorEnabled(true);
            _player.enableMouseInput(false);
        } else {
            _main_window.setCursorEnabled(false);
            _player.enableMouseInput(true);
        }

        // updating the world
        _player.move(getDeltaT(), _main_window);

        // checking if the window is minimized
        if(_main_window.isMinimized())
            return;

        // drawing a new frame
        _master_renderer.beginFrame(_swap_chain);
        _master_renderer.loadPlayerCamera(_player);
        _master_renderer.beginGeometryStage(_default_framebuffer);
        _master_renderer.drawWorld(_world.getWorldBuffer());
        _master_renderer.endGeometryStage();
        _master_renderer.endFrame(_swap_chain);
    }

    void App::onWindowResize() {

        // will recreate the swap chain
        undicht::Engine::onWindowResize();

        _master_renderer.onSwapChainResize(_swap_chain, _default_render_pass);
    }

} // namespace cell