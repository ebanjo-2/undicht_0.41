#ifndef LIGHT_H
#define LIGHT_H

#include "glm/glm.hpp"
#include "buffer_layout.h"

namespace cell {

    const extern undicht::BufferLayout POINT_LIGHT_LAYOUT;
    const extern undicht::BufferLayout DIRECT_LIGHT_LAYOUT;

    class Light {

      public:

        enum Type : uint32_t {
            Undefined,
            Point, 
            // Spotlight, // not jet implemented
            Directional
        };

      protected:

        Type _type;
        glm::vec3 _pos_or_dir; // meaning of this vector is dependant on the type of light
        glm::vec3 _color;

        float _range; // gets calculated from the brightness of the light (length of the color vector)

      public:

        Light(Type type = Undefined);
        Light(Type type, const glm::vec3& pos_or_dir, const glm::vec3& color);

        void setType(Type type);
        void setPosition(const glm::vec3& pos); // for point lights only
        void setDirection(const glm::vec3& dir); // for directional lights only
        void setColor(const glm::vec3& color);

        const Type& getType() const;
        const glm::vec3& getPosition() const; // for point lights only
        const glm::vec3& getDirection() const; // for directional lights only
        const glm::vec3& getColor() const;

        /// @brief stores the point light data in the buffer
        /// @param buffer if not nullptr, the data of the pointlight will be stored
        /// @return the size of the data that gets stored (in bytes)
        uint32_t fillBuffer(char* buffer) const;

        /// @brief init the point light from data
        /// same layout and size as used by fillBuffer() is expected
        /// @return the number of bytes used
        uint32_t loadFromData(const char* buffer);

    };

} // cell

#endif // LIGHT_H