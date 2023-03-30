#include "world/world_loader.h"
#include "debug.h"

namespace cell {

    void WorldLoader::init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {

        _world.init(device, load_cmd, load_buf);
        _env_gen.init();
    }

    void WorldLoader::cleanUp() {

        _env_gen.cleanUp();
        _world.cleanUp();
    }

    bool WorldLoader::openWorldFile(const std::string& world_file) {

        return _world_file.open(world_file);
    }

    void WorldLoader::updateMaterials(undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {

        _world_file.readMaterials(_world.getMaterialAtlas(), load_cmd, load_buf);
    }

    void WorldLoader::updateEnvironment(undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {
        /** @brief updates the environment from an environment map referenced in the world file
        * or generates a new environment */

        glm::vec3 sun_dir = glm::vec3(0.5f,1.0,0.5f);

        _world.setSunDirection(sun_dir);
        _world.setSunColor(glm::vec3(23.47, 21.31, 20.79));
        _world_file.readEnvironment(_world.getEnvironment(), load_cmd, load_buf);
    }

    void WorldLoader::loadChunks(const glm::vec3& player_pos, int32_t chunk_distance, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {
        /** @brief load the missing chunks around the player */
        
        // calculating the chunk positions of the chunks that should be loaded
        std::vector<glm::ivec3> chunk_positions;
        glm::ivec3 player_chunk = ChunkSystem<Cell>::calcChunkPosition(glm::ivec3(player_pos));
        for(int x = -chunk_distance; x <= chunk_distance; x++) {
            for(int y = -chunk_distance; y <= chunk_distance; y++) {
                for(int z = -chunk_distance; z <= chunk_distance; z++) {
                    
                    glm::ivec3 chunk_pos = player_chunk + glm::ivec3(x * 255, y * 255, z * 255);
                    chunk_positions.push_back(chunk_pos);
                }
            }
        }

        // loading currently unloaded chunks
        bool update_cell_buffer = false;
        bool update_light_buffer = false;
        for(glm::ivec3& chunk_pos : chunk_positions) {
            
            // cell chunk
            if(!_world.getCellWorld().getChunkAt(chunk_pos)) {

                update_cell_buffer = true;
                CellChunk* new_chunk = new CellChunk; // gets deleted by the cell world

                if(_world_file.read(*new_chunk, chunk_pos)) // readin the chunk from the file
                    _world.getCellWorld().loadChunk(chunk_pos, new_chunk);
                else
                    ; // generate new chunk 

                UND_LOG << "loaded chunk at: " << chunk_pos.x << " : " << chunk_pos.y << " : " << chunk_pos.z << "\n";
            }

            // light chunk
            if(!_world.getLightWorld().getChunkAt(chunk_pos)) {

                update_light_buffer = true;
                LightChunk* new_chunk = new LightChunk; // gets deleted by the light world

                if(_world_file.read(*new_chunk, chunk_pos)) // readin the chunk from the file
                    _world.getLightWorld().loadChunk(chunk_pos, new_chunk);
                else
                    ; // generate new chunk 
            }

        }

        // update the vertex buffers
        if(update_cell_buffer) _world.updateWorldBuffer(load_cmd, load_buf);
        if(update_light_buffer) _world.updateLightBuffer(load_cmd, load_buf);

    }

    DrawableWorld& WorldLoader::getWorld() {
        
        return _world;
    }

} // cell