#ifndef WORLD_H
#define WORLD_H

#include "vector"
#include "world/chunk.h"
#include "glm/glm.hpp"

namespace cell {

    class World {

      protected:

        std::vector<Chunk> _loaded_chunks;
        std::vector<glm::ivec3> _chunk_positions; // the positions of the loaded chunks

      public:
        
        // loading chunks into ram
        // returns the loaded chunk
        const Chunk& loadChunk(const glm::ivec3& chunk_pos, const std::vector<Cell>& cells);
        const Chunk& loadChunk(const glm::ivec3& chunk_pos, const Cell* cell_buffer, uint32_t byte_size);

        // removes the chunk from the vector of loaded chunks
        void unloadChunk(const glm::ivec3& chunk_pos);

        // accessing chunks
        const Chunk* getChunkAt(const glm::ivec3& world_pos) const;
        std::vector<const Chunk*> getChunksAt(const glm::ivec3& world_pos0, const glm::ivec3& world_pos1) const; // rerturns all chunks within the volume between pos0 and pos1 (unloaded chunks will be added as nullptr)

        uint32_t getNumberOfLoadedChunks() const;

      protected:
        // protected world functions

        // takes a position in the world and calculates which chunk it belongs to
        glm::ivec3 calcChunkPosition(const glm::ivec3& world_pos) const;

    };

} // namespace cell

#endif // WORLD_H