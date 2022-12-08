#include "world/world.h"
#include "algorithm"

namespace cell {

    ///////////////////////////////////////// loading chunks into ram /////////////////////////////////////////

    const Chunk &World::loadChunk(const glm::ivec3 &chunk_pos, const std::vector<Cell> &cells) {
        // returns the loaded chunk

        return loadChunk(chunk_pos, cells.data(), cells.size() * sizeof(Cell));
    }

    const Chunk &World::loadChunk(const glm::ivec3 &chunk_pos, const Cell *cell_buffer, uint32_t byte_size) {

        // checking if the chunk is already loaded
        Chunk* c = (Chunk*)getChunkAt(chunk_pos);
        if(c != nullptr) {
            c->initFromData(cell_buffer, byte_size);
            return *c;
        }

        _loaded_chunks.emplace_back(Chunk());
        _loaded_chunks.back().initFromData(cell_buffer, byte_size);
        _chunk_positions.push_back(chunk_pos);

        return _loaded_chunks.back();
    }

    void World::unloadChunk(const glm::ivec3 &chunk_pos) {
        // removes the chunk from the vector of loaded chunks

        for(int i = 0; i < _chunk_positions.size(); i++) {

            if(_chunk_positions.at(i) == chunk_pos) {

                _loaded_chunks.at(i).initFromData(nullptr, 0);
                break;
            }

        }

    }

    /////////////////////////////////////////////// accessing chunks //////////////////////////////////////////

    const Chunk *World::getChunkAt(const glm::ivec3 &world_pos) const {

        // calculating the chunk-position of the requested world-position
        glm::ivec3 chunk_pos = calcChunkPosition(world_pos);

        for (int i = 0; i < _chunk_positions.size(); i++) {

            if (_chunk_positions.at(i) == chunk_pos) {

                return &_loaded_chunks.at(i);
            }
        }

        return nullptr;
    }

    std::vector<const Chunk *> World::getChunksAt(const glm::ivec3 &world_pos0, const glm::ivec3 &world_pos1) const {
        // rerturns all chunks within the volume between pos0 and pos1 (unloaded chunks will be added as nullptr)

        std::vector<const Chunk *> chunks;

        for(int x = world_pos0.x; x <= world_pos1.x; x += 255) {
            for(int y = world_pos0.y; y <= world_pos1.y; y += 255) {
                for(int z = world_pos0.z; z <= world_pos1.z; z += 255) {
                    
                    // not loaded chunks will be added as nullptr
                    chunks.push_back(getChunkAt(glm::ivec3(x, y, z)));
                }            
            }
        }

        return chunks;
    }

    uint32_t World::getNumberOfLoadedChunks() const {

        return _loaded_chunks.size();
    }

    ///////////////////////////////////////// protected world functions /////////////////////////////////////////

    glm::ivec3 World::calcChunkPosition(const glm::ivec3& world_pos) const{
        // takes a position in the world and calculates which chunk it belongs to

        glm::ivec3 chunk_pos;
        
        if(world_pos.x >= 0)
            chunk_pos.x = (world_pos.x / 255) * 255;
        else
            chunk_pos.x = (world_pos.x / 255) * 255 - 255;

        if(world_pos.y >= 0)
            chunk_pos.y = (world_pos.y / 255) * 255;
        else
            chunk_pos.y = (world_pos.y / 255) * 255 - 255;

        if(world_pos.z >= 0)
            chunk_pos.z = (world_pos.z / 255) * 255;
        else
            chunk_pos.z = (world_pos.z / 255) * 255 - 255;

        return chunk_pos;
    }

} // namespace cell