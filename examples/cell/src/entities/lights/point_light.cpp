#include "point_light.h"

namespace cell {

    using namespace undicht;

    const BufferLayout POINT_LIGHT_LAYOUT({UND_VEC3F, UND_VEC3F, UND_FLOAT32, UND_FLOAT32, UND_FLOAT32});
    // vec3: position
    // vec3: color
    // float: range
    // float: linear attenuation factor
    // float: quadratic attenuation factor


    PointLight::PointLight() {

    }

    PointLight::PointLight(const glm::vec3& pos, const glm::vec3& color, float range) {
        
        setPosition(pos);
        setColor(color);
        setRange(range);

    }

    void PointLight::setPosition(const glm::vec3& pos) {

        _pos = pos;
    }   

    void PointLight::setColor(const glm::vec3& color) {

        _color = color;
    }

    void PointLight::setRange(float range) {
        // calculating values for the attenuation constants
        // https://wiki.ogre3d.org/Light+Attenuation+Shortcut

        _range = range;

        _a_linear = 4.5f / range;
        _a_quadr = 75.0f / (range * range);
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
        buffer[7] = _a_linear;
        buffer[8] = _a_quadr;

        return 9 * sizeof(float);
    }

} // cell