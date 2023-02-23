#include "world/drawable_world.h"
#include "debug.h"
#include "glm/gtx/string_cast.hpp"
#include "algorithm"

namespace cell {

    void DrawableWorld::init(const undicht::vulkan::LogicalDevice& device) {
        
        _sun.setType(Light::Type::Directional);
        _cell_buffer.init(device);
        _light_buffer.init(device);
        _materials.init(device);

    }

    void DrawableWorld::cleanUp() {

        _cell_buffer.cleanUp();
        _light_buffer.cleanUp();
        _materials.cleanUp();
    }

    ///////////////////////////////////////////// update parts of the drawable world //////////////////////////////////////////

    CellWorld& DrawableWorld::getCellWorld() {

        return _cell_world;
    }

    LightWorld& DrawableWorld::getLightWorld() {

        return _light_world;
    }

    void DrawableWorld::setSunDirection(const glm::vec3& dir) {

        _sun.setDirection(dir);

    }

    void DrawableWorld::setSunColor(const glm::vec3& color) {

        _sun.setColor(color);
    }


    // make sure the chunk is correctly stored in the world buffer
    void DrawableWorld::updateWorldBuffer() {

        for(int i = 0; i < _cell_world.getLoadedChunks().size(); i++) {

            const CellChunk* chunk = (CellChunk*)_cell_world.getLoadedChunks().at(i);
            const glm::ivec3& chunk_pos = _cell_world.getChunkPositions().at(i);

            _cell_buffer.updateChunk(*chunk, chunk_pos);
        }

    }

    void DrawableWorld::updateLightBuffer() {

        // make sure each light chunk is correctly stored in the light buffer
        for(int i = 0; i < _light_world.getLoadedChunks().size(); i++) {

            const LightChunk* chunk = (LightChunk*)_light_world.getLoadedChunks().at(i);
            const glm::ivec3& chunk_pos = _light_world.getChunkPositions().at(i);

            _light_buffer.updateChunk(*chunk, chunk_pos);
        }

        /*uint32_t lights_to_update = std::max((uint32_t)_lights.size(), _light_buffer.getPointLightCount());

        for(int i = 0; i < lights_to_update; i++) {
            
            if(i < _lights.size()) {
                // update / add the light
                if(i < _light_buffer.getPointLightCount()) {
                    _light_buffer.updatePointLight(i, _lights.at(i)); // update
                } else {
                    _light_buffer.addPointLight(_lights.at(i)); // add
                }
            } else {
                // remove the entry in the light buffer
                _light_buffer.freePointLight(i);
            }

        }*/

    }

    /////////////////////////////////// access parts of the drawable world (for rendering) ///////////////////////////////////

    const CellBuffer& DrawableWorld::getCellBuffer() const {

        return _cell_buffer;
    }

    const LightBuffer& DrawableWorld::getLightBuffer() const {

        return _light_buffer;
    }

    MaterialAtlas& DrawableWorld::getMaterialAtlas() {

        return _materials;
    }

    const Light& DrawableWorld::getSun() const {

        return _sun;
    }

} // namespace cell