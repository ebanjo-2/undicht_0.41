#ifndef WORLD_EDIT_H
#define WORLD_EDIT_H

#include "chunk_edit.h"
#include "vector"
#include "world/cells/cell_world.h"

namespace cell {

    class WorldEdit {
      // edit the world without worrying about chunks

      protected:

        // to edit a single chunk at a time
        ChunkEdit _chunk_edit;

      public:

        /// @brief fills the volume defined by pos0 and pos1
        void place(CellWorld& world, const glm::ivec3& pos0, const glm::ivec3& pos1, uint32_t material);

        /// @brief removes all cells within the volume
        void remove(CellWorld& world, const glm::ivec3& pos0, const glm::ivec3& pos1);

      protected:
        // protected function of the WorldEdit class

        // calculates the part of the volume defined by pos0 and pos1
        // that is inside the chunk at the specified position
        Cell calcVolumeInChunk(const glm::ivec3& pos0, const glm::ivec3& pos1, const glm::ivec3& chunk_pos);

    };

} // cell

#endif // WORLD_EDIT_H