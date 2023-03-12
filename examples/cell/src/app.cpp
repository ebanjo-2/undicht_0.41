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

        _master_renderer.init(_vk_instance.getInstance(), (GLFWwindow*)_main_window.getWindow(), _gpu, _swap_chain);
        _world.init(_gpu);
        _player.init();
        _player.setPosition(glm::vec3(0, -5, 10));

        glm::vec3 sun_dir = glm::vec3(0.5f,1.0,0.5f);

        _world.setSunDirection(sun_dir);
        _world.setSunColor(glm::vec3(23.47, 21.31, 20.79) * 0.5f);

        if(_world_file.open(UND_ENGINE_SOURCE_DIR + "examples/cell/worlds/first_world.world")) {

            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(0,-255,0), new CellChunk()), glm::ivec3(0,-255,0));
            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(255,-255,0), new CellChunk()), glm::ivec3(255,-255,0));
            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(-255,-255,0), new CellChunk()), glm::ivec3(-255,-255,0));
            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(0,-255,-255), new CellChunk()), glm::ivec3(0,-255,-255));
            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(0,-255, 255), new CellChunk()), glm::ivec3(0,-255, 255));
            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(255,-255,-255), new CellChunk()), glm::ivec3(255,-255,-255));
            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(255,-255, 255), new CellChunk()), glm::ivec3(255,-255, 255));
            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(-255,-255,-255), new CellChunk()), glm::ivec3(-255,-255,-255));
            _world_file.read(*(CellChunk*)_world.getCellWorld().loadChunk(glm::ivec3(-255,-255, 255), new CellChunk()), glm::ivec3(-255,-255, 255));

            UND_LOG << "updating world buffer\n";
            _world.updateWorldBuffer();
            UND_LOG << "done updating the world buffer\n";

            _world_file.read(*(LightChunk*)_world.getLightWorld().loadChunk(glm::ivec3(0,-255,0), new LightChunk()), glm::ivec3(0,-255,0));
            _world.updateLightBuffer();

            _world_file.readMaterials(_world.getMaterialAtlas());
            
            //_world_file.readEnvironment(_world.getEnvironment());

        } else {
            UND_LOG << "failed to open the world file\n";
            //_world_file.newWorldFile();
        }

        CubeMapData<float> env_map;
        _env_gen.init();
        _env_gen.setSunDir(sun_dir);
        
        // darkish sky / clouds 
        _env_gen.setCloudCoverage(1.1f); 
        _env_gen.setCloudDensity(1.7f);
        _env_gen.setSkyBrightness(1.2f);
        _env_gen.setCloudBrightness(0.05f);

        // brightish sky / clouds 
        /*_env_gen.setCloudCoverage(0.55f); 
        _env_gen.setCloudDensity(2.5f);
        _env_gen.setSkyBrightness(1.0f);
        _env_gen.setCloudBrightness(1.0f);*/

        _env_gen.generate(env_map);
        _world.getEnvironment().load(env_map);
        _world.getEnvironment().calcLightingMaps(env_map);

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
        _player.move(getDeltaT(), _main_window);

        // checking if the window is minimized
        if(_main_window.isMinimized())
            return;
        
        // debug menu
        if(_main_window.isKeyPressed(GLFW_KEY_V)) 
            _debug_menu.open();            

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
            _master_renderer.drawAmbientLight(_world.getEnvironment());
            _master_renderer.beginFinalSubPass();
            _master_renderer.drawFinal(0.3f);

            // imgui debug menu
            _master_renderer.beginImguiRenderPass();
            _debug_menu.display(getFPS(), _world.getEnvironment());
            _master_renderer.drawImGui();

            // end frame
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