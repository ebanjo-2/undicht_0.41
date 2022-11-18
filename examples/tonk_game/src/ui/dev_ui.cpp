#include "dev_ui.h"
#include "file_tools.h"
#include "map/tile.h"
#include "map/tile_set.h"
#include "debug.h"
#include "string"

using namespace undicht;
using namespace tools;

namespace tonk {

    void DevUI::showMainMenu(const glm::vec2& pos) {

        ImGui::Begin("Tonk Game");
        ImGui::Text(("Pos: " + toStr(pos.x) + " : " + toStr(pos.y) + "\0").c_str());
        if(ImGui::Button("Generate Map")) _generate_new_map = true;
        if(ImGui::Button("Tile Map")) _open_tile_map = true;
        if(ImGui::Button("Tile Editor")) _open_tile_editor = true;
        if(ImGui::Button("Exit")) _should_app_stop = true;
        ImGui::End();

    }

    void DevUI::showTileMap(const ImTextureID& texture, uint32_t width, uint32_t height) {

        if(_open_tile_map) {
            ImGui::Begin("Tile Map", &_open_tile_map);
            ImGui::Image(texture, ImVec2((float)width, (float)height));
            ImGui::End();
        }

    }

    void DevUI::showTileEditor(TileSet& tile_set, const TileMap& tile_map, const ImTextureID& texture) {

        if(!_open_tile_editor)
            return;


        ImGui::Begin("Tile Editor", &_open_tile_editor);

        // button for saving the file
        if(ImGui::Button("Save"))
            _save_tile_set = true;

        ImGui::SameLine();
        if(ImGui::Button("(Load) // to be implemented")); // to be implemented

        // drop-down menu for all tiles
        if(tile_set.getTileCount()) {
            if(ImGui::BeginCombo("Tile: ", tile_set.getTile(_tile_selected)->getName().data())) {

                for(int i = 0; i < tile_set.getTileCount(); i++) 
                    if(ImGui::Selectable(tile_set.getTile(i)->getName().data(), false)) {
                        _tile_selected = i;

                        if(tile_set.getTile(_tile_selected)->getAllNeighbours().size())
                            _neighbour_selected = tile_set.getTile(_tile_selected)->getAllNeighbours().front()._neigbour_id;
                        else
                            _neighbour_selected = -1;

                    }
                
                ImGui::EndCombo();
            }

            // displaying the tile image
            ImVec2 uv0;
            ImVec2 uv1;
            tile_map.calcUVs(_tile_selected, uv0.x, uv0.y, uv1.x, uv1.y);
            ImGui::Image(texture, ImVec2(64.0f, 64.0f), uv0, uv1);
        }

        Tile* current_tile = (Tile*)tile_set.getTile(_tile_selected);

        // drop-down menu to choose which neighbour to edit
        if(_neighbour_selected != (uint32_t)-1) {

            if( ImGui::BeginCombo("Neighbour: ", tile_set.getTile(_neighbour_selected)->getName().data())) {

                for(int i = 0; i < current_tile->getNeighbourCount(); i++)
                    if(ImGui::Selectable(tile_set.getTile(current_tile->getAllNeighbours()[i]._neigbour_id)->getName().data(), false)) 
                        _neighbour_selected = current_tile->getAllNeighbours()[i]._neigbour_id;

                ImGui::EndCombo();

            }

            // displaying the neighbour image
            ImVec2 uv0;
            ImVec2 uv1;
            tile_map.calcUVs(_neighbour_selected, uv0.x, uv0.y, uv1.x, uv1.y);
            ImGui::Image(texture, ImVec2(64.0f, 64.0f), uv0, uv1);

            // editing the propability values with sliders
            ImGui::SliderFloat("propability x positive", &current_tile->getNeighbour(_neighbour_selected)->_xp_propability, 0.0f, 1.0f);
            ImGui::SliderFloat("propability x negative", &current_tile->getNeighbour(_neighbour_selected)->_xn_propability, 0.0f, 1.0f);
            ImGui::SliderFloat("propability y positive", &current_tile->getNeighbour(_neighbour_selected)->_yp_propability, 0.0f, 1.0f);
            ImGui::SliderFloat("propability y negative", &current_tile->getNeighbour(_neighbour_selected)->_yn_propability, 0.0f, 1.0f);
        }

        // button to resolve the neighbour propabilities
        if(ImGui::Button("resolve neigbours"))
            _resolve_neighbours = true;

        ImGui::End();
    }



} // tonk