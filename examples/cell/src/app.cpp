#include "app.h"
#include "debug.h"
#include "file_tools.h"
#include "IBL/ibl.h"
#include "files/chunk_file.h"

namespace cell {

    using namespace undicht;
    using namespace tools;

    void App::init() {

        UND_LOG << "App::init() gets called\n";
        
        undicht::Engine::init(true, true);

        UND_LOG << "initialized the engine\n";

        _master_renderer.init(_gpu, _swap_chain);
        _world.init(_gpu);
        _player.init();
        _player.setPosition(glm::vec3(0, -5, 10));

        _world.setSunDirection(glm::vec3(1,0.5,1));
        _world.setSunColor(glm::vec3(23.47, 21.31, 20.79) * 0.5f);

        _master_renderer.loadEnvironment(UND_ENGINE_SOURCE_DIR + "examples/cell/res/environment_maps/Mono_Lake_C/Mono_Lake_C_HiRes.jpg");
        //_master_renderer.loadEnvironment(UND_ENGINE_SOURCE_DIR + "examples/cell/res/environment_maps/Winter_Forest/WinterForest_8k.jpg");
        //_master_renderer.loadEnvironment(UND_ENGINE_SOURCE_DIR + "examples/cell/res/environment_maps/Milkyway/Milkyway_BG.jpg");

        if(_world_file.open(UND_ENGINE_SOURCE_DIR + "examples/cell/worlds/first_world.world")) {

            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(0,-255,0), new CellChunk()), glm::ivec3(0,-255,0));
            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(255,-255,0), new CellChunk()), glm::ivec3(255,-255,0));
            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(-255,-255,0), new CellChunk()), glm::ivec3(-255,-255,0));
            _world.updateWorldBuffer();

            _world_file.read(*(LightChunk*)_world.getLightWorld().loadChunk(glm::ivec3(0,-255,0), new LightChunk()), glm::ivec3(0,-255,0));
            _world.updateLightBuffer();

            //_world_file.write(*(CellChunk*)_world.getCellWorld().getChunkAt(glm::ivec3(0,-255,0)), glm::ivec3(-255,-255,0));
            //_world_file.write(light_chunk, glm::ivec3(0,-255,0));
            _world_file.readMaterials(_world.getMaterialAtlas());

        } else {
            UND_LOG << "failed to open the world file\n";
            //_world_file.newWorldFile();
        }

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
        _world.setSunDirection(glm::vec3(glm::sin(0.0000000001f * getTimeSinceEpoch()), 0.2, glm::cos(0.0000000001f * getTimeSinceEpoch()))); // will get normalized
        //_world.setSunTarget(_player.getPosition());
        _player.move(getDeltaT(), _main_window);

        // checking if the window is minimized
        if(_main_window.isMinimized())
            return;

        // drawing a new frame
        _master_renderer.loadPlayerCamera(_player);
        if(_master_renderer.beginFrame(_swap_chain)) {
            
            // shadow pass
            _master_renderer.beginShadowPass(_world.getSun(), _player.getPosition());
            _master_renderer.drawToShadowMap(_world.getCellBuffer());

            // main render pass
            _master_renderer.beginMainRenderPass();
            _master_renderer.beginGeometrySubPass(_world.getMaterialAtlas());
            _master_renderer.drawWorld(_world.getCellBuffer());
            _master_renderer.beginLightSubPass();
            _master_renderer.drawLight(_world.getSun());
            _master_renderer.drawLights(_world.getLightBuffer());
            _master_renderer.drawAmbientLight();
            _master_renderer.beginFinalSubPass();
            _master_renderer.drawFinal(1.0f);

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