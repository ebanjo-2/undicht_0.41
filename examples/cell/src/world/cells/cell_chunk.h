#ifndef CELL_CHUNK_H
#define CELL_CHUNK_H

#include "world/chunk_system/chunk.h"
#include "world/cells/cell.h"
#include "world/cells/mini_chunk.h"
#include "vector"

namespace cell {

    class CellChunk : public Chunk<Cell> {
      // chunks are 255*255*255 units in size
      // (the biggest cell possible within a chunk goes from 0 to 255 (not covering the volume at 255))
      // a cell connecting to it would have to start at 255

      protected:

        std::vector<Cell> _cells; // all cells within this chunk
        std::vector<MiniChunk> _mini_chunks;
        
        // keeping track of which cells are no longer used and can be recycled
        std::vector<uint32_t> _unused_cells;

      public:

        CellChunk();

        // updating cells (adding, changing, removing)
        uint32_t addCell(const Cell& c); // returns an id with which the cell can be accessed
        void setCell(uint32_t id, const Cell& c);
        void removeCell(uint32_t id);

        // getting cells
        // when adding cells to the chunk, the internal vector containing the cells may resize
        // this will effect the const Cell* pointers, but not the ids (something to keep in mind)
        const Cell* getCell(uint32_t id) const; // returns nullptr if there is no cell with that id
        const Cell* getCell(uint32_t x, uint32_t y, uint32_t z) const;
        uint32_t getCellID(uint32_t x, uint32_t y, uint32_t z) const;

        uint32_t getCellCount() const;
        const std::vector<Cell>& getAllCells() const;

        std::vector<uint32_t> getCellIDsInVolume(const Cell& volume) const;
        std::vector<const Cell*> getCellsInVolume(const Cell& volume) const;

        uint32_t fillBuffer(char* buffer) const; // store the contents of the chunk in the buffer (if buffer != nullptr), return number of elements stored
        void loadFromBuffer(const char* buffer, uint32_t byte_size); // initialize the complete data of the chunk from the buffer
        void loadFromBuffer(const std::vector<Cell>& buffer);

      protected:
        // protected chunk functions

        void initMiniChunks();
        const MiniChunk* calcMiniChunk(uint32_t x, uint32_t y, uint32_t z) const;
        std::vector<const MiniChunk*> calcMiniChunks(const Cell& volume) const;

        bool withinVolume(const Cell& c, uint32_t x, uint32_t y, uint32_t z) const;
        bool sharedVolume(const Cell& c1, const Cell& c2) const;

    };

} // namespace cell

#endif // CELL_CHUNK_H