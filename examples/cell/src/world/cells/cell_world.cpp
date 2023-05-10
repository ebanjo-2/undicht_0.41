#include "world/cells/cell_world.h"
#include "debug.h"
#include "math/math_tools.h"
#include "math/ray_cast.h"
#include "math/cell_math.h"

namespace cell {

    using namespace undicht;
    using namespace tools;

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

    const Cell* CellWorld::rayCastCell(const glm::vec3& pos, const glm::vec3& dir, glm::ivec3& hit, uint8_t& face) const {
        /// @brief casts a ray until it hits a cell
        /// @param hit the position, at which a cell was hit
        /// @param dir should be normalized
        /// @return nullptr, if no cell was hit

        glm::vec3 sample_point = pos;
        glm::ivec3 chunk_pos = calcChunkPosition(toCellPos(sample_point));
        const CellChunk* chunk = (CellChunk*)getChunkAt(chunk_pos);
        glm::uvec3 local_hit;
        glm::vec3 local_sample_point = sample_point - glm::vec3(chunk_pos);

        while(chunk) {
            
            //UND_LOG << "sampling chunk at: " << chunk_pos.x << " ; " << chunk_pos.y << " ; " << chunk_pos.z << "\n";
            //UND_LOG << "sample point: " << sample_point.x << " ; " << sample_point.y << " ; " << sample_point.z << "\n";
            
            const Cell* cell = chunk->rayCastCell(local_sample_point, dir, local_hit, face);
            if(cell) {
                hit = chunk_pos + glm::ivec3(local_hit);
                return cell;
            }

            // moving the sample point until it intersects the next chunk
            sample_point = rayCastSamplePoint(sample_point, dir, face, glm::vec3(255.0f));
            chunk_pos = calcChunkPosition(toCellPos(sample_point + 0.5f * (glm::sign(dir) - 1.0f)));
            chunk = (CellChunk*)getChunkAt(chunk_pos);
            local_sample_point = sample_point - glm::vec3(chunk_pos);

        }

        return nullptr;
    }

} // namespace cell