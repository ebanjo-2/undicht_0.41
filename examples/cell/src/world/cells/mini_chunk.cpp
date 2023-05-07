#include "mini_chunk.h"
#include "algorithm"
#include "iostream"

namespace cell {

    MiniChunk::MiniChunk(uint32_t x, uint32_t y, uint32_t z) : _x(x), _y(y), _z(z) {
    }

    glm::ivec3 MiniChunk::getPosition() const {

        return glm::ivec3(_x, _y, _z);
    }

    ///////////////////////////// adding / removing cell references ///////////////////////////

    void MiniChunk::addCellRef(const Cell &c, uint32_t id) {
        // will first check if the cell is within the volume of the mini chunk

        if (!withinVolume(c))
            return;

        _cell_refs.push_back(id);
    }

    void MiniChunk::updCellRef(const Cell &c, uint32_t id) {

        if (!withinVolume(c))
            return;

        std::vector<uint32_t>::iterator pos = std::find(_cell_refs.begin(), _cell_refs.end(), id);

        if (pos != _cell_refs.end())
            *pos = id;
    }

    void MiniChunk::remCellRef(const Cell &c, uint32_t id) {
        // will first check if the cell is within the volume of the mini chunk

        if (!withinVolume(c))
            return;

        std::vector<uint32_t>::iterator pos = std::find(_cell_refs.begin(), _cell_refs.end(), id);

        if (pos != _cell_refs.end())
            _cell_refs.erase(pos);
    }

    /////////////////////////// getting references to the cells within the mini chunk ///////////////////////////

    const std::vector<uint32_t> &MiniChunk::getCellRefs() const {

        return _cell_refs;
    }

    bool MiniChunk::withinVolume(const Cell &c) const {
        // @return wether the Cell overlaps with the volume of the mini chunk

        uint8_t x1, y1, z1, x2, y2, z2;
        c.getPos0(x1, y1, z1);
        c.getPos1(x2, y2, z2);

        if ((x1 >= _x + 16) || (x2 < _x))
            return false;
        if ((y1 >= _y + 16) || (y2 < _y))
            return false;
        if ((z1 >= _z + 16) || (z2 < _z))
            return false;

        return true;
    }

} // namespace cell