#ifndef CHUNK_SYSTEM_H
#define CHUNK_SYSTEM_H

#include "cstdint"
#include "world/chunk_system/chunk.h"
#include "glm/glm.hpp"

namespace cell {

    template<typename T>
    class ChunkSystem {
      // contains multiple chunks and places them in world space
      // manages the loading and unloading of chunks
      // chunk positions are mupltiples of 255
      protected:
        
        std::vector<Chunk<T>*> _loaded_chunks;
        std::vector<glm::ivec3> _chunk_positions;

      public:

        virtual ~ChunkSystem();
        
        /// @param chunk a chunk object that was dynamically allocated (new), will be deleted by the ChunkSystem class
        /// @return pointer to the chunk
        virtual Chunk<T>* loadChunk(const glm::ivec3& chunk_pos, Chunk<T>* chunk);

        // removes the chunk from the vector of loaded chunks
        virtual void unloadChunk(const glm::ivec3& chunk_pos);

        // accessing chunks
        
        /// @return nullptr if no chunk is loaded at that position
        virtual Chunk<T>* getChunkAt(const glm::ivec3& world_pos) const;

        /// @return all chunks within the volume between pos0 and pos1 (unloaded chunks will be added as nullptr)
        virtual std::vector<Chunk<T>*> getChunksAt(const glm::ivec3& world_pos0, const glm::ivec3& world_pos1) const;

        virtual const std::vector<Chunk<T>*>& getLoadedChunks() const;
        virtual const std::vector<glm::ivec3>& getChunkPositions() const;
        virtual uint32_t getNumberOfLoadedChunks() const;

      protected:
        // protected functions

        // takes a position in the world and calculates which chunk it belongs to
        glm::ivec3 calcChunkPosition(const glm::ivec3& world_pos) const;

    };

} // cell

#endif // CHUNK_SYSTEM_H