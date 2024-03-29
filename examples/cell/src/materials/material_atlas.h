#ifndef MATERIAL_ATLAS_H
#define MATERIAL_ATLAS_H

#include "types.h"
#include "renderer/vulkan/texture.h"
#include "materials/material.h"
#include "vector"
#include "images/image_file.h"
#include "core/vulkan/command_buffer.h"
#include "renderer/vulkan/transfer_buffer.h"

namespace cell {

    class MaterialAtlas {

      public:
        // constants

        const static int32_t TILE_WIDTH;
        const static int32_t TILE_HEIGHT;
        const static int32_t TILE_MAP_WIDTH;
        const static int32_t TILE_MAP_HEIGHT;
        const static int32_t TILE_MAP_COLS;
        const static int32_t TILE_MAP_ROWS;
        const static undicht::FixedType TILE_MAP_FORMAT;
    
      protected:

        undicht::vulkan::LogicalDevice _device_handle;
        undicht::vulkan::Texture _tile_map;
        std::vector<Material> _materials;

      public:

        void init(const undicht::vulkan::LogicalDevice& device);
        void cleanUp();

        /// @brief adds/updates the material on the material atlas
        /// @param mat the material to update / add
        /// @return the id with which the material is associated and with which it can be accessed in the shader
        uint32_t setMaterial(const Material& mat, undicht::vulkan::CommandBuffer& cmd, undicht::vulkan::TransferBuffer& buf);
        void setMaterial(const Material& mat, uint32_t fixed_id, undicht::vulkan::CommandBuffer& cmd, undicht::vulkan::TransferBuffer& buf);

        const Material* getMaterial(const std::string& mat_name) const; // nullptr if the material wasnt found
        const Material* getMaterial(uint32_t id) const; // nullptr if the material wasnt found
        int getMaterialID(const std::string& mat_name) const; // -1 if the material wasnt found

        const undicht::vulkan::Texture& getTileMap() const;

      protected:
        // protected material atlas functions

        void loadAlbedoTexture(const std::string& file_name, undicht::tools::ImageData<char>& data);
        void loadNormalTexture(const std::string& file_name, undicht::tools::ImageData<char>& data);

    };

} // cell

#endif // MATERIAL_ATLAS_H