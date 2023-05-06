#include "world/cells/cell_world.h"
#include "debug.h"

namespace cell {

    using namespace undicht;

    void CellWorld::init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {

        _buffer.init(device, load_cmd, load_buf);

        _buffer.allocate(10000000 * CELL_LAYOUT.getTotalSize()); // 10.000.000 = 10 million cells
    }
    
    void CellWorld::cleanUp() {

        _buffer.cleanUp();
    }

    void CellWorld::applyUpdates(undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {
        /** updates the vulkan buffer with the changes made since the last call to applyUpdates() 
        * @param load_cmd records the commands necessary to move the data from the transfer buffer to the internal vulkan buffer 
        * @param load_buf used as a staging buffer to transfer data to memory that is not directly visible to the cpu */

        // make sure each chunk is correctly stored in the world buffer
        for(int i = 0; i < getLoadedChunks().size(); i++) {

            const CellChunk* chunk = (CellChunk*)getLoadedChunks().at(i);

            if(chunk->getHasChanged()) {
                // update the internal buffer

                // UND_LOG << "updating the internal cell buffer\n";

                const glm::ivec3& chunk_pos = getChunkPositions().at(i);
                _buffer.updateChunk(*chunk, chunk_pos, load_cmd, load_buf);
            }
        }

    }

    const CellBuffer& CellWorld::getBuffer() const {

        return _buffer;
    }

} // namespace cell