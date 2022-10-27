#ifndef TONK_TILE_MAP_H
#define TONK_TILE_MAP_H

#include "renderer/vulkan/texture.h"
#include "string"
#include "core/vulkan/logical_device.h"

namespace tonk {

    class TileMap {

        undicht::vulkan::Texture _map;

    public:

        void init(const undicht::vulkan::LogicalDevice& device);
        void cleanUp();

        void setTile(int id, const std::string& file_name);

        const undicht::vulkan::Texture& getMap() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
        uint32_t getTileWidth() const;
        uint32_t getTileHeight() const;

    };

} // tonk

#endif // TONK_TILE_MAP_H