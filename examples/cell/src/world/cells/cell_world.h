#ifndef CELL_WORLD_H
#define CELL_WORLD_H

#include "vector"
#include "cell.h"
#include "world/chunk_system/chunk_system.h"
#include "world/cells/cell_chunk.h"
#include "glm/glm.hpp"
#include "cell_buffer.h"

namespace cell {

    class CellWorld : public ChunkSystem<Cell> {

      protected:

        CellBuffer _buffer;
    
      public:

        void init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);
        void cleanUp();
        
        /** updates the vulkan buffer with the changes made since the last call to applyUpdates() 
         * @param load_cmd records the commands necessary to move the data from the transfer buffer to the internal vulkan buffer 
         * @param load_buf used as a staging buffer to transfer data to memory that is not directly visible to the cpu */
        void applyUpdates(undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);

        const CellBuffer& getBuffer() const;

        /// @brief casts a ray until it hits a cell
        /// @param hit the position, at which a cell was hit
        /// @return nullptr, if no cell was hit
        const Cell* rayCastCell(const glm::vec3& pos, const glm::vec3& dir, glm::ivec3& hit, uint8_t& face) const;

    };

} // namespace cell

#endif // CELL_WORLD_H