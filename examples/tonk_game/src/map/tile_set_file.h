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

    };

} // tonk

#endif // TONK_TILE_SET_FILE_H