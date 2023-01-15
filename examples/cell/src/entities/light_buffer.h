#ifndef LIGHT_BUFFER_H
#define LIGHT_BUFFER_H

#include "glm/glm.hpp"
#include "renderer/vulkan/vertex_buffer.h"
#include <vector>
#include "buffer_layout.h"

#include "entities/lights/point_light.h"

namespace cell {

    const extern undicht::BufferLayout LIGHT_VERTEX_LAYOUT; // per vertex data

    class LightBuffer {
    
    protected:

        undicht::vulkan::VertexBuffer _point_light_buffer;
        std::vector<bool> _lights_in_use;

    public:

        void init(const undicht::vulkan::LogicalDevice& device);
        void cleanUp();

        /// @brief add a point light to the buffer
        /// @return an unique id with which the light can be accessed
        uint32_t addPointLight(const PointLight& light);
        void updatePointLight(uint32_t id, const PointLight& light);
        void freePointLight(uint32_t id);
        
        uint32_t getPointLightModelVertexCount() const;
        uint32_t getPointLightCount() const;
        const undicht::vulkan::VertexBuffer& getPointLightBuffer() const;

    };

} // cell

#endif // LIGHT_BUFFER_H