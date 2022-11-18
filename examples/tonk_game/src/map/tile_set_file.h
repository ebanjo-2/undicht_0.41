#ifndef TONK_TILE_SET_FILE_H
#define TONK_TILE_SET_FILE_H

#include "string"
#include "xml/xml_file.h"

#include "map/tile_set.h"
#include "map/tile_map.h"

namespace tonk {

    class TileSetFile : public undicht::tools::XmlFile {

    protected:

    public:

        TileSetFile(const std::string& file_name);
        bool open(const std::string& file_name);

        void loadTileSet(TileSet& tile_set, TileMap& map);
        void savePropabilities(const TileSet& tile_set);


    protected:
        // private functions for loading tiles

        void loadTileGroup(const undicht::tools::XmlElement* group_data, TileSet& tile_set, TileMap& tile_map) const;
        void loadTile(const undicht::tools::XmlElement* tile_data, const std::string& group_name, Tile& tile, TileSet& tile_set, TileMap& tile_map) const;
        void loadTileToTileMap(const undicht::tools::XmlElement* tile_data, TileMap& tile_map, uint32_t tile_id) const;

    };

} // tonk

#endif // TONK_TILE_SET_FILE_H