#ifndef CHUNK_H
#define CHUNK_H

#include "world/cell.h"
#include "world/mini_chunk.h"
#include "vector"

namespace cell {

    class Chunk {
      // chunks are 255*255*255 units in size
      // (the biggest cell possible within a chunk goes from 0 to 255 (not covering the volume at 255))
      // a cell connecting to it would have to start at 255

      protected:

        std::vector<Cell> _cells; // all cells within this chunk
        std::vector<MiniChunk> _mini_chunks;
        
        // keeping track of which cells are no longer used and can be recycled
        std::vector<uint32_t> _unused_cells;

        // keeping track of whether the chunk was edited (and needs to be updated in gpu memory)
        bool _has_changed = false;

      public:

        Chunk();

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

        // storing the complete chunk data in a buffer
        // will return the number of bytes that are going to be written
        // will only write the data if buffer is not nullptr
        // if buffer is not nullptr it has to have enough memory allocated
        // the ints of the cells are stored in little endian format,
        // which means the data in the buffer is structured like this:
        // v z0 y0 x0 u z1 y1 x1 (one byte each)
        uint32_t fillBuffer(const void* buffer) const;

        bool getWasEdited() const;
        void markAsUnEdited();

        // loading the chunk from existing data
        // will remove the current data in the chunks _cells buffer
        void initFromData(const Cell* buffer, uint32_t byte_size); // calling it with a byte_size of 0 or nullptr will effectivly clear the cells of the chunk
        void initFromData(const std::vector<Cell>& cells);

      protected:
        // protected chunk functions

        void initMiniChunks();
        const MiniChunk* calcMiniChunk(uint32_t x, uint32_t y, uint32_t z) const;
        std::vector<const MiniChunk*> calcMiniChunks(const Cell& volume) const;

        bool withinVolume(const Cell& c, uint32_t x, uint32_t y, uint32_t z) const;
        bool sharedVolume(const Cell& c1, const Cell& c2) const;

    };

} // namespace cell

#endif // CHUNK_H