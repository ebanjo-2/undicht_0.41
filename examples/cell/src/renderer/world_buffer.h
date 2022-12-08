#ifndef WORLD_BUFFER_H
#define WORLD_BUFFER_H

#include "renderer/vulkan/vertex_buffer.h"
#include "vector"
#include "glm/glm.hpp"
#include "world/chunk.h"
#include "core/vulkan/logical_device.h"

namespace cell {

    const extern undicht::BufferLayout CUBE_VERTEX_LAYOUT;

    class WorldBuffer {

      protected:

        undicht::vulkan::VertexBuffer _buffer;

        struct BufferEntry {
            size_t offset = 0;
            size_t byte_size = 0;
            glm::ivec3 _chunk_pos;
        };

        std::vector<BufferEntry> _buffer_sections;

      public:

        WorldBuffer(const undicht::vulkan::LogicalDevice& device);
        virtual ~WorldBuffer();
        
        // storing data in the buffer
        void addChunk(const Chunk& c);
        void updateChunk(const Chunk& c);
        void freeChunk(const Chunk& c);

        // accessing the vertex buffer
        const undicht::vulkan::VertexBuffer& getBuffer() const;


    };

} // namespace cell

#endif // WORLD_BUFFER_H