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

      public:

        struct BufferEntry {
            size_t offset = 0;
            size_t byte_size = 0;
            glm::ivec3 _chunk_pos;

            bool operator < (const BufferEntry& other) const {
              return offset < other.offset;
            }

        };

      protected:

        undicht::vulkan::VertexBuffer _buffer;
        std::vector<BufferEntry> _buffer_sections;

      public:

        void init(const undicht::vulkan::LogicalDevice& device);
        void cleanUp();
        
        // storing data in the buffer
        void addChunk(const Chunk& c, const glm::ivec3& chunk_pos);
        void updateChunk(const Chunk& c, const glm::ivec3& chunk_pos);
        void freeChunk(const Chunk& c, const glm::ivec3& chunk_pos);

        // accessing the vertex buffer
        const undicht::vulkan::VertexBuffer& getBuffer() const;

        // getting the areas of the buffer that contain data that should be drawn
        const std::vector<BufferEntry>& getDrawAreas() const;

      protected:
        // private WorldBuffer functions

        void sortBufferEntries();

        BufferEntry* findBufferEntry(const glm::ivec3& chunk_pos);
        BufferEntry findFreeMemory(uint32_t byte_size) const;

    };

} // namespace cell

#endif // WORLD_BUFFER_H