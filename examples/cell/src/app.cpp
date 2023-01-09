#include "app.h"
#include "debug.h"
#include "file_tools.h"

namespace cell {

    using namespace undicht;
    using namespace tools;

    void App::init() {

        undicht::Engine::init(true, false);

        _master_renderer.init(_gpu, _swap_chain);
        _world.init(_gpu);
        _lights.init(_gpu);
        _player.init();
        _materials.init(_gpu);

        // setting some materials for testing
        uint32_t grass = _materials.setMaterial(Material("Grass", UND_ENGINE_SOURCE_DIR + "examples/cell/res/grass.png"));
        uint32_t sand = _materials.setMaterial(Material("Sand", UND_ENGINE_SOURCE_DIR + "examples/cell/res/sand.png"));

        // setting some cells for testing
        std::vector<Cell> cells = {
            Cell(0, 0, 0, 255, 1, 255, grass),
            Cell(50, 1, 50, 100, 21, 70, grass),
            Cell(65, 21, 50, 85, 50, 70, sand),
            Cell(5, 5, 5, 6, 7, 6, grass),
        };

        _world.loadChunk(glm::ivec3(0,0,0), cells);
        _world.loadChunk(glm::ivec3(255,0,0), cells);
        _world.loadChunk(glm::ivec3(-255,0,0), cells);
        _world.updateWorldBuffer(glm::ivec3(0,0,0));
        _world.updateWorldBuffer(glm::ivec3(255,0,0));
        _world.updateWorldBuffer(glm::ivec3(-255,0,0));

        PointLight first_light;
        first_light._color = glm::vec3(1.0f, 1.0f, 1.0f);
        first_light._pos = glm::vec3(5.0f, 20.0f, 50.0f);
        first_light._intensity = 5.0f;
        _lights.addPointLight(first_light);

    }

    void App::cleanUp() {

        _gpu.waitForProcessesToFinish();

        _materials.cleanUp();
        _player.cleanUp();
        _world.cleanUp();
        _lights.cleanUp();
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
        if(_master_renderer.beginFrame(_swap_chain)) {
            _master_renderer.loadPlayerCamera(_player);

            _master_renderer.beginGeometryStage();
            _master_renderer.drawWorld(_world.getWorldBuffer(), _materials);

            _master_renderer.beginLightStage();
            _master_renderer.drawLights(_lights);

            _master_renderer.beginFinalStage();
            _master_renderer.drawFinal(_materials);

            _master_renderer.endFrame(_swap_chain);
        } else {
            // skipping a frame, recreating the swap chain
            onWindowResize();
        }

    }

    void App::onWindowResize() {

        // adjusting the cameras aspect ratio
        _player.setAspectRatio(float(_main_window.getWidth()) / _main_window.getHeight());

        // will recreate the swap chain
        undicht::Engine::onWindowResize();

        _master_renderer.onSwapChainResize(_swap_chain);
    }

} // namespace cell