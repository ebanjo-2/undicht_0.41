#include "tonk_game.h"
#include "imgui/vulkan/imgui_api.h"
#include "file_tools.h"
#include "debug.h"

#include "map/tile_set_file.h"

using namespace undicht;
using namespace vulkan;
using namespace tools;

namespace tonk {

    void TonkGame::init() {

        Engine::init();

        _renderer.init(_vk_instance.getInstance(), _gpu, &_default_framebuffer, _default_render_pass.getRenderPass(), _swap_chain.getExtent(), _main_window.getWindow());
        
        // load tile set + tile map
        _tile_map.init(_gpu);
        TileSetFile tile_set_file(UND_ENGINE_SOURCE_DIR + "examples/tonk_game/res/default.tonk");
        tile_set_file.loadTileSet(_tile_set, _tile_map);

        _imgui_tile_map_handle = _renderer.createImGuiTexture(_tile_map.getMap());

        // init map
        _map.init(_gpu);
        _map.setTileMap(_tile_map);
        _map_generator.setSize(50, 30);

    }

    void TonkGame::mainLoop() {
        
        processUserInput();

        // begin frame
        _renderer.beginFrame(_swap_chain);
        
        // ImGui debug window
        debugMenu();

        // drawing
        _renderer.drawMap(_map, _tile_map, _map_center, 0.04f);
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

        _renderer.onWindowResize(_swap_chain.getExtent());
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
        
        ImGui::Begin("Tonk Game");
        ImGui::Text(("Pos: " + toStr(_map_center.x) + " : " + toStr(_map_center.y)).c_str());
        if(ImGui::Button("Generate Map")) _generate_new_map = true;
        if(ImGui::Button("Tile Map")) _open_tile_map = true;
        if(ImGui::Button("Exit")) stop();
        ImGui::End();

        if(_open_tile_map) {
            ImGui::Begin("Tile Map", &_open_tile_map);
            ImGui::Image(_imgui_tile_map_handle, {_tile_map.getWidth(), _tile_map.getHeight()});
            ImGui::End();
        }

        if(_generate_new_map) {
            _map.clear();
            _map_generator.placeStartTile(_map, _tile_set);
            _generate_new_map = false;
            _map_complete = false;
        }

        if(!_map_complete) {
            _map_complete = _map_generator.generate(_map, _tile_set, 1);
        }

    }

} // tonk