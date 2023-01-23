#include "point_light.h"

namespace cell {

    using namespace undicht;

    const BufferLayout POINT_LIGHT_LAYOUT({UND_VEC3F, UND_VEC3F, UND_FLOAT32});
    // vec3: position
    // vec3: color
    // float: range

    PointLight::PointLight() {

    }

    PointLight::PointLight(const glm::vec3& pos, const glm::vec3& color) {
        
        setPosition(pos);
        setColor(color);

    }

    void PointLight::setPosition(const glm::vec3& pos) {

        _pos = pos;
    }   

    void PointLight::setColor(const glm::vec3& color) {

        _color = color;

        // calculating the distance at which the brightness of the light is less then 0.0005
        // radiance = brightness / (distance²)
        float brightness = glm::length(color);
        _range = glm::sqrt(brightness / 0.001);

    }

    uint32_t PointLight::fillBuffer(float* buffer) const {
        /// @brief stores the point light data in the buffer
        /// @param buffer if not nullptr, the data of the pointlight will be stored (layout as described by )
        /// @return the size of the data that gets stored

        buffer[0] = _pos.x;
        buffer[1] = _pos.y;
        buffer[2] = _pos.z;

        buffer[3] = _color.x;
        buffer[4] = _color.y;
        buffer[5] = _color.z;

        buffer[6] = _range;

        return 7 * sizeof(float);
    }

} // cell