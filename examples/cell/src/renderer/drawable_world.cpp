#include "renderer/drawable_world.h"
#include "debug.h"
#include "glm/gtx/string_cast.hpp"

namespace cell {

    void DrawableWorld::init(const undicht::vulkan::LogicalDevice& device) {

        _world_buffer.init(device);
    }

    void DrawableWorld::cleanUp() {

        _world_buffer.cleanUp();
    }

    // make sure the chunk is correctly stored in the world buffer
    void DrawableWorld::updateWorldBuffer(const glm::ivec3& chunk_pos) {

        const Chunk* chunk = getChunkAt(chunk_pos);

        if(chunk != nullptr) {

            _world_buffer.updateChunk(*chunk, chunk_pos);

        } else {
            UND_WARNING << "failed to update chunk at: " << glm::to_string(chunk_pos) << " because the chunk is not loaded \n";
            return;
        }

    }

    const WorldBuffer& DrawableWorld::getWorldBuffer() const {

        return _world_buffer;
    }


} // namespace cell