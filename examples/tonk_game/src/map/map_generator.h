#ifndef TONK_MAP_GENERATOR_H
#define TONK_MAP_GENERATOR_H

#include "map/map.h"
#include "map/tile_set.h"
#include "map/tile_map.h"

namespace tonk {

    class MapGenerator {

    protected:

        uint32_t _width = 40;
        uint32_t _height = 25;


    public:

        // settings
        void setSize(uint32_t width, uint32_t height);

        // returns true once the map has been completly generated
        bool generate(Map& map, const TileSet& tile_set, const TileMap& tile_map);

    protected:

        std::vector<std::pair<uint32_t, float>> calcPropabilities(int x, int y, const TileSet& tile_set, const Map& map);
        uint32_t chooseTile(const std::vector<std::pair<uint32_t, float>>& propabilities, uint32_t total_tile_count);
        void findNextPositionToResolve(uint32_t& x, uint32_t& y, const TileSet& tile_set, const Map& map);
        
    };

} // tonk

#endif // TONK_MAP_GENERATOR_H