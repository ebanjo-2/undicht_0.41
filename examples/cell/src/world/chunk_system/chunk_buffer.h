#ifndef CHUNK_BUFFER_H
#define CHUNK_BUFFER_H

#include "buffer_layout.h"
#include "renderer/vulkan/vertex_buffer.h"
#include "vector"
#include "glm/glm.hpp"
#include "world/chunk_system/chunk.h"

namespace cell {

    template<typename T>
    class ChunkBuffer {
        // stores that contents of multiple chunks in a vertex buffer

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

        undicht::vulkan::LogicalDevice _device_handle;
        undicht::vulkan::VertexBuffer _buffer;
        std::vector<BufferEntry> _buffer_sections;

      public:

        virtual void init(const undicht::vulkan::LogicalDevice& device);
        virtual void cleanUp();
        
        // setting the base model
        virtual void setBaseModel(const std::vector<float>& vertices);
        virtual void setBaseModel(const char* vertices, uint32_t byte_size);

        // storing data in the buffer
        virtual void addChunk(const Chunk<T>& c, const glm::ivec3& chunk_pos);
        virtual void updateChunk(const Chunk<T>& c, const glm::ivec3& chunk_pos);
        virtual void freeChunk(const Chunk<T>& c, const glm::ivec3& chunk_pos);

        // accessing the vertex buffer
        virtual const undicht::vulkan::VertexBuffer& getBuffer() const;

        // getting the areas of the buffer that contain data that should be drawn
        virtual const std::vector<BufferEntry>& getDrawAreas() const;

      protected:
        // protected ChunkBuffer functions

        virtual void sortBufferEntries();

        virtual BufferEntry* findBufferEntry(const glm::ivec3& chunk_pos);
        virtual BufferEntry findFreeMemory(uint32_t byte_size) const;

    };

} // cell

#endif // CHUNK_BUFFER_H