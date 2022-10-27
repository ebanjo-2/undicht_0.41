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

        void placeStartTile(Map& map, const TileSet& tile_set);

        // returns true once the map has been completly generated
        bool generate(Map& map, const TileSet& tile_set, unsigned tiles_to_generate = -1);

    protected:
        // private functions for generating maps

        // calc the tile propabilities for a position, based on the surrounding tiles
        void static getTilePropabilities(const Map& map, const TileSet& tile_set, uint32_t x, uint32_t y, std::vector<float>& propabilities);

        // choose a tile id based on the propabilities
        uint32_t static chooseTile(std::vector<float>& propabilities);

        // next tile to "collapse"
        bool static findNextTile(const Map& map, uint32_t& x, uint32_t& y);
    };

} // tonk

#endif // TONK_MAP_GENERATOR_H