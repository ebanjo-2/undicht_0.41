#include "world/lights/light.h"

namespace cell {

    using namespace undicht;

    // vec3: position
    // vec3: color
    // float: range
    const BufferLayout POINT_LIGHT_LAYOUT({UND_UINT32, UND_VEC3F, UND_VEC3F, UND_FLOAT32});

    // vec3: direction
    // vec3: color
    const BufferLayout DIRECT_LIGHT_LAYOUT({UND_UINT32, UND_VEC3F, UND_VEC3F});

    Light::Light(Type type) {

        setType(type);
    }

    Light::Light(Type type, const glm::vec3& pos_or_dir, const glm::vec3& color) {

        setType(type);
        setPosition(pos_or_dir);
        setDirection(pos_or_dir);
        setColor(color);

    }

    void Light::setType(Type type) {

        _type = type;
    }

    void Light::setPosition(const glm::vec3& pos) {
        // for point lights only

        if(_type == Type::Point)
            _pos_or_dir = pos;

    }

    void Light::setDirection(const glm::vec3& dir) {
        // for directional lights only

        if(_type == Type::Directional)
            _pos_or_dir = glm::normalize(dir);
    }

    void Light::setColor(const glm::vec3& color) {
        
        _color = color;

        // point light range
        float brightness = glm::length(color);
        _range = glm::sqrt(brightness / 0.005);
    }

    const glm::vec3& Light::getPosition() const {
        // for point lights only

        return _pos_or_dir;
    } 
    const glm::vec3& Light::getDirection() const {
        // for directional lights only

        return _pos_or_dir;
    } 
    const glm::vec3& Light::getColor() const {

        return _color;
    }

    uint32_t Light::fillBuffer(char* buffer) const {
        /// @brief stores the point light data in the buffer
        /// @param buffer if not nullptr, the data of the pointlight will be stored (layout as described by POINT_LIGHT_LAYOUT)
        /// @return the size of the data that gets stored (in bytes)
        
        uint32_t* ubuffer = (uint32_t*)buffer;
        float* fbuffer = (float*)ubuffer + 1;

        if(buffer) {
            
            ubuffer[0] = _type;

            fbuffer[0] = _pos_or_dir.x;
            fbuffer[1] = _pos_or_dir.y;
            fbuffer[2] = _pos_or_dir.z;

            fbuffer[3] = _color.x;
            fbuffer[4] = _color.y;
            fbuffer[5] = _color.z;

            if(_type == Point)
                fbuffer[6] = _range;
        }

        if(_type == Point)
            return 7 * sizeof(float) + sizeof(uint32_t);
        else 
            return 6 * sizeof(float) + sizeof(uint32_t);
    }

    uint32_t Light::loadFromData(const char* buffer) {
        /// @brief init the point light from data
        /// same layout and size as used by fillBuffer() is expected
        /// @return the number of bytes used

        uint32_t* ubuffer = (uint32_t*)buffer;
        float* fbuffer = (float*)ubuffer + 1;

        if(buffer) {
            // just reverse of what fillBuffer() does

            _type = (Light::Type)ubuffer[0];

            _pos_or_dir.x = fbuffer[0];
            _pos_or_dir.y = fbuffer[1];
            _pos_or_dir.z = fbuffer[2];

            _color.x = fbuffer[3];
            _color.y = fbuffer[4];
            _color.z = fbuffer[5];

            if(_type == Point)
                _range = fbuffer[6];
        } else {
            return 0;
        }

        if(_type == Point)
            return 7 * sizeof(float) + sizeof(uint32_t);
        else 
            return 6 * sizeof(float) + sizeof(uint32_t);
    }

} // cell