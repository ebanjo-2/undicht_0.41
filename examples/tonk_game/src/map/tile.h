#ifndef TONK_TILE_H
#define TONK_TILE_H

#define TONK_X_POSITIVE 0
#define TONK_X_NEGATIVE 1
#define TONK_Y_POSITIVE 2
#define TONK_Y_NEGATIVE 3
#define TONK_Z_POSITIVE 4
#define TONK_Z_NEGATIVE 5

#include "vector"
#include "string"
#include "cinttypes"

namespace tonk {


    class Tile {

    public:

        struct Neighbour {
            // contains the multipliers for the propability of the referenced tile
            // to exist next to this cell

            uint32_t _neigbour_id = -1;

            // multipliers should be in the range of 0.0 to 1.0
            // two tiles referencing a neighbour with the same propability
            // will have an equal chance of becoming a neighbour to the tile
            float _xp_propability = 0.0f; // multiplier for positive x direction
            float _xn_propability = 0.0f;
            float _yp_propability = 0.0f;
            float _yn_propability = 0.0f;
            float _zp_propability = 0.0f;
            float _zn_propability = 0.0f;
        };

        // two cells that can be neighbours should refernce each other as neighbours
        // with the same propabilities (except positive and negative directions swapped)

    protected:

        // the name is the universally unique identifier of the tile (across tile sets)
        std::string _name = ""; // should be structured like this: "group":"name"
        // _unique_id is a unique identifier within the tile set
        uint32_t _unique_id = 0;
        uint32_t _tile_map_id = 0; // id of the tile image stored in the tile map
            
        std::vector<Neighbour> _neighbours;

    public:

        void setPossibleNeighbour(uint32_t unique_id, float propability, uint32_t direction);
        void removePossibleNeighbour(uint32_t unique_id);

        Neighbour* getNeighbour(uint32_t unique_id);

        std::vector<std::pair<uint32_t, float>> getPossibleNeighbours(uint32_t direction) const;

        void setName(const std::string& name);
        void setUniqueID(uint32_t id);
        void setTileMapID(uint32_t id);
        std::string getName() const;
        uint32_t getUniqueID() const;
        uint32_t getTileMapID() const;
        uint32_t getNeighbourCount() const;
        const std::vector<Neighbour>& getAllNeighbours() const;

    };

}

#endif // TONK_TILE_H