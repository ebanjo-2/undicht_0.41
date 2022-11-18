#include "tile_set_file.h"
#include "file_tools.h"
#include "debug.h"

using namespace undicht;
using namespace tools;

namespace tonk {

    TileSetFile::TileSetFile(const std::string& file_name) {
        
        XmlFile::open(file_name);

    }

    bool TileSetFile::open(const std::string& file_name) {

        return XmlFile::open(file_name);
    }

    void TileSetFile::loadTileSet(TileSet& tile_set, TileMap& tile_map) {

        // get the TONKTILESET element
        const XmlElement* tonk_tile_set = getElement({"TONKTILESET"});

        // loading all groups
        std::vector<const XmlElement*> tile_groups = tonk_tile_set->getAllElements({"group"});
        for(const XmlElement* group : tile_groups) {

            loadTileGroup(group, tile_set, tile_map);    
        }

        // resolving neighbour ids
        tile_set.resolveNeighbourIDs();

    }

    void TileSetFile::savePropabilities(const TileSet& tile_set) {

        const XmlElement* tonk_tile_set = getElement({"TONKTILESET"});

        if(!tonk_tile_set) {
            UND_LOG << "failed to save tile propabilities: no tile set was loaded \n";
            return;
        }

        // updating the propabilities
        for(const Tile& tile : tile_set.getTiles()) {
            
            std::string tile_name = tile.getName();
            std::string group_name = tile_name.substr(0, tile_name.find(':'));
            tile_name.erase(0, group_name.size() + 2);

            // finding the xml element that stores this tiles data
            const XmlElement* group = tonk_tile_set->getElement({"group name=" + group_name});

            if(!group) {
                UND_LOG << "failed to store tile " << tile_name << " : group " << group_name << " not found\n";
                continue;
            }

            // finding the tile element
            const XmlElement* tile_element = group->getElement({"tile name=" + tile_name});

            if(!tile_element) {
                UND_LOG << "failed to store tile " << tile_name << " : tile not found\n";
                continue;
            }

            // storing the propability features
            for(const Tile::Neighbour& neighbour : tile.getAllNeighbours()) {
                
                std::string neighbour_name = tile_set.getTile(neighbour._neigbour_id)->getName();

                // finding the element that stores the neighbours propabilities
                XmlElement* feature = (XmlElement*)tile_element->getElement({"feature neighbour=" + neighbour_name});

                if(!feature) continue; // could be intentional (in the way that only one tile stores the other one as a possible neighbour)

                feature->m_content = toStr(neighbour._xp_propability) + " " + toStr(neighbour._xn_propability) + " " + toStr(neighbour._yp_propability) + " " + toStr(neighbour._yn_propability);
            }

        }

        // writing the changes to the file
        write(m_file_name);
    }

    void TileSetFile::loadTileGroup(const undicht::tools::XmlElement* group_data, TileSet& tile_set, TileMap& tile_map) const {
        
        // getting the group name
        const XmlTagAttrib* group_name = group_data->getAttribute("name");

        if(!group_name) {
            UND_LOG << "failed to load tile group: no name could be found\n";
            return;
        }

        // loading the tiles from this group
        std::vector<const XmlElement*> tiles = group_data->getAllElements({"tile"});
        for(const XmlElement* tile : tiles) {

            uint32_t next_tile_id = tile_set.getTileCount();

            // loading the tiles image
            loadTileToTileMap(tile, tile_map, next_tile_id);
            
            Tile new_tile;
            new_tile.setUniqueID(next_tile_id);
            new_tile.setTileMapID(next_tile_id);
            loadTile(tile, group_name->m_value, new_tile, tile_set, tile_map);

            // adding the new tile to the tile set
            tile_set.setTile(new_tile);
        }

    }

    void TileSetFile::loadTile(const undicht::tools::XmlElement* tile_data, const std::string& group_name, Tile& tile, TileSet& tile_set, TileMap& tile_map) const {

        // loading the tile name
        const XmlTagAttrib* name = tile_data->getAttribute("name");
        if(!name) {
            UND_LOG << "failed to load Tile: no name found!\n";
        }

        tile.setName(group_name + "::" + name->m_value);

        // loading the tiles neighbours
        std::vector<const XmlElement*> features = tile_data->getAllElements({"feature"});
        for(const XmlElement* feature : features) {

            const XmlTagAttrib* neighbour = feature->getAttribute("neighbour");
            if(!neighbour) {
                UND_LOG << "failed to load tile neighbour: \"neighbour\" attribute not specified\n";
                continue;
            }

            // extracting the propabilities
            std::string propability_string = feature->getContent();
            std::vector<float> propabilities;
            extractFloatArray(propabilities, propability_string, 4);

            // resolve the neighbours unique id
            tile_set.markFutureTile(neighbour->m_value);
            uint32_t neighbour_id = tile_set.getTileID(neighbour->m_value);

            tile.setPossibleNeighbour(neighbour_id, propabilities.at(0), TONK_X_POSITIVE);
            tile.setPossibleNeighbour(neighbour_id, propabilities.at(1), TONK_X_NEGATIVE);
            tile.setPossibleNeighbour(neighbour_id, propabilities.at(2), TONK_Y_POSITIVE);
            tile.setPossibleNeighbour(neighbour_id, propabilities.at(3), TONK_Y_NEGATIVE);

        }

    }

    void TileSetFile::loadTileToTileMap(const undicht::tools::XmlElement* tile_data, TileMap& tile_map, uint32_t tile_id) const {

        const XmlTagAttrib* image_file = tile_data->getAttribute("file");

        if(!image_file) {
            UND_LOG << "failed to load tile image: no image file was specified \n";
            return;
        }

        std::string file_name = getFilePath(m_file_name) + image_file->m_value.substr(1, image_file->m_value.size() - 2);
        tile_map.setTile(tile_id, file_name);
    }


} // tonk