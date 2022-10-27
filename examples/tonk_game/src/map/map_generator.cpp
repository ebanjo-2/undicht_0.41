#include "map_generator.h"
#include "random"
#include "debug.h"

namespace tonk {

    //////////////////////////////////////// settings ////////////////////////////////////////

    void MapGenerator::setSize(uint32_t width, uint32_t height) {
        
        _width = width;
        _height = height;

    }

    //////////////////////////////////////// generate ////////////////////////////////////////

    void MapGenerator::placeStartTile(Map& map, const TileSet& tile_set) {
        
        map.setSize(_width, _height);

        // seed rng
        srand(getTimeMillesec());

        // choose start position
        int x = rand() % map.getWidth();
        int y = rand() % map.getHeight();

        // get start propabilities
        std::vector<float> propabilities(tile_set.getTileCount());
        tile_set.getStartPropabilities(propabilities);

        map.setTile(chooseTile(propabilities), x, y);

    }

    bool MapGenerator::generate(Map& map, const TileSet& tile_set, unsigned tiles_to_generate) {

        map.setSize(_width, _height);

        // determine tiles
        uint32_t x, y;
        std::vector<float> propabilities(tile_set.getTileCount());
        while(findNextTile(map, x, y) && tiles_to_generate) {
            
            getTilePropabilities(map, tile_set, x, y, propabilities);
            map.setTile(chooseTile(propabilities), x, y);

            tiles_to_generate--;
        }

        // if tiles_to_generate is not 0, no new undetermined tiles could be found
        return tiles_to_generate != 0;
    }

    ////////////////////// private functions for generating maps ////////////////////////////

    void MapGenerator::getTilePropabilities(const Map& map, const TileSet& tile_set, uint32_t x, uint32_t y, std::vector<float>& propabilities) {
        // a list of how likely it is for all tiles to be placed on the position
        // based on its neighbouring cells

        // setting all propabilities to 1
        std::fill(propabilities.begin(), propabilities.end(), 1.0f);

        // check all neighbouring cells
        uint32_t start_x = std::max(int(x - 1), 0);
        uint32_t start_y = std::max(int(y - 1), 0);
        uint32_t end_x = std::min(x + 1, map.getWidth() - 1);
        uint32_t end_y = std::min(y + 1, map.getHeight() - 1);

        std::vector<float> propabilities0(tile_set.getTileCount());
        tile_set.getUpPropabilities(map.getTile(x, y + 1), propabilities0);

        std::vector<float> propabilities1(tile_set.getTileCount());
        tile_set.getDownPropabilities(map.getTile(x, y - 1), propabilities1);

        std::vector<float> propabilities2(tile_set.getTileCount());
        tile_set.getLeftPropabilities(map.getTile(x + 1, y), propabilities2);

        std::vector<float> propabilities3(tile_set.getTileCount());
        tile_set.getRightPropabilities(map.getTile(x - 1, y), propabilities3);

        // multiply the tile propabilities onto the total propabilities
        propabilities.resize(tile_set.getTileCount());
        for(int i = 0; i < propabilities.size(); i++) {
            propabilities.at(i) = propabilities0.at(i);
            propabilities.at(i) *= propabilities1.at(i);
            propabilities.at(i) *= propabilities2.at(i);
            propabilities.at(i) *= propabilities3.at(i);
        }

    }

    uint32_t MapGenerator::chooseTile(std::vector<float>& propabilities) {
        
        // sum of all propabilities
        float sum = 0.0f;
        for(float& p : propabilities)
            sum += p;

        // random number between 0 and sum
        float r = (rand()%10000) / 10000.0f * sum;

        // choosing a tile
        sum = 0.0f;
        for(int i = 0; i < propabilities.size(); i++) {
            sum += propabilities.at(i);

            if(sum >= r)
                return i;

        }

        // shouldnt happen
        UND_WARNING << "failed to choose a tile\n";
        return 0;
    }

    bool MapGenerator::findNextTile(const Map& map, uint32_t& x, uint32_t& y) {
        // ideally the tile with the lowest overall propabilities
        // could be quite slow

        // instead trying to find the tile with the most determined neighbours
        x = 0;
        y = 0;

        int max_neighbours = 0;

        for(int ix = 0; ix < map.getWidth(); ix++) {
            for(int iy = 0; iy < map.getHeight(); iy++) {

                // see if tile is already set
                if(map.getTile(ix, iy) != uint32_t(-1)) continue;

                // counting the determined neighbours of this cell
                int neighbours = 0;
                if(map.getTile(ix - 1, iy) != uint32_t(-1)) neighbours++;
                if(map.getTile(ix + 1, iy) != uint32_t(-1)) neighbours++;
                if(map.getTile(ix, iy - 1) != uint32_t(-1)) neighbours++;
                if(map.getTile(ix, iy + 1) != uint32_t(-1)) neighbours++;

                if(neighbours > max_neighbours) {
                    max_neighbours = neighbours;
                    x = ix;
                    y = iy;
                }

                if(neighbours == 4)
                    return true;

            }

        }

        // should be 0 if no undetermined cell could be found
        return max_neighbours;
    }

}