#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "glm/glm.hpp"
#include "buffer_layout.h"

namespace cell {

    const extern undicht::BufferLayout POINT_LIGHT_LAYOUT;

    class PointLight {

      protected:

        glm::vec3 _pos;
        glm::vec3 _color; 

        float _range; // gets calculated from the brightness of the light (length of the color vector)

      public:

        PointLight();
        PointLight(const glm::vec3& pos, const glm::vec3& color);

        void setPosition(const glm::vec3& pos);
        void setColor(const glm::vec3& color);

        /// @brief stores the point light data in the buffer
        /// @param buffer if not nullptr, the data of the pointlight will be stored (layout as described by POINT_LIGHT_LAYOUT)
        /// @return the size of the data that gets stored
        uint32_t fillBuffer(float* buffer) const;


    };

} // namespace cell

#endif