#include "world/lights/light_world.h"

namespace cell {

    void LightWorld::init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {

        _buffer.init(device, load_cmd, load_buf);
        _buffer.allocate(1000 * POINT_LIGHT_LAYOUT.getTotalSize());

    }
    
    void LightWorld::cleanUp() {

        _buffer.cleanUp();
    }

    void LightWorld::applyUpdates(undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {
        /** updates the vulkan buffer with the changes made since the last call to applyUpdates() 
        * @param load_cmd records the commands necessary to move the data from the transfer buffer to the internal vulkan buffer 
        * @param load_buf used as a staging buffer to transfer data to memory that is not directly visible to the cpu */

        // make sure each chunk is correctly stored in the buffer
        for(int i = 0; i < getLoadedChunks().size(); i++) {

            const LightChunk* chunk = (LightChunk*)getLoadedChunks().at(i);

            if(chunk->getHasChanged()) {
                // update the internal buffer
                
                const glm::ivec3& chunk_pos = getChunkPositions().at(i);
                _buffer.updateChunk(*chunk, chunk_pos, load_cmd, load_buf);
            }
        }

    }

    const LightBuffer& LightWorld::getBuffer() const {

        return _buffer;
    }

} // cell