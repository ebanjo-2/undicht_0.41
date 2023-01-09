#include "material_atlas.h"
#include "types.h"
#include "core/vulkan/formats.h"
#include "images/image_file.h"
#include "debug.h"

namespace cell {

    using namespace undicht;
    using namespace vulkan;
    using namespace tools;

    const int32_t MaterialAtlas::TILE_WIDTH = 16;
    const int32_t MaterialAtlas::TILE_HEIGHT = 16;
    const int32_t MaterialAtlas::TILE_MAP_WIDTH = TILE_WIDTH * 64; // 16 * 64 = 1024
    const int32_t MaterialAtlas::TILE_MAP_HEIGHT = TILE_HEIGHT * 64; // 16 * 64 = 1024
    const int32_t MaterialAtlas::TILE_MAP_COLS = TILE_MAP_WIDTH / TILE_WIDTH;
    const int32_t MaterialAtlas::TILE_MAP_ROWS = TILE_MAP_HEIGHT / TILE_HEIGHT;
    const FixedType MaterialAtlas::TILE_MAP_FORMAT = UND_R8G8B8A8;

    void MaterialAtlas::init(const undicht::vulkan::LogicalDevice& device) {

        _tile_map.setExtent(TILE_MAP_WIDTH, TILE_MAP_HEIGHT);
        _tile_map.setFormat(translate(TILE_MAP_FORMAT));
        _tile_map.setMipMaps(false);
        _tile_map.init(device);

        _materials.resize(TILE_MAP_COLS * TILE_MAP_ROWS);

    }

    void MaterialAtlas::cleanUp() {

        _tile_map.cleanUp();
    }

    uint32_t MaterialAtlas::setMaterial(const Material& mat) {
        /// @brief adds/updates the material on the material atlas
        /// @param mat the material to update / add
        /// @return the id with which the material is associated and with which it can be accessed in the shader

        // looking if the material is already part of the atlas
        int id = getMaterialID(mat.getName());
        if(id != -1)
            setMaterial(mat, id);

        // adding the material to the atlas
        for(int i = 0; i < _materials.size(); i++) {
            if(!_materials.at(i).getName().size()) {
                id = i;
                break;
            }
        }

        setMaterial(mat, id);

        return id;
    }

    void MaterialAtlas::setMaterial(const Material& mat, uint32_t fixed_id) {

        if(fixed_id >= _materials.size())
            return;

        _materials.at(fixed_id) = mat;

        // loading the diffuse texture
        ImageData data;
        ImageFile(mat.getDiffuseTexture(), data);

        if(data._nr_channels != TILE_MAP_FORMAT.m_num_components || data._width != TILE_WIDTH || data._height != TILE_HEIGHT) {
            UND_ERROR << "failed to load diffuse texture: " << mat.getDiffuseTexture() << "\n";
            return;
        }

        int pos_x = (fixed_id % TILE_MAP_COLS) * TILE_WIDTH;
        int pos_y = (fixed_id / TILE_MAP_COLS) * TILE_HEIGHT;

        _tile_map.setData(data._pixels.data(), data._pixels.size(), {TILE_WIDTH, TILE_HEIGHT, 1}, {pos_x, pos_y, 0});
    }

    const Material* MaterialAtlas::getMaterial(const std::string& mat_name) const {

        return getMaterial(getMaterialID(mat_name));
    }
    
    const Material* MaterialAtlas::getMaterial(uint32_t id) const {

        if(id < _materials.size()) {
            return &_materials.at(id);
        }

        return nullptr;
    }

    int MaterialAtlas::getMaterialID(const std::string& mat_name) const {
        // -1 (unsigned) if the material wasnt found

        for(int i = 0; i < _materials.size(); i++) {

            if(!mat_name.compare(_materials.at(i).getName())) {
                return i;
            }

        }

        return -1;
    } 

    const undicht::vulkan::Texture& MaterialAtlas::getTileMap() const {

        return _tile_map;
    }

    
} // cell