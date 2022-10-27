#include "tile_set.h"
#include "debug.h"

namespace tonk {

    unsigned TileSet::addTile(const std::string& tile_name) {

        _tile_names.push_back(tile_name);
        unsigned tile_id = _tile_names.size() - 1;

        _up_propabilities.resize(tile_id + 1);
        _down_propabilities.resize(tile_id + 1);
         _right_propabilities.resize(tile_id + 1);
        _left_propabilities.resize(tile_id + 1);

        for(std::vector<float>& v : _up_propabilities) v.resize(tile_id + 1, 0.0f);
        for(std::vector<float>& v : _down_propabilities) v.resize(tile_id + 1, 0.0f);
        for(std::vector<float>& v : _right_propabilities) v.resize(tile_id + 1, 0.0f);
        for(std::vector<float>& v : _left_propabilities) v.resize(tile_id + 1, 0.0f);

        return tile_id;
    }

    void TileSet::addTileProbabilityFeature(const std::string& tile_name, const std::string& neighbour_name, float up, float down, float right, float left) {


        addTileProbabilityFeature(getTileId(tile_name), getTileId(neighbour_name), up, down, right, left);
    }


    void TileSet::addTileProbabilityFeature(unsigned tile_id, unsigned neighbour_id, float up, float down, float right, float left) {

        _up_propabilities.at(tile_id).at(neighbour_id) = up;
        _down_propabilities.at(tile_id).at(neighbour_id) = down;
        _right_propabilities.at(tile_id).at(neighbour_id) = right;
        _left_propabilities.at(tile_id).at(neighbour_id) = left;

    }

    void TileSet::calcFinalPropabilities() {

        // connect one sided propabilities
        for(int i = 0; i < getTileCount(); i++) {

            for(int j = 0; j < getTileCount(); j++) {
                _up_propabilities.at(i).at(j) = std::max(_up_propabilities.at(i).at(j), _down_propabilities.at(j).at(i));
                _down_propabilities.at(i).at(j) = std::max(_down_propabilities.at(i).at(j), _up_propabilities.at(j).at(i));
                _right_propabilities.at(i).at(j) = std::max(_right_propabilities.at(i).at(j), _left_propabilities.at(j).at(i));
                _left_propabilities.at(i).at(j) = std::max(_left_propabilities.at(i).at(j), _right_propabilities.at(j).at(i));
            }

        }

        // calculate propabilities for a cell with no neighbours
        _start_propabilities.resize(_tile_names.size());
        std::fill(_start_propabilities.begin(), _start_propabilities.end(), 1.0f);

        for(int i = 0; i < getTileCount(); i++) {

            for(int j = 0; j < getTileCount(); j++) {

                _start_propabilities.at(i) *= _up_propabilities.at(j).at(i);
                _start_propabilities.at(i) *= _down_propabilities.at(j).at(i);
                _start_propabilities.at(i) *= _right_propabilities.at(j).at(i);
                _start_propabilities.at(i) *= _left_propabilities.at(j).at(i);

            }

        }

    }

    unsigned TileSet::getTileId(const std::string& tile_name) const {


        for(int i = 0; i < _tile_names.size(); i++) {

            if(!_tile_names.at(i).compare(tile_name))
                return i;

        }

        UND_ERROR << "Tile does not exist: " << tile_name << "\n";
        return 0;
    }
    
    std::string TileSet::getTileName(unsigned tile_id) const {

        return _tile_names.at(tile_id);
    }

    unsigned TileSet::getTileCount() const {
        
        return _tile_names.size();
    }

    void TileSet::getStartPropabilities(std::vector<float>& propabilities) const {

        propabilities = _start_propabilities;
    }

    void TileSet::getUpPropabilities(unsigned tile_id, std::vector<float>& propabilities) const {

        // unknown tile
        if(tile_id >= getTileCount()) {
            std::fill(propabilities.begin(), propabilities.end(), 1.0f);
            return;
        }

        propabilities = _up_propabilities.at(tile_id);
    }

    void TileSet::getDownPropabilities(unsigned tile_id, std::vector<float>& propabilities) const {

        // unknown tile
        if(tile_id >= getTileCount()) {
            std::fill(propabilities.begin(), propabilities.end(), 1.0f);
            return;
        }

        propabilities = _down_propabilities.at(tile_id);
    }

    void TileSet::getRightPropabilities(unsigned tile_id, std::vector<float>& propabilities) const {

        // unknown tile
        if(tile_id >= getTileCount()) {
            std::fill(propabilities.begin(), propabilities.end(), 1.0f);
            return;
        }

        propabilities = _right_propabilities.at(tile_id);
    }

    void TileSet::getLeftPropabilities(unsigned tile_id, std::vector<float>& propabilities) const {

        // unknown tile
        if(tile_id >= getTileCount()) {
            std::fill(propabilities.begin(), propabilities.end(), 1.0f);
            return;
        }

        propabilities = _left_propabilities.at(tile_id);
    }

} // tonk