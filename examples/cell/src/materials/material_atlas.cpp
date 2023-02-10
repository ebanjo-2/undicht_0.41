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
    const FixedType MaterialAtlas::TILE_MAP_FORMAT = UND_R8G8B8A8_SRGB;

    void MaterialAtlas::init(const undicht::vulkan::LogicalDevice& device) {

        _tile_map.setExtent(TILE_MAP_WIDTH, TILE_MAP_HEIGHT, 1, 2);
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

        // loading the textures
        ImageData diffuse_data;
        ImageData specular_data;
        loadAlbedoTexture(mat.getAlbedoTexture(), diffuse_data);
        loadNormalTexture(mat.getNormalTexture(), specular_data);

        int pos_x = (fixed_id % TILE_MAP_COLS) * TILE_WIDTH;
        int pos_y = (fixed_id / TILE_MAP_COLS) * TILE_HEIGHT;

        _tile_map.setData(diffuse_data._pixels.data(), diffuse_data._pixels.size(), 0, {TILE_WIDTH, TILE_HEIGHT, 1}, {pos_x, pos_y, 0});
        _tile_map.setData(specular_data._pixels.data(), specular_data._pixels.size(), 1, {TILE_WIDTH, TILE_HEIGHT, 1}, {pos_x, pos_y, 0});
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

    //////////////////////////////////// protected material atlas functions ////////////////////////////////////

    void MaterialAtlas::loadAlbedoTexture(const std::string& file_name, ImageData& data) {

        if(!file_name.compare("")) {
            // init the data with default values
            data._nr_channels = 4;
            data._width = 16;
            data._height = 16;
            data._pixels.resize(16 * 16 * 4);

            for(int x = 0; x < 16; x++) {
                for(int y = 0; y < 16; y++) {
                    data._pixels.at(4 * (x * 16 + y) + 0) = 0;
                    data._pixels.at(4 * (x * 16 + y) + 1) = 100;
                    data._pixels.at(4 * (x * 16 + y) + 2) = 200;
                    data._pixels.at(4 * (x * 16 + y) + 3) = 0; // roughness
                }
            }

            return;
        }

        ImageFile(file_name, data);

        if(data._nr_channels != TILE_MAP_FORMAT.m_num_components || data._width != TILE_WIDTH || data._height != TILE_HEIGHT) {
            UND_ERROR << "failed to load Albedo + Roughness texture: " << file_name << "\n";
            return;
        }

    }

    void MaterialAtlas::loadNormalTexture(const std::string& file_name, ImageData& data) {

        if(!file_name.compare("")) {
            // init the data with default values
            data._nr_channels = 4;
            data._width = 16;
            data._height = 16;
            data._pixels.resize(16 * 16 * 4);

            for(int x = 0; x < 16; x++) {
                for(int y = 0; y < 16; y++) {
                    data._pixels.at(4 * (x * 16 + y) + 0) = 0; // normal
                    data._pixels.at(4 * (x * 16 + y) + 1) = 255;
                    data._pixels.at(4 * (x * 16 + y) + 2) = 0;
                    data._pixels.at(4 * (x * 16 + y) + 3) = 0; // metalness
                }
            }

            return;
        }

        ImageFile(file_name, data);

        if(data._nr_channels != TILE_MAP_FORMAT.m_num_components || data._width != TILE_WIDTH || data._height != TILE_HEIGHT) {
            UND_ERROR << "failed to load Normal + Metalness texture: " << file_name << "\n";
            return;
        }

    }

} // cell