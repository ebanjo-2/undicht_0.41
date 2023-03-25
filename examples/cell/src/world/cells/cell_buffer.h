#ifndef CELL_BUFFER_H
#define CELL_BUFFER_H

#include "world/cells/cell.h"
#include "world/cells/cell_chunk.h"
#include "world/chunk_system/chunk_buffer.h"

namespace cell {

    const extern undicht::BufferLayout CUBE_VERTEX_LAYOUT;

    class CellBuffer : public ChunkBuffer<Cell> {

      public:

        void init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);

        uint32_t getCellModelVertexCount() const;

    };

} // cell

#endif // CELL_BUFFER_H