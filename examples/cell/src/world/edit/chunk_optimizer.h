#ifndef CHUNK_OPTIMIZER_H
#define CHUNK_OPTIMIZER_H

#include "world/cells/cell.h"
#include "world/cells/cell_chunk.h"
#include "array"
#include "glm/glm.hpp"

namespace cell {

    // taken from undicht 0.38
    class ChunkOptimizer {

        public:

            const unsigned short VOID_CELL = -1;

            // stores the material of every position within a chunk
            std::array<unsigned short, 255 * 255 * 255> _cell_mat_buffer;
            std::array<bool, 255 * 255 * 255> _cell_set_buffer;

        public:

            /** tries to minimize the number of cells in the chunk */
            void optimizeChunk(CellChunk old_chunk, CellChunk* optimized);

        protected:

            void loadChunk(const CellChunk& chunk);

            /** tries to find a cell with the biggest volume, starting at pos0 */
            Cell findCell(const glm::ivec3& pos0);

            unsigned char calcVisibleFaces(const Cell& c) ;

            void markCellAsSet(const Cell& c);

            /** @return true, if the volume is completely filled with that material */
            bool sameMaterial(const glm::ivec3& pos0, const glm::ivec3& pos1, unsigned short mat);

            bool containsVoid(const glm::ivec3& pos0, const glm::ivec3& pos1);


    };

    /** tries to minimize the number of cells in the chunk */
    void optimizeChunk(CellChunk old, CellChunk* optimized);

} // cell

#endif // CHUNK_OPTIMIZER_H