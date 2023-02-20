#include "renderer/drawable_world.h"
#include "debug.h"
#include "glm/gtx/string_cast.hpp"
#include "algorithm"

namespace cell {

    void DrawableWorld::init(const undicht::vulkan::LogicalDevice& device) {

        _world_buffer.init(device);
        _light_buffer.init(device);
        _materials.init(device);

    }

    void DrawableWorld::cleanUp() {

        _world_buffer.cleanUp();
        _light_buffer.cleanUp();
        _materials.cleanUp();
    }

    ///////////////////////////////////////////// update parts of the drawable world //////////////////////////////////////////

    uint32_t DrawableWorld::addLight(const PointLight& light) {

        _lights.push_back(light);

        return _lights.size() - 1;
    }

    void DrawableWorld::removeLight(uint32_t light_id) {

        if(light_id < _lights.size())
            _lights.erase(_lights.begin() + light_id);

    }

    void DrawableWorld::removeAllLights() {

        _lights.clear();
    }

    void DrawableWorld::setSunDirection(const glm::vec3& dir) {

        _sun.setDirection(dir);

    }

    void DrawableWorld::setSunColor(const glm::vec3& color) {

        _sun.setColor(color);
    }

    void DrawableWorld::setSunTarget(const glm::vec3& target) {
        // for shadow mapping
        
        _sun.setShadowOrigin(target - 100.0f * _sun.getDirection());
    } 

    // make sure the chunk is correctly stored in the world buffer
    void DrawableWorld::updateWorldBuffer() {

        for(int i = 0; i < _loaded_chunks.size(); i++) {

            const Chunk& chunk = _loaded_chunks.at(i);
            const glm::ivec3& chunk_pos = _chunk_positions.at(i);

            _world_buffer.updateChunk(chunk, chunk_pos);
        }

    }

    void DrawableWorld::updateLightBuffer() {

        uint32_t lights_to_update = std::max((uint32_t)_lights.size(), _light_buffer.getPointLightCount());

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

        }

    }

    /////////////////////////////////// access parts of the drawable world (for rendering) ///////////////////////////////////

    const WorldBuffer& DrawableWorld::getWorldBuffer() const {

        return _world_buffer;
    }

    const LightBuffer& DrawableWorld::getLightBuffer() const {

        return _light_buffer;
    }

    const MaterialAtlas& DrawableWorld::getMaterialAtlas() const {

        return _materials;
    }

    const DirectLight& DrawableWorld::getSun() const {

        return _sun;
    }

} // namespace cell