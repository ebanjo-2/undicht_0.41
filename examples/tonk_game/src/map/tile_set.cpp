#include "tile_set.h"
#include "debug.h"

namespace tonk {

    void TileSet::setTile(const Tile& tile) {

        // searching for the tile in case it was set before
        Tile* tmp = (Tile*) getTile(tile.getName());

        if(tmp) {
            // saving the unique id
            uint32_t unique_id = tmp->getUniqueID();
            *tmp = tile;
            tmp->setUniqueID(unique_id);
        } else {
            _tiles.push_back(tile);
            _tiles.back().setUniqueID(_tiles.size() - 1);
        }
        
    }

    void TileSet::markFutureTile(const std::string& name) {

        if(!getTile(name)) {

            _tiles.emplace_back(Tile());
            _tiles.back().setName(name);
            _tiles.back().setUniqueID(_tiles.size() - 1);
        }

    }

    const Tile* TileSet::getTile(uint32_t unique_tile_id) const {

        if(unique_tile_id < _tiles.size())
            return &_tiles.at(unique_tile_id);
        else
            return nullptr;
    }

    const Tile* TileSet::getTile(const std::string& tile_name) const {

        for(const Tile& t : _tiles) {
            if(!t.getName().compare(tile_name))
                return &t;
        }

        return nullptr;
    }

    uint32_t TileSet::getTileID(const std::string& tile_name) const {

        const Tile* t = getTile(tile_name);

        if(t != nullptr)
            return t->getUniqueID();
        else
            return 0;
    }

    unsigned TileSet::getTileCount() const {

        return _tiles.size();
    }

    const std::vector<Tile>& TileSet::getTiles() const {

        return _tiles;
    }


    void TileSet::resolveNeighbourIDs() {

        for(Tile& tile : _tiles) {

            for(Tile& possible_neighbour : _tiles) {
                
                if(tile.getUniqueID() == possible_neighbour.getUniqueID())
                    continue;
                
                // the possible neighbour mentioned the tile as a neighbour
                Tile::Neighbour* neighbour_data = possible_neighbour.getNeighbour(tile.getUniqueID());
                if(neighbour_data) {

                    // telling the tile about its neighbour (switching the positive and negative borders)
                    tile.setPossibleNeighbour(possible_neighbour.getUniqueID(), neighbour_data->_xn_propability, TONK_X_POSITIVE);
                    tile.setPossibleNeighbour(possible_neighbour.getUniqueID(), neighbour_data->_xp_propability, TONK_X_NEGATIVE);
                    tile.setPossibleNeighbour(possible_neighbour.getUniqueID(), neighbour_data->_yn_propability, TONK_Y_POSITIVE);
                    tile.setPossibleNeighbour(possible_neighbour.getUniqueID(), neighbour_data->_yp_propability, TONK_Y_NEGATIVE);

                }
                
            }

        }

    }

} // tonk