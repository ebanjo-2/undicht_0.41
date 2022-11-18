#include "map.h"
#include "vector"
#include "buffer_layout.h"

namespace tonk {

    using namespace undicht;
    using namespace vulkan;

    const std::vector<float> SQUARE_VERTICES = {
        0.0f, 1.0f, // top left
        1.0f, 1.0f, // top right
        1.0f, 0.0f, // bottom right
        0.0f, 0.0f, // bottom left
    };

    std::vector<unsigned> SQUARE_INDICES = {
        0, 1, 3, 1, 2, 3, // defined in clockwise order
    };

    const BufferLayout VERTEX_LAYOUT({UND_VEC2F}); // vec2 doubling as position and uv
    const BufferLayout INSTANCE_LAYOUT({UND_VEC2F, UND_VEC2F, UND_VEC2F}); // location of tile in world, location of tile in tilemap texture (uv0 + uv1)

    void Map::init(const LogicalDevice& device) {

        _vertex_buffer.init(device);

        _vertex_buffer.setVertexData(SQUARE_VERTICES.data(), SQUARE_VERTICES.size() * sizeof(float), 0);
        _vertex_buffer.setIndexData(SQUARE_INDICES.data(), SQUARE_INDICES.size() * sizeof(unsigned), 0);

    }

    void Map::cleanUp() {

        _vertex_buffer.cleanUp();

    }

    void Map::setSize(uint32_t width, uint32_t height) {
        // reserving memory in the vertex buffer

        _width = width;
        _height = height;

        _tiles.resize(width * height, -1);

        _vertex_buffer.setInstanceData(0, 0, width * height * INSTANCE_LAYOUT.getTotalSize());
    }

    void Map::setTile(uint32_t tile_id, uint32_t x, uint32_t y, const TileMap& tile_map) {

        _tiles.at(x + y * _width) = tile_id;
        
        // calculating the position of the tile on the tile map
        float u0, v0, u1, v1;
        tile_map.calcUVs(tile_id, u0, v0, u1, v1);

        float per_vertex_data[] = {
            float(x), float(y),
            u0, v0, u1, v1,
        };

        _vertex_buffer.setInstanceData(per_vertex_data, INSTANCE_LAYOUT.getTotalSize(), (x + y * _width) * INSTANCE_LAYOUT.getTotalSize());

    } 

    void Map::clear() {
        
        _tiles.clear();
    }


    const undicht::vulkan::VertexBuffer& Map::getVertexBuffer() const {

        return _vertex_buffer;
    }

    uint32_t Map::getVertexCount() const {
        
        return 6; // 2 triangles
    }

    bool Map::getDrawIndexed() const {

        return true;
    }

    uint32_t Map::getInstanceCount() const {

        return _width * _height;
    }

    uint32_t Map::getWidth() const {

        return _width;
    }

    uint32_t Map::getHeight() const {

        return _height;
    }

    uint32_t Map::getTile(uint32_t x, uint32_t y) const {

        if( x >= 0 && x < _width && y >= 0 && y < _height)
            return _tiles.at(x + y * _width);
        else 
            return -1;
    }

} // tonk