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
        XmlElement* tonk_tile_set = getElement({"TONKTILESET"});

        // loading all groups
        std::vector<XmlElement*> tile_groups = tonk_tile_set->getAllElements({"group"});
        for(XmlElement* group : tile_groups) {

            // getting the group name
            const XmlTagAttrib* group_name = group->getAttribute("name");

            if(!group_name) {
                UND_LOG << "failed to load tile group: no name could be found\n";
                continue;
            }

            // loading the tiles from this group
            std::vector<XmlElement*> tiles = group->getAllElements({"tile"});
            for(XmlElement* tile : tiles) {
                
                // loading the tile name
                const XmlTagAttrib* name = tile->getAttribute("name");
                if(!name){
                    UND_LOG << "failed to load tile: no name could be found\n";
                    continue;
                }

                unsigned tile_id = tile_set.addTile(group_name->m_value + "::" + name->m_value);

                // load the tile image to the tile map
                const XmlTagAttrib* file = tile->getAttribute("file");
                if(!name) {
                    UND_LOG << "failed to load tile: no image file was specified \n";
                    continue;
                }

                tile_map.setTile(tile_id, getFilePath(m_file_name) + file->m_value.substr( 1, file->m_value.size() - 2));

                // loading the tiles features
                std::vector<XmlElement*> features = tile->getAllElements({"feature"});
                for(XmlElement* feature : features) {

                    const XmlTagAttrib* neighbour = feature->getAttribute("neighbour");
                    if(!neighbour) {
                        UND_LOG << "failed to load propability feature: no neighbour specified\n";
                        continue;
                    }

                    // extracting the propabilities
                    std::string propability_string = feature->getContent();
                    std::vector<float> propabilities;
                    extractFloatArray(propabilities, propability_string, 4);

                    // adding the propability feature to the set
                    tile_set.addTileProbabilityFeature(group_name->m_value + "::" + name->m_value, neighbour->m_value, propabilities.at(0), propabilities.at(1), propabilities.at(2), propabilities.at(3));

                }
                
            }

        }

        tile_set.calcFinalPropabilities();

    }

} // tonk