#ifndef CELL_WORLD_H
#define CELL_WORLD_H

#include "vector"
#include "cell.h"
#include "world/chunk_system/chunk_system.h"
#include "world/cells/cell_chunk.h"
#include "glm/glm.hpp"

namespace cell {

    class CellWorld : public ChunkSystem<Cell> {

    };

} // namespace cell

#endif // CELL_WORLD_H