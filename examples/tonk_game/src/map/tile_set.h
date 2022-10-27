#ifndef TONK_TILE_SET_H
#define TONK_TILE_SET_H

#include "vector"
#include "string"


namespace tonk {

    class TileSet {
        // contains the tile "rules"
        // probabilities of a tile being placed next to another tile

    protected:

        std::vector<std::string> _tile_names;

        std::vector<std::vector<float>> _up_propabilities;
        std::vector<std::vector<float>> _down_propabilities;
        std::vector<std::vector<float>> _right_propabilities;
        std::vector<std::vector<float>> _left_propabilities;

        // propabilities for a cell with no neighbours
        std::vector<float> _start_propabilities;

    public:

        unsigned addTile(const std::string& tile_name); // returns the id of the tile
        void addTileProbabilityFeature(const std::string& tile_name, const std::string& neighbour_name, float up, float down, float right, float left);
        void addTileProbabilityFeature(unsigned tile_id, unsigned neighbour_id, float up, float down, float right, float left);
        void calcFinalPropabilities();

        unsigned getTileId(const std::string& tile_name) const;
        std::string getTileName(unsigned tile_id) const;
        unsigned getTileCount() const;

        void getStartPropabilities(std::vector<float>& propabilities) const;

        void getUpPropabilities(unsigned tile_id, std::vector<float>& propabilities) const;
        void getDownPropabilities(unsigned tile_id, std::vector<float>& propabilities) const;
        void getRightPropabilities(unsigned tile_id, std::vector<float>& propabilities) const;
        void getLeftPropabilities(unsigned tile_id, std::vector<float>& propabilities) const;

    };

} // tonk

#endif // TONK_TILE_SET_H