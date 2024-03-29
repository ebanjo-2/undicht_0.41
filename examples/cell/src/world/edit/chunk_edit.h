#ifndef CHUNK_EDIT_H
#define CHUNK_EDIT_H

#include "world/cells/cell_chunk.h"

namespace cell {

    class ChunkEdit {
        /** a class that can edit a single chunk at a time */

      public:

        // place / remove cells from the chunk
        void add(CellChunk& chunk, const Cell& volume);
        void subtract(CellChunk& chunk, const Cell& volume);

      protected:
        // protected functions for ChunkEdit

        // edits the cell at the cell_id so that it doesnt overlap with c
        // the cell behind cell_id will be split into a maximum of 6 new cells
        void subtractFromCell(CellChunk& chunk, int cell_id, const Cell& c);

        /// calculates the volume shared by both cell
        /// @return false, if there is no shared volume
        bool overlappingVolume(const Cell& c0, const Cell& c1, Cell& volume);

    };

} // cell

#endif // CHUNK_EDIT_H