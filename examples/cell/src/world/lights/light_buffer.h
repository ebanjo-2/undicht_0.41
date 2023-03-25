#ifndef LIGHT_BUFFER_H
#define LIGHT_BUFFER_H

#include "world/lights/light.h"
#include "world/lights/light_chunk.h"
#include "world/chunk_system/chunk_buffer.h"

namespace cell {

    const extern undicht::BufferLayout LIGHT_VERTEX_LAYOUT; // per vertex data

    class LightBuffer : public ChunkBuffer<Light> {
      
      public:

        void init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);

        uint32_t getPointLightModelVertexCount() const;
    };

} // cell

#endif // LIGHT_BUFFER_H