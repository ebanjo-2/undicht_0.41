#include "direct_light.h"

namespace cell {

    using namespace undicht;

    const BufferLayout DIRECT_LIGHT_LAYOUT({UND_VEC3F, UND_VEC3F});
    // vec3: position
    // vec3: color

    DirectLight::DirectLight() {

    }

    DirectLight::DirectLight(const glm::vec3& direction, const glm::vec3& color) {

        setDirection(direction);
        setColor(color);

    }

    void DirectLight::setDirection(const glm::vec3& direction) {

        _direction = direction;
    }

    void DirectLight::setColor(const glm::vec3& color) {

        _color = color;
    }

    uint32_t DirectLight::fillBuffer(float* buffer) const {
        /// @brief stores the point light data in the buffer
        /// @param buffer if not nullptr, the data of the pointlight will be stored (layout as described by )
        /// @return the size of the data that gets stored

        if(buffer != nullptr) {

        }
        
        return 6 * sizeof(float);
    }

}