#include "app.h"
#include "debug.h"
#include "file_tools.h"
#include "IBL/ibl.h"
#include "files/chunk_file.h"
#include "renderer/vulkan/immediate_command.h"
#include "renderer/vulkan/transfer_buffer.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    void App::init() {

        UND_LOG << "App::init() gets called\n";
        
        undicht::Engine::init(true);

        UND_LOG << "initialized the engine\n";

        TransferBuffer transfer_buffer;
        transfer_buffer.init(_gpu);
        //transfer_buffer.allocateInternalBuffer(100000000);

        { // loading stuff
            ImmediateCommand transfer_cmd(_gpu);

            _player.init();
            _player.setPosition(glm::vec3(0, -5, 10));
            _master_renderer.init(_vk_instance.getInstance(), _main_window, _gpu, transfer_cmd, transfer_buffer);

            _world_loader.init(_gpu, transfer_cmd, transfer_buffer);

            if(!_world_loader.openWorldFile(UND_ENGINE_SOURCE_DIR + "examples/cell/worlds/first_world.world")) {
                UND_LOG << "failed to open the world file\n";
            } 

            _world_loader.updateMaterials(transfer_cmd, transfer_buffer);
            _world_loader.updateEnvironment(transfer_cmd, transfer_buffer);

        } // transfer command gets submitted

        transfer_buffer.cleanUp();
        _gpu.resetGraphicsCmdPool();

    }

    void App::cleanUp() {

        _gpu.waitForProcessesToFinish();

        _player.cleanUp();
        _master_renderer.cleanUp();

        _world_loader.cleanUp();

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

        _master_renderer.beginFramePreperation();

        // updating the world
        _world_loader.getWorld().setSunDirection(glm::vec3(glm::sin(0.0000000001f * getTimeSinceEpoch()), 0.2, glm::cos(0.0000000001f * getTimeSinceEpoch()))); // will get normalized
        _player.move(getDeltaT(), _main_window);
        _world_loader.loadChunks(_player.getPosition(), 0, _master_renderer.getTransferCmd(), _master_renderer.getTransferBuf());

        // checking if the window is minimized
        if(_main_window.isMinimized())
            return;
        
        // debug menu
        if(_main_window.isKeyPressed(GLFW_KEY_V)) 
            _debug_menu.open();

        // drawing a new frame
        _master_renderer.endFramePreperation();
        _master_renderer.loadPlayerCamera(_player);
        if(_master_renderer.beginFrame()) {
            
            // shadow pass
            _master_renderer.beginShadowPass(_world_loader.getWorld().getSun(), _player.getPosition());
            _master_renderer.drawToShadowMap(_world_loader.getWorld().getCellBuffer());
            _master_renderer.endShadowPass();

            // main render pass
            _master_renderer.beginMainRenderPass();
            _master_renderer.beginGeometrySubPass(_world_loader.getWorld().getMaterialAtlas());
            _master_renderer.drawWorld(_world_loader.getWorld().getCellBuffer());
            _master_renderer.beginLightSubPass();
            _master_renderer.drawLight(_world_loader.getWorld().getSun());
            _master_renderer.drawLights(_world_loader.getWorld().getLightBuffer());
            _master_renderer.drawAmbientLight(_world_loader.getWorld().getEnvironment());
            _master_renderer.beginFinalSubPass();
            _master_renderer.drawFinal(1.0f);
            _master_renderer.endMainRenderPass();

            // imgui debug menu
            _master_renderer.beginImguiRenderPass();
            _debug_menu.display(getFPS(), _world_loader.getWorld().getEnvironment(), _master_renderer.getTransferCmd(), _master_renderer.getTransferBuf());
            _master_renderer.drawImGui();
            _master_renderer.endImguiRenderPass();

            // end frame
            _master_renderer.endFrame();

        } else {
            // skipping a frame, recreating the swap chain
            onWindowResize();
        }

    }

    void App::onWindowResize() {

        // adjusting the cameras aspect ratio
        _player.setAspectRatio(float(_main_window.getWidth()) / _main_window.getHeight());

        _master_renderer.onWindowResize(_main_window);
    }

} // namespace cell