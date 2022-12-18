#ifndef MINI_CHUNK_H
#define MINI_CHUNK_H

#include "cstdint"
#include "vector"
#include "world/cell.h"

namespace cell {

    class MiniChunk {
        // cells are stored within a chunk with a size of 256 * 256 * 256
        // which means there can be quite a lot of cells within a chunk
        // to edit a specific location within the chunk, 
        // the cell covering that location has to be found (which could take a while)
        // MiniChunks are 16*16*16 units in size
        // they store references to the cells that are present within the volume of the mini chunk
        // to make it faster to find cells at specific locations

      protected:
        
        // the position of this mini chunk
        uint32_t _x;
        uint32_t _y;
        uint32_t _z;

        // referenzes to the cells present in the volume of the mini chunk
        std::vector<uint32_t> _cell_refs;

      public:

        MiniChunk() = default;
        MiniChunk(uint32_t x, uint32_t y, uint32_t z);

        // adding / removing cell references
        // will first check if the cell is within the volume of the mini chunk
        void addCellRef(const Cell& c, uint32_t id);
        void updCellRef(const Cell& c, uint32_t id);
        void remCellRef(const Cell& c, uint32_t id);

        // getting references to the cells within the mini chunk
        const std::vector<uint32_t>& getCellRefs() const;

        // @return wether the Cell overlaps with the volume of the mini chunk
        bool withinVolume(const Cell& c) const;

    };

} // namespace cell

#endif // MINI_CHUNK_H