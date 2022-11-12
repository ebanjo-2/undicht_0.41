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


    bool MapGenerator::generate(Map& map, const TileSet& tile_set, unsigned tiles_to_generate) {

        map.setSize(_width, _height);

        map.setTile(1, 4, 4);

        return tiles_to_generate != 0;
    }

}