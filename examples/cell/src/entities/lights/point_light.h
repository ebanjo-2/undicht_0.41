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
        float _range;

        // parameters for the attenuation function, which describes the loss of light intensity with distance
        // Attenuation = Constant + Linear * Distance + Quadratic * Distance^2 
        // src: https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
        // the Constant is always set to 1
        float _a_linear = 0.7f; 
        float _a_quadr = 1.8f; 

      public:

        PointLight();
        PointLight(const glm::vec3& pos, const glm::vec3& color, float range);

        void setPosition(const glm::vec3& pos);
        void setColor(const glm::vec3& color);
        void setRange(float range);

        /// @brief stores the point light data in the buffer
        /// @param buffer if not nullptr, the data of the pointlight will be stored (layout as described by )
        /// @return the size of the data that gets stored
        uint32_t fillBuffer(float* buffer) const;


    };

} // namespace cell

#endif