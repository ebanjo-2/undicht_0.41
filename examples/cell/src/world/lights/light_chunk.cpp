#include "world/lights/light_chunk.h"

namespace cell {

    uint32_t LightChunk::addLight(const Light& light) {

        _lights.push_back(light);
        _has_changed = true;

        return _lights.size() - 1;
    }

    void LightChunk::updateLight(uint32_t id, const Light& light) {

        if(id < _lights.size()) {
            _lights.at(id) = light;
            _has_changed = true;
        }
        
    }

    void LightChunk::freeLight(uint32_t id) {

        if(id < _lights.size()) {
            _lights.erase(_lights.begin() + id);
            _has_changed = true;
        }

    }

    uint32_t LightChunk::fillBuffer(char* buffer) const {
        // store the contents of the chunk in the buffer (if buffer != nullptr), return size of elements stored

        uint32_t light_size = POINT_LIGHT_LAYOUT.getTotalSize();

        if(buffer != nullptr) {

            for(int i = 0; i < _lights.size(); i++) {

                _lights.at(i).fillBuffer(buffer + i * light_size);
            }

        }

        return _lights.size() * light_size;
    }

    void LightChunk::loadFromBuffer(const char* buffer, uint32_t byte_size) {
        // initialize the complete data of the chunk from the buffer

        /*_lights.clear();
        uint32_t light_size = POINT_LIGHT_LAYOUT.getTotalSize();

        if(buffer != nullptr)
            _lights.insert(_lights.begin(), buffer, buffer + byte_size);*/
    }
    
    void LightChunk::loadFromBuffer(const std::vector<Light>& buffer) {

        _lights = buffer;
    }

} // cell