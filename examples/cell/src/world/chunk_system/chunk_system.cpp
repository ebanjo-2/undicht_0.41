#include "world/chunk_system/chunk_system.h"
#include "cstdint"

namespace cell {

    // to avoid linker errors
    // Tell the C++ compiler which instantiations to make while it is compiling the template class’s .cpp file.
    // https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl
    class Cell;
    class Light;
    template class ChunkSystem<Cell>;
    template class ChunkSystem<Light>;

    template<typename T>
    ChunkSystem<T>::~ChunkSystem() {

        for(Chunk<T>*& c : _loaded_chunks)
            delete c;

    }

    template<typename T>
    Chunk<T>* ChunkSystem<T>::loadChunk(const glm::ivec3& chunk_pos, Chunk<T>* chunk) {
        /// @param chunk a chunk object that was dynamically allocated (new), will be deleted by the ChunkSystem class
        /// @return reference to the chunk

        // checking if the chunk is already loaded
        unloadChunk(chunk_pos);

        _loaded_chunks.push_back(chunk);
        _chunk_positions.push_back(chunk_pos);

        return _loaded_chunks.back();
    }

    template<typename T>
    void ChunkSystem<T>::unloadChunk(const glm::ivec3& chunk_pos) {
        // removes the chunk from the vector of loaded chunks

        for(int i = 0; i < _chunk_positions.size(); i++) {

            if(_chunk_positions.at(i) == chunk_pos) {
                _chunk_positions.erase(_chunk_positions.begin() + i);
                _loaded_chunks.erase(_loaded_chunks.begin() + i);
                break;
            }
        }

    }

    ////////////////////////////////// accessing chunks //////////////////////////////////
        
    template<typename T>
    Chunk<T>* ChunkSystem<T>::getChunkAt(const glm::ivec3& world_pos) const {
        /// @return nullptr if no chunk is loaded at that position

        // calculating the chunk-position of the requested world-position
        glm::ivec3 chunk_pos = calcChunkPosition(world_pos);

        for (int i = 0; i < _chunk_positions.size(); i++) {

            if (_chunk_positions.at(i) == chunk_pos) {

                return _loaded_chunks.at(i);
            }
        }

        return nullptr;
    }

    template<typename T>
    std::vector<Chunk<T>*> ChunkSystem<T>::getChunksAt(const glm::ivec3& world_pos0, const glm::ivec3& world_pos1) const {
        /// @return all chunks within the volume between pos0 and pos1 (unloaded chunks will be added as nullptr)

        std::vector<glm::ivec3> positions = getChunkPositionsAt(world_pos0, world_pos1);
        std::vector<Chunk<T>*> chunks;

        for(glm::ivec3& pos : positions)
            chunks.push_back(getChunkAt(pos)); // not loaded chunks will be added as nullptr

        return chunks;
    }

    template<typename T>
    std::vector<glm::ivec3> ChunkSystem<T>::getChunkPositionsAt(const glm::ivec3& world_pos0, const glm::ivec3& world_pos1) const {

        std::vector<glm::ivec3> positions;

        for(int x = world_pos0.x; x <= world_pos1.x; x += glm::max(1, glm::min(world_pos1.x - x, 255))) {
            for(int y = world_pos0.y; y <= world_pos1.y; y+= glm::max(1, glm::min(world_pos1.y - y, 255))) {
                for(int z = world_pos0.z; z <= world_pos1.z; z += glm::max(1, glm::min(world_pos1.z - z, 255))) {
                    
                    positions.push_back(calcChunkPosition(glm::ivec3(x, y, z)));
                }            
            }
        }

        return positions;
    }

    template<typename T>
    const std::vector<Chunk<T>*>& ChunkSystem<T>::getLoadedChunks() const {

        return _loaded_chunks;
    }

    template<typename T>
    const std::vector<glm::ivec3>& ChunkSystem<T>::getChunkPositions() const {

        return _chunk_positions;
    }

    template<typename T>
    uint32_t ChunkSystem<T>::getNumberOfLoadedChunks() const {
        
        return _loaded_chunks.size();
    }

    ////////////////////////////////// static functions functions //////////////////////////////////

    template<typename T>
    glm::ivec3 ChunkSystem<T>::calcChunkPosition(const glm::ivec3& world_pos) {
        // takes a position in the world and calculates which chunk it belongs to
        glm::ivec3 chunk_pos;
        
        if(world_pos.x >= 0)
            chunk_pos.x = (world_pos.x / 255) * 255;
        else
            chunk_pos.x = ((world_pos.x + 1) / 255) * 255 - 255;

        if(world_pos.y >= 0)
            chunk_pos.y = (world_pos.y / 255) * 255;
        else
            chunk_pos.y = ((world_pos.y + 1) / 255) * 255 - 255;

        if(world_pos.z >= 0)
            chunk_pos.z = (world_pos.z / 255) * 255;
        else
            chunk_pos.z = ((world_pos.z + 1) / 255) * 255 - 255;

        return chunk_pos;
    }

} // cell