#ifndef TONK_MAP_GENERATOR_H
#define TONK_MAP_GENERATOR_H

#include "map/map.h"
#include "map/tile_set.h"

namespace tonk {

    class MapGenerator {

    protected:

        uint32_t _width = 40;
        uint32_t _height = 25;


    public:

        // settings
        void setSize(uint32_t width, uint32_t height);

        // returns true once the map has been completly generated
        bool generate(Map& map, const TileSet& tile_set, unsigned tiles_to_generate = -1);

    };

} // tonk

#endif // TONK_MAP_GENERATOR_H