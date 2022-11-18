#ifndef TONK_DEV_UI_H
#define TONK_DEV_UI_H

#include "imgui/vulkan/imgui_api.h"
#include "glm/glm.hpp"
#include "map/tile_set.h"
#include "map/tile_map.h"

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
        bool _save_tile_set = false;
        bool _resolve_neighbours = false;
        void showTileEditor(TileSet& tile_set, const TileMap& tile_map, const ImTextureID& texture);

    };

} // tonk

#endif // TONK_DEV_UI_H