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
            ImGui::Image(texture, {width, height});
            ImGui::End();
        }

    }

    void DevUI::showTileEditor(TileSet& tile_set) {

        if(!_open_tile_editor)
            return;

        Tile* current_tile = (Tile*)tile_set.getTile(_tile_selected);
        Tile* current_neighbour_tile = (Tile*)tile_set.getTile(_neighbour_selected);

        ImGui::Begin("Tile Editor", &_open_tile_editor);

        // drop-down menu for all tiles
        if(ImGui::BeginCombo("Tile: ", tile_set.getTile(_tile_selected)->getName().data())) {

            for(int i = 0; i < tile_set.getTileCount(); i++) 
                if(ImGui::Selectable(tile_set.getTile(i)->getName().data(), false))
                    _tile_selected = i;
                
            ImGui::EndCombo();
        }

        // drop-down menu to choose which neighbour to edit
        if(ImGui::BeginCombo("Neighbour: ", current_neighbour_tile->getName().data())) {

            for(int i = 0; i < current_tile->getNeighbourCount(); i++)
                if(ImGui::Selectable(tile_set.getTile(current_tile->getAllNeighbours()[i]._neigbour_id)->getName().data(), false)) 
                    _neighbour_selected = current_tile->getAllNeighbours()[i]._neigbour_id;

            ImGui::EndCombo();
        }

        // editing the propability values
        std::vector<char> buffer(255, ' ');
        std::string propability;

        // pos x
        propability = toStr(current_tile->getAllNeighbours()[_neighbour_selected]._xp_propability);
        std::copy(propability.begin(), propability.end(), buffer.begin());
        if(ImGui::InputText("Pos X: ", buffer.data(), buffer.size())) 
            current_tile->setPossibleNeighbour(_neighbour_selected, atof(buffer.data()), TONK_X_POSITIVE);

        // neg x
        propability = toStr(current_tile->getAllNeighbours()[_neighbour_selected]._xn_propability);
        std::copy(propability.begin(), propability.end(), buffer.begin());
        if(ImGui::InputText("Neg X: ", buffer.data(), buffer.size())) 
            current_tile->setPossibleNeighbour(_neighbour_selected, atof(buffer.data()), TONK_X_NEGATIVE);

        // pos y
        propability = toStr(current_tile->getAllNeighbours()[_neighbour_selected]._yp_propability);
        std::copy(propability.begin(), propability.end(), buffer.begin());
        if(ImGui::InputText("Pos Y: ", buffer.data(), buffer.size())) 
            current_tile->setPossibleNeighbour(_neighbour_selected, atof(buffer.data()), TONK_Y_POSITIVE);

        // neg x
        propability = toStr(current_tile->getAllNeighbours()[_neighbour_selected]._yn_propability);
        std::copy(propability.begin(), propability.end(), buffer.begin());
        if(ImGui::InputText("Neg Y: ", buffer.data(), buffer.size())) 
            current_tile->setPossibleNeighbour(_neighbour_selected, atof(buffer.data()), TONK_Y_NEGATIVE);

        
        ImGui::End();
    }



} // tonk