#ifndef TONK_TILE_SET_H
#define TONK_TILE_SET_H

#include "vector"
#include "string"

#include "tile.h"


namespace tonk {

    class TileSet {

    protected:

        std::vector<Tile> _tiles;

    public:

        void setTile(const Tile& tile);
        void markFutureTile(const std::string& name);

        const Tile* getTile(uint32_t unique_tile_id) const;
        const Tile* getTile(const std::string& tile_name) const;
        uint32_t getTileID(const std::string& tile_name) const;
        unsigned getTileCount() const;

        const std::vector<Tile>& getTiles() const;

        void resolveNeighbourIDs();

    };

} // tonk

#endif // TONK_TILE_SET_H