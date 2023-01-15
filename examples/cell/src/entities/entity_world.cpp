#include "entity_world.h"

namespace cell {

    ///////////////////////////////////////// loading chunks into ram /////////////////////////////////////////

    const EntityChunk &EntityWorld::loadChunk(const glm::ivec3 &chunk_pos, const std::vector<Entity> &entities) {
        // returns the loaded chunk

        // checking if the chunk is already loaded
        EntityChunk* c = (EntityChunk*)getChunkAt(chunk_pos);
        if(c != nullptr) {
            c->initFromData(entities);
            return *c;
        }

        _loaded_chunks.emplace_back(EntityChunk());
        _loaded_chunks.back().initFromData(entities);
        _chunk_positions.push_back(chunk_pos);

        return _loaded_chunks.back();
    }

    void EntityWorld::unloadChunk(const glm::ivec3 &chunk_pos) {
        // removes the chunk from the vector of loaded chunks

        for(int i = 0; i < _chunk_positions.size(); i++) {

            if(_chunk_positions.at(i) == chunk_pos) {

                _loaded_chunks.at(i).initFromData({});
                break;
            }

        }

    }

    /////////////////////////////////////////////// accessing chunks //////////////////////////////////////////

    const EntityChunk *EntityWorld::getChunkAt(const glm::ivec3 &world_pos) const {

        // calculating the chunk-position of the requested world-position
        glm::ivec3 chunk_pos = calcChunkPosition(world_pos);

        for (int i = 0; i < _chunk_positions.size(); i++) {

            if (_chunk_positions.at(i) == chunk_pos) {

                return &_loaded_chunks.at(i);
            }
        }

        return nullptr;
    }

    uint32_t  EntityWorld::getNumberOfLoadedChunks() const {

        return _loaded_chunks.size();
    }

    ///////////////////////////////////////// protected world functions /////////////////////////////////////////

    glm::ivec3 EntityWorld::calcChunkPosition(const glm::ivec3& world_pos) const{
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