#include "tile_map.h"
#include "core/vulkan/formats.h"
#include "images/image_file.h"
#include "debug.h"

using namespace undicht;
using namespace vulkan;
using namespace tools;

const int TILE_WIDTH = 16;
const int TILE_HEIGHT = 16;
const int TILE_MAP_WIDTH = TILE_WIDTH * 64; // 16 * 64 = 1024
const int TILE_MAP_HEIGHT = TILE_HEIGHT * 64; // 16 * 64 = 1024
const int TILE_MAP_COLS = TILE_MAP_WIDTH / TILE_WIDTH;
const int TILE_MAP_ROWS = TILE_MAP_HEIGHT / TILE_HEIGHT;
const FixedType TILE_MAP_FORMAT = UND_R8G8B8A8_SRGB;



namespace tonk {

    void TileMap::init(const undicht::vulkan::LogicalDevice& device) {
        
        _map.setExtent(TILE_MAP_WIDTH, TILE_MAP_HEIGHT);
        _map.setFormat(translate(TILE_MAP_FORMAT));
        _map.init(device);
    }

    void TileMap::cleanUp() {
        
        _map.cleanUp();
    }

    void TileMap::setTile(int id, const std::string& file_name) {
        
        ImageData data;
        ImageFile(file_name, data);

        if(data._nr_channels != TILE_MAP_FORMAT.m_num_components || data._width != TILE_WIDTH || data._height != TILE_HEIGHT) {

            UND_ERROR << "failed to load tile: " << file_name << "\n";
            return;
        }

        int pos_x = (id % TILE_MAP_COLS) * TILE_WIDTH;
        int pos_y = (id / TILE_MAP_COLS) * TILE_HEIGHT;

        _map.setData(data._pixels.data(), data._pixels.size(), 0, {TILE_WIDTH, TILE_HEIGHT, 1}, {pos_x, pos_y, 0});
        
    }

    const undicht::vulkan::Texture& TileMap::getMap() const {

        return _map;
    }

    uint32_t TileMap::getWidth() const {
        
        return TILE_MAP_WIDTH;
    }

    uint32_t TileMap::getHeight() const {

        return TILE_MAP_HEIGHT;
    }

    uint32_t TileMap::getTileWidth() const {
        
        return TILE_WIDTH;
    }

    uint32_t TileMap::getTileHeight() const {

        return TILE_HEIGHT;
    }

    void TileMap::calcUVs(uint32_t id, float& u0, float& v0, float& u1, float& v1) const{

        uint32_t num_cols = getWidth() / getTileWidth();
        uint32_t num_rows = getHeight() / getTileHeight();

        u0 = ((id % num_cols)) / float(num_cols);
        v0 = ((id / num_cols)) / float(num_rows);

        u1 = u0 + getTileWidth() / float(getWidth());
        v1 = v0 + getTileHeight() / float(getHeight());
    }


} // tonk