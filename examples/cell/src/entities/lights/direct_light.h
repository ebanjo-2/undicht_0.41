#ifndef DIRECT_LIGHT_H
#define DIRECT_LIGHT_H

#include <cstdint>
#include "glm/glm.hpp"
#include "buffer_layout.h"

namespace cell {

    const extern undicht::BufferLayout DIRECT_LIGHT_LAYOUT;

    class DirectLight {
        // short for directional light (a light with light rays that are all parallel, as if the light source was at an infinite distance )
      protected:

        glm::vec3 _direction;
        glm::vec3 _color; 

      public:

        DirectLight();
        DirectLight(const glm::vec3& direction, const glm::vec3& color);

        void setDirection(const glm::vec3& direction);
        void setColor(const glm::vec3& color);

        /// @brief stores the point light data in the buffer
        /// @param buffer if not nullptr, the data of the pointlight will be stored (layout as described by DIRECT_LIGHT_LAYOUT)
        /// @return the size of the data that gets stored
        uint32_t fillBuffer(float* buffer) const;
    };

} // cell

#endif // DIRECT_LIGHT_H