#include "map_generator.h"
#include "random"
#include "debug.h"
#include "tile_set.h"

namespace tonk {

    //////////////////////////////////////// settings ////////////////////////////////////////

    void MapGenerator::setSize(uint32_t width, uint32_t height) {
        
        _width = width;
        _height = height;

    }

    //////////////////////////////////////// generate ////////////////////////////////////////


    bool MapGenerator::generate(Map& map, const TileSet& tile_set, const TileMap& tile_map) {

        map.setSize(_width, _height);

        for(int i = 0; i < _width * _height; i++) {

            uint32_t x, y;
            //findNextPositionToResolve(x, y, tile_set, map);
            x = i % _width;
            y = i / _width;

            std::vector<std::pair<uint32_t, float>> props = calcPropabilities(x, y, tile_set, map);
            map.setTile(chooseTile(props, tile_set.getTileCount()), x, y, tile_map);
            
        }


        return true;
    }

    std::vector<std::pair<uint32_t, float>> MapGenerator::calcPropabilities(int x, int y, const TileSet& tile_set, const Map& map) {

        std::vector<std::pair<uint32_t, float>> propabilities;

        for(int i = TONK_X_POSITIVE; i <= TONK_Y_NEGATIVE; i++) {
            
            uint32_t x_neighbour = x - 1 + (i == TONK_X_NEGATIVE) * 2;
            uint32_t y_neighbour = y - 1 + (i == TONK_Y_NEGATIVE) * 2;
            x_neighbour = std::min(x_neighbour, _width - 1);
            y_neighbour = std::min(y_neighbour, _height - 1);

            const Tile* neighbour = tile_set.getTile(map.getTile(x_neighbour, y_neighbour));
            if(!neighbour) continue;

            std::vector<std::pair<uint32_t, float>> additional_propabilities = neighbour->getPossibleNeighbours(i);

            if(!propabilities.size() && additional_propabilities.size()) {

                propabilities = additional_propabilities;
                
            } else if(additional_propabilities.size()) {
                
                for(std::pair<uint32_t, float>& prop : propabilities) {
                    
                    float multiplier = 0.0f;
                    for(std::pair<uint32_t, float>& ap : additional_propabilities)
                        if(ap.first == prop.first) {
                            multiplier = ap.second;
                            break;
                        }

                    prop.second *= multiplier;
                }

            }

        }

        return propabilities;
    }

    uint32_t MapGenerator::chooseTile(const std::vector<std::pair<uint32_t, float>>& propabilities, uint32_t total_tile_count) {

        if(!propabilities.size())
            return rand() % total_tile_count;

        float sum = 0.0f;

        for(const std::pair<uint32_t, float>& p : propabilities)
            sum += p.second;

        float chosen = (rand() % 1000) * sum / 1000.0f;

        float f = 0.0f;
        for(const std::pair<uint32_t, float>& p : propabilities) {
            f += p.second;

            if(f >= chosen)
                return p.first;
        }

        UND_LOG << "failed to set tile\n";
        return 0;
    }

    void MapGenerator::findNextPositionToResolve(uint32_t& x, uint32_t& y, const TileSet& tile_set, const Map& map) {

        do {

            x = rand() % map.getWidth();
            y = rand() % map.getHeight();

        } while(map.getTile(x, y) != (uint32_t) -1);

    }

}