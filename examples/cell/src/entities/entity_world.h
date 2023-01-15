#ifndef ENTITY_WORLD_H
#define ENTITY_WORLD_H

#include "entity_chunk.h"
#include "glm/glm.hpp"

namespace cell {

    class EntityWorld {
      protected:

        std::vector<EntityChunk> _loaded_chunks;
        std::vector<glm::ivec3> _chunk_positions; // the positions of the loaded chunks

      public:
        
        // loading chunks into ram
        // returns the loaded chunk
        const EntityChunk& loadChunk(const glm::ivec3& chunk_pos, const std::vector<Entity>& entities);

        // removes the chunk from the vector of loaded chunks
        void unloadChunk(const glm::ivec3& chunk_pos);

        // accessing chunks
        const EntityChunk* getChunkAt(const glm::ivec3& world_pos) const;

        uint32_t getNumberOfLoadedChunks() const;

      protected:
        // protected world functions

        // takes a position in the world and calculates which chunk it belongs to
        glm::ivec3 calcChunkPosition(const glm::ivec3& world_pos) const;
    };

} // cell

#endif // ENTITY_WORLD_H