#include "tonk_game.h"
#include "imgui/vulkan/imgui_api.h"
#include "file_tools.h"
#include "debug.h"

#include "map/tile_set_file.h"

using namespace undicht;
using namespace vulkan;
using namespace tools;

const std::string DEFAULT_TILE_SET = UND_ENGINE_SOURCE_DIR + "examples/tonk_game/res/default.tonk";

namespace tonk {

    void TonkGame::init() {

        Engine::init();

        _renderer.init(_vk_instance.getInstance(), _gpu, &_default_framebuffer, _swap_chain, _default_render_pass.getRenderPass(), _swap_chain.getExtent(), _main_window.getWindow());
        
        // load tile set + tile map
        _tile_map.init(_gpu);
        TileSetFile tile_set_file(DEFAULT_TILE_SET);
        tile_set_file.loadTileSet(_tile_set, _tile_map);

        _imgui_tile_map_handle = _renderer.createImGuiTexture(_tile_map.getMap());

        // init map
        _map.init(_gpu);
        _map_generator.setSize(50, 50);

    }

    void TonkGame::mainLoop() {
        
        processUserInput();

        // begin frame
        _renderer.beginFrame(_swap_chain);
        
        // ImGui debug window
        debugMenu();

        // drawing
        _renderer.drawMap(_swap_chain, _map, _tile_map, _map_center, 0.04f);
        _renderer.drawImGui();

        // end frame
        _renderer.endFrame(_swap_chain);
 
    }

    void TonkGame::cleanUp() {

        _gpu.waitForProcessesToFinish();

        _map.cleanUp();
        _renderer.cleanUp();
        _tile_map.cleanUp();

        Engine::cleanUp();

    }

    void TonkGame::onWindowResize() {

        Engine::onWindowResize();

        _renderer.onWindowResize(_swap_chain);
    }

    /////////////////////////////////// processing user input ///////////////////////////////////

    void TonkGame::processUserInput() {

        // moving freely over the map
        if(_main_window.isKeyPressed(GLFW_KEY_W))
            _map_center += glm::vec2(0.0f, -0.03f);

        if(_main_window.isKeyPressed(GLFW_KEY_S))
            _map_center += glm::vec2(0.0f, 0.03f);

        if(_main_window.isKeyPressed(GLFW_KEY_A))
            _map_center += glm::vec2(-0.03f, 0.0f);

        if(_main_window.isKeyPressed(GLFW_KEY_D))
            _map_center += glm::vec2(0.03f, 0.0f);

        // clamp the map center position
        _map_center = glm::max(_map_center, glm::vec2(0.0f, 0.0f));
        _map_center = glm::min(_map_center, glm::vec2(_map.getWidth(), _map.getHeight()));
    }

    ///////////////////////////////////////// debug menu ////////////////////////////////////////////

    void TonkGame::debugMenu() {
        
        _dev_ui.showMainMenu(_map_center);
        _dev_ui.showTileMap(_imgui_tile_map_handle, _tile_map.getWidth(), _tile_map.getHeight());
        _dev_ui.showTileEditor(_tile_set, _tile_map, _imgui_tile_map_handle);

        if(_dev_ui._should_app_stop)
            stop();

        if(_dev_ui._generate_new_map) {
            _map.clear();
            _dev_ui._generate_new_map = false;
            _map_complete = false;
        }

        if(!_map_complete) {
            _map_complete = _map_generator.generate(_map, _tile_set, _tile_map);
        }

        if(_dev_ui._save_tile_set) {
            TileSetFile file(DEFAULT_TILE_SET);
            file.savePropabilities(_tile_set);
            _dev_ui._save_tile_set = false;
        }

        if(_dev_ui._resolve_neighbours) {
            _tile_set.resolveNeighbourIDs();
            _dev_ui._resolve_neighbours = false;
        }
    }

} // tonk