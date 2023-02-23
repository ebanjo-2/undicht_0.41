#include "world/cells/cell_chunk.h"
#include "iostream"
#include "math/math_tools.h"
#include "debug.h"

using namespace undicht::tools;

namespace cell {

    CellChunk::CellChunk() {

        initMiniChunks();
    }

    ////////////////////////////// updating cells (adding, changing, removing) //////////////////////////////

    uint32_t CellChunk::addCell(const Cell &c) {
        // returns an id with which the cell can be accessed

        uint32_t cell_id;

        if (_unused_cells.size()) { // trying to recycle an unused cell
            _cells.at(_unused_cells.back()) = c;
            cell_id = _unused_cells.back();
            _unused_cells.pop_back();
        } else { // adding the cell at the back of the cell vector
            _cells.push_back(c);
            cell_id = _cells.size() - 1;
        }

        // updating the mini chunks
        for (MiniChunk &mc : _mini_chunks)
            mc.addCellRef(c, cell_id);

        _has_changed = true;

        return cell_id;
    }

    void CellChunk::setCell(uint32_t id, const Cell &c) {

        if (id >= _cells.size())
            return;

        _cells.at(id) = c;

        // updating the mini chunks
        for (MiniChunk &mc : _mini_chunks)
            mc.updCellRef(c, id);

        _has_changed = true;
    }

    void CellChunk::removeCell(uint32_t id) {

        if (id >= _cells.size())
            return;

        // updating the mini chunks
        for (MiniChunk &mc : _mini_chunks)
            mc.remCellRef(_cells.at(id), id);

        // giving the cell up for recycling
        _unused_cells.push_back(id);
        _cells.at(id) = Cell(); // clearing the contents stored there

        _has_changed = true;
    }

    /////////////////////////////////////////// getting cells //////////////////////////////////////////////

    const Cell *CellChunk::getCell(uint32_t id) const {
        // returns nullptr if there is no cell with that id

        if (id >= _cells.size())
            return nullptr;

        return &_cells.at(id);
    }

    const Cell *CellChunk::getCell(uint32_t x, uint32_t y, uint32_t z) const {

        uint32_t id = getCellID(x, y, z);

        if (id >= _cells.size())
            return nullptr;

        return &_cells.at(id);
    }

    uint32_t CellChunk::getCellID(uint32_t x, uint32_t y, uint32_t z) const {

        const MiniChunk *mini_chunk = calcMiniChunk(x, y, z);

        if (!mini_chunk)
            return -1;

        for (uint32_t cell_id : mini_chunk->getCellRefs()) {

            const Cell *c = getCell(cell_id);

            if (c && withinVolume(*c, x, y, z))
                return cell_id;
        }

        return -1;
    }

    uint32_t CellChunk::getCellCount() const {

        return _cells.size();
    }

    const std::vector<Cell> &CellChunk::getAllCells() const {

        return _cells;
    }

    std::vector<uint32_t> CellChunk::getCellIDsInVolume(const Cell &volume) const {
        /// @return the ids of all cells within that volume

        std::vector<uint32_t> ids;
        std::vector<const MiniChunk *> mini_chunks = calcMiniChunks(volume);

        for (const MiniChunk *mc : mini_chunks) {

            for (uint32_t id : mc->getCellRefs()) {

                const Cell *cell = getCell(id);

                if (cell && sharedVolume(*cell, volume))
                    ids.push_back(id);
            }
        }

        return ids;
    }

    std::vector<const Cell *> CellChunk::getCellsInVolume(const Cell &volume) const {
        /// @return all cells within the volume

        std::vector<const Cell *> cells;
        std::vector<const MiniChunk *> mini_chunks = calcMiniChunks(volume);

        for (const MiniChunk *mc : mini_chunks) {

            for (uint32_t id : mc->getCellRefs()) {

                const Cell *cell = getCell(id);

                if (cell && sharedVolume(*cell, volume))
                    cells.push_back(cell);
            }
        }

        return cells;
    }

    uint32_t CellChunk::fillBuffer(char *buffer) const {

        if (buffer && _cells.size())
            std::copy(_cells.begin(), _cells.end(), (Cell*)buffer);

        // the size of the chunk data
        return _cells.size() * sizeof(Cell);
    }

    void CellChunk::loadFromBuffer(const char* buffer, uint32_t byte_size) {
        // initializes the complete chunk from the cell data stored in the buffer

        _cells.clear();

        if(buffer != nullptr)
            _cells.insert(_cells.begin(), (Cell*)buffer, (Cell*)(buffer + byte_size));

        initMiniChunks();
    }

    void CellChunk::loadFromBuffer(const std::vector<Cell>& buffer) {
        // initializes the complete chunk from the cell data stored in the buffer

        loadFromBuffer((char*)buffer.data(), sizeof(Cell) * buffer.size());
    }

    /*bool Chunk::getWasEdited() const {
        
        return _has_changed;
    }

    void Chunk::markAsUnEdited() {

        _has_changed = false;
    }*/

    ///////////////////////////// loading the chunk from existing data ///////////////////////////////////
    // will remove the current data in the chunks _cells buffer

    /*void Chunk::initFromData(const Cell* buffer, uint32_t byte_size) {
        // calling it with a byte_size of 0 or nullptr will effectivly clear the cells of the chunk
        
        _cells.clear();

        if(buffer != nullptr)
            _cells.insert(_cells.begin(), (Cell*)buffer, (Cell*)(buffer + byte_size / CELL_LAYOUT.getTotalSize()));

        initMiniChunks();
    }

    void Chunk::initFromData(const std::vector<Cell>& cells) {

        initFromData(cells.data(), sizeof(Cell) * cells.size());
    }*/

    /////////////////////////////////// protected chunk functions ////////////////////////////////////////

    void CellChunk::initMiniChunks() {
        // create 16 * 16 * 16 mini chunks

        _mini_chunks.clear();

        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {

                    _mini_chunks.emplace_back(MiniChunk(x, y, z));
                }
            }
        }

        // filling the minichunks with the cell data
        for(int i = 0; i < _cells.size(); i++) {

            for(MiniChunk& mc : _mini_chunks) {

                mc.addCellRef(_cells.at(i), i);
            }
        }

    }

    const MiniChunk *CellChunk::calcMiniChunk(uint32_t x, uint32_t y, uint32_t z) const {
        /// @return the mini chunk containing that location

        x /= 16;
        y /= 16;
        z /= 16;

        uint32_t chunk_id = x * 256 + y * 16 + z;

        if (chunk_id < _mini_chunks.size())
            return &_mini_chunks.at(chunk_id);

        return nullptr;
    }

    std::vector<const MiniChunk *> CellChunk::calcMiniChunks(const Cell &volume) const {
        // @return all mini chunks within that volume

        std::vector<const MiniChunk *> mini_chunks;

        uint32_t x1, y1, z1, x2, y2, z2;
        volume.getPos0(x1, y1, z1);
        volume.getPos1(x2, y2, z2);

        for (uint32_t x = x1; x <= x2; x += 16) {
            for (uint32_t y = y1; y <= y2; y += 16) {
                for (uint32_t z = z1; z <= z2; z += 16) {

                    const MiniChunk *mc = calcMiniChunk(x, y, z);
                    if (mc)
                        mini_chunks.push_back(mc);
                }
            }
        }

        return mini_chunks;
    }

    bool CellChunk::withinVolume(const Cell &c, uint32_t x, uint32_t y, uint32_t z) const {

        uint32_t x1, y1, z1, x2, y2, z2;
        c.getPos0(x1, y1, z1);
        c.getPos1(x2, y2, z2);

        if (x < x1 || x >= x2)
            return false;
        if (y < y1 || y >= y2)
            return false;
        if (z < z1 || z >= z2)
            return false;

        return true;
    }

    bool CellChunk::sharedVolume(const Cell &c1, const Cell &c2) const {
        /// @return true, if the cells have some shared volume ("touching" doesnt count)

        uint32_t x11, y11, z11, x12, y12, z12;
        c1.getPos0(x11, y11, z11);
        c1.getPos1(x12, y12, z12);

        uint32_t x21, y21, z21, x22, y22, z22;
        c2.getPos0(x21, y21, z21);
        c2.getPos1(x22, y22, z22);

        /*if(!overlappingRanges<uint32_t>(x11, x12, x21, x22))
            return false;
        if(!overlappingRanges<uint32_t>(y11, y12, y21, y22))
            return false;
        if(!overlappingRanges<uint32_t>(z11, z12, z21, z22))
            return false;*/

        // can be "simplified" (made faster), since it is known that x11 < x12 and x21 < x22

        if (x11 == x12 || x21 == x22 || y11 == y12 || y21 == y22 || z11 == z12 || z21 == z22)
            return false; // one of the cells has no volume

        if (x11 < x21) {
            if (x12 <= x21)
                return false;
        } else if (x11 > x21)
            if (x11 >= x22)
                return false;

        if (y11 < y21) {
            if (y12 <= y21)
                return false;
        } else if (y11 > y21)
            if (y11 >= y22)
                return false;

        if (z11 < z21) {
            if (z12 <= z21)
                return false;
        } else if (z11 > z21)
            if (z11 >= z22)
                return false;

        return true;
    }

} // namespace cell