#include "app.h"
#include "debug.h"
#include "file_tools.h"
#include "IBL/ibl.h"

namespace cell {

    using namespace undicht;
    using namespace tools;

    void App::init() {
        
        undicht::Engine::init(true, true);

        _master_renderer.init(_gpu, _swap_chain);
        _world.init(_gpu);
        _lights.init(_gpu);
        _player.init();
        _materials.init(_gpu);

        _player.setPosition(glm::vec3(0, -5, 10));

        // setting some materials for testing
        uint32_t grass = _materials.setMaterial(Material("Grass", 
            UND_ENGINE_SOURCE_DIR + "examples/cell/res/grass.png"
        ));

        uint32_t sand = _materials.setMaterial(Material("Sand", 
            UND_ENGINE_SOURCE_DIR + "examples/cell/res/sand.png"
        ));

        uint32_t gold = _materials.setMaterial(Material("Gold",
            UND_ENGINE_SOURCE_DIR + "examples/cell/res/gold_albedo.png",
            UND_ENGINE_SOURCE_DIR + "examples/cell/res/gold_metal.png"
        ));

        // setting some cells for testing
        std::vector<Cell> cells = {
            Cell(0, 255, 0, 255, 254, 255, sand),
            Cell(50, 254, 50, 100, 234, 70, grass),
            Cell(65, 234, 50, 85, 204, 70, sand),
            Cell(5, 250, 5, 6, 248, 6, grass),
            Cell(20, 251, 10, 44, 247, 34, gold),
            Cell(0, 254, 30, 1, 244, 31, gold),
        };

        _world.loadChunk(glm::ivec3(0,-255,0), cells);
        _world.loadChunk(glm::ivec3(255,-255,0), cells);
        _world.loadChunk(glm::ivec3(-255,-255,0), cells);
        _world.updateWorldBuffer(glm::ivec3(0,-255,0));
        _world.updateWorldBuffer(glm::ivec3(255,-255,0));
        _world.updateWorldBuffer(glm::ivec3(-255,-255,0));

        _lights.addPointLight(PointLight(glm::vec3(05.4,-54.4,35.4),glm::vec3(23.47, 21.31, 20.79)));
        _lights.addPointLight(PointLight(glm::vec3(10.5,-20.5,20.5),glm::vec3(50.0,50.0,50.0)));
        _lights.addPointLight(PointLight(glm::vec3(15.5,-3.5,50.5),glm::vec3(1.0,0.0,1.0)));

        _sun.setColor(glm::vec3(23.47, 21.31, 20.79) * 0.5f);
        _sun.setDirection(glm::vec3(1, 1, -1)); // will get normalized
        _sun.setShadowOrigin(glm::vec3(5, -50, 50));

        _master_renderer.loadEnvironment(UND_ENGINE_SOURCE_DIR + "examples/cell/res/environment_maps/Mono_Lake_C/Mono_Lake_C_HiRes.jpg");
        //_master_renderer.loadEnvironment(UND_ENGINE_SOURCE_DIR + "examples/cell/res/environment_maps/Winter_Forest/WinterForest_8k.jpg");

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
        _sun.setDirection(glm::vec3(glm::sin(0.0000000001f * getTimeSinceEpoch()), 0.2, glm::cos(0.0000000001f * getTimeSinceEpoch()))); // will get normalized
        _sun.setShadowOrigin(glm::vec3(0,0,50) - 100.0f * _sun.getDirection()); // rotating the sun pointing at 0,0,50
        _player.move(getDeltaT(), _main_window);

        // checking if the window is minimized
        if(_main_window.isMinimized())
            return;

        // drawing a new frame
        _master_renderer.loadPlayerCamera(_player);
        if(_master_renderer.beginFrame(_swap_chain)) {
            
            // shadow pass
            _master_renderer.beginShadowPass(_sun);
            _master_renderer.drawToShadowMap(_world.getWorldBuffer());

            // main render pass
            _master_renderer.beginMainRenderPass();
            _master_renderer.beginGeometrySubPass(_materials);
            _master_renderer.drawWorld(_world.getWorldBuffer());
            _master_renderer.beginLightSubPass(_materials);
            _master_renderer.drawLight(_sun);
            _master_renderer.drawLights(_lights);
            _master_renderer.drawAmbientLight(glm::vec3(0.02, 0.02, 0.02));
            _master_renderer.beginFinalSubPass();
            _master_renderer.drawFinal(2.0f);

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