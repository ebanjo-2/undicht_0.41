#include "map/tile.h"
#include "debug.h"

namespace tonk {

    void Tile::setPossibleNeighbour(uint32_t unique_id, float propability, uint32_t direction) {
                
        Neighbour* neighbour = getNeighbour(unique_id);

        if(neighbour == nullptr) {
            // creating a new neighbour
            _neighbours.emplace_back(Neighbour());
            neighbour = &_neighbours.back();
            neighbour->_neigbour_id = unique_id;
        }
        
        // setting the propability
        if(direction == TONK_X_POSITIVE)
            neighbour->_xp_propability = propability;
        else if(direction == TONK_X_NEGATIVE)
            neighbour->_xn_propability = propability;
        else if(direction == TONK_Y_POSITIVE)
            neighbour->_yp_propability = propability;
        else if(direction == TONK_Y_NEGATIVE)
            neighbour->_yn_propability = propability;
        else if(direction == TONK_Z_POSITIVE)
            neighbour->_zp_propability = propability;
        else if(direction == TONK_Z_NEGATIVE)
            neighbour->_zn_propability = propability;

    }



    void Tile::removePossibleNeighbour(uint32_t unique_id) {

        for(int i = 0; i < _neighbours.size(); i++) {

            if(_neighbours.at(i)._neigbour_id == unique_id) {

                _neighbours.erase(_neighbours.begin() + i);
            }

        }

    }

    Tile::Neighbour* Tile::getNeighbour(uint32_t unique_id) {

        for(Neighbour& n : _neighbours) {

            if(n._neigbour_id == unique_id)
                return &n;

        }

        return nullptr;
    }

    std::vector<std::pair<uint32_t, float>> Tile::getPossibleNeighbours(uint32_t direction) const {

        std::vector<std::pair<uint32_t, float>> neighbours;

        for(const Neighbour& neighbour : _neighbours) {

            if(direction == TONK_X_POSITIVE)
                neighbours.push_back({neighbour._neigbour_id, neighbour._xp_propability});
            else if(direction == TONK_X_NEGATIVE)
                neighbours.push_back({neighbour._neigbour_id, neighbour._xn_propability});
            else if(direction == TONK_Y_POSITIVE)
                neighbours.push_back({neighbour._neigbour_id, neighbour._yp_propability});
            else if(direction == TONK_Y_NEGATIVE)
                neighbours.push_back({neighbour._neigbour_id, neighbour._yn_propability});
            else if(direction == TONK_Z_POSITIVE)
                neighbours.push_back({neighbour._neigbour_id, neighbour._zp_propability});
            else if(direction == TONK_Z_NEGATIVE)
                neighbours.push_back({neighbour._neigbour_id, neighbour._zn_propability});
            else 
                UND_LOG << "cant find neighbours in the requested direction: direction is false \n";

        }

        return neighbours;
    }

    void Tile::setName(const std::string& name) {

        _name = name;
    }

    void Tile::setUniqueID(uint32_t id) {
        
        _unique_id = id;
    }

    void Tile::setTileMapID(uint32_t id) {

        _tile_map_id = id;
    }

    std::string Tile::getName() const {

        return _name;
    }

    uint32_t Tile::getUniqueID() const {

        return _unique_id;
    }

    uint32_t Tile::getTileMapID() const {

        return _tile_map_id;
    }

    uint32_t Tile::getNeighbourCount() const {

        return _neighbours.size();
    }

    const std::vector<Tile::Neighbour>& Tile::getAllNeighbours() const {

        return _neighbours;
    }

} // tonk 