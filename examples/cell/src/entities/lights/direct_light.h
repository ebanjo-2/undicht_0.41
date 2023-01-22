#ifndef DIRECT_LIGHT_H
#define DIRECT_LIGHT_H

#include <cstdint>
#include "glm/glm.hpp"
#include "buffer_layout.h"
#include "math/orthographic_projection.h"

namespace cell {

    const extern undicht::BufferLayout DIRECT_LIGHT_LAYOUT;

    class DirectLight {
        // short for directional light (a light with light rays that are all parallel, as if the light source was at an infinite distance )
      public:

        glm::vec3 _direction;
        glm::vec3 _color;

        // for shadow mapping
        glm::vec3 _shadow_origin;
        glm::mat4 _shadow_view;
        glm::mat4 _shadow_proj;
        float _shadow_proj_width = 20.0f;
        float _shadow_proj_height = 20.0f;

      public:

        DirectLight();
        DirectLight(const glm::vec3& direction, const glm::vec3& color, const glm::vec3 shadow_origin);

        void setDirection(const glm::vec3& direction);
        void setColor(const glm::vec3& color);
        void setShadowOrigin(const glm::vec3& shadow_origin);
        void setShadowProjSize(float proj_width, float proj_height); // the area the projection matrix should cover

        const glm::vec3& getDirection() const;
        const glm::vec3& getColor() const;
        const glm::vec3& getShadowOrigin() const;
        const glm::mat4& getShadowView() const;
        const glm::mat4& getShadowProj() const;

      protected:
        // private functions

        void updateShadowMatrices();

    };

} // cell

#endif // DIRECT_LIGHT_H