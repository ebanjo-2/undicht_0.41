#ifndef TONK_DEV_UI_H
#define TONK_DEV_UI_H

#include "imgui/vulkan/imgui_api.h"
#include "glm/glm.hpp"
#include "map/tile_set.h"

namespace tonk {

    class DevUI {

    public:
        // main menu

        bool _show_main_menu = true; // options that can be set in the main menu
        bool _generate_new_map = false;
        bool _open_tile_map = false;
        bool _open_tile_editor = false;
        bool _should_app_stop = false;

        void showMainMenu(const glm::vec2& pos);

        // tile map
        void showTileMap(const ImTextureID& texture, uint32_t width, uint32_t height);

        // tile editor
        uint32_t _tile_selected = 0;
        uint32_t _neighbour_selected = 0;
        void showTileEditor(TileSet& tile_set);

    };

} // tonk

#endif // TONK_DEV_UI_H