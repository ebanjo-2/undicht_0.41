#ifndef TONK_MAP_H
#define TONK_MAP_H

#include "vector"
#include "core/vulkan/logical_device.h"
#include "renderer/vulkan/vertex_buffer.h"
#include "tile_map.h"

namespace tonk {

    class Map {

    protected:

        undicht::vulkan::VertexBuffer _vertex_buffer;

        uint32_t _width = 0;
        uint32_t _height = 0;

        std::vector<uint32_t> _tiles; // a copy of the map in cpu accessible ram

    public:

        void init(const undicht::vulkan::LogicalDevice& device);
        void cleanUp();

        void setSize(uint32_t width, uint32_t height);
        void setTile(uint32_t tile_id, uint32_t x, uint32_t y, const TileMap& tile_map); // 0,0 is top left

        void clear();

        const undicht::vulkan::VertexBuffer& getVertexBuffer() const;
        uint32_t getVertexCount() const;
        bool getDrawIndexed() const;
        uint32_t getInstanceCount() const;

        uint32_t getWidth() const;
        uint32_t getHeight() const;

        uint32_t getTile(uint32_t x, uint32_t y) const;

    };

} // tonk

#endif // TONK_MAP_H