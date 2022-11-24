#ifndef CELL_H
#define CELL_H
#include <cstdint>
#include "iostream"

#include "buffer_layout.h"

namespace cell {

    const extern undicht::BufferLayout CELL_LAYOUT;

    class Cell {

      protected:
        // first 3 bytes: position (x, y, z) (from 0 to 255)
        // last bytes (of both uint32s): position of tile on tile map (also identifying the type of cell)
        uint32_t _pos_0 = 0;
        uint32_t _pos_1 = 0;

      public:

        Cell() = default;
        Cell(uint32_t x0, uint32_t y0, uint32_t z0, uint32_t x1, uint32_t y1, uint32_t z1, uint32_t id = 0);

        // pos0 should contain lower values then pos1 (x0 <= x1 and y0 <= y1 and z0 <= z1)
        void getPos0(uint32_t &x, uint32_t &y, uint32_t &z) const;
        void getPos1(uint32_t &x, uint32_t &y, uint32_t &z) const;
        void getUV(uint32_t &u, uint32_t &v) const;
        uint32_t getID() const;

        void setPos0(uint32_t x, uint32_t y, uint32_t z);
        void setPos1(uint32_t x, uint32_t y, uint32_t z);
        void setUV(uint32_t u, uint32_t v);
        void setID(uint32_t id);

    };

    std::ostream& operator<< (std::ostream& out, const Cell& c);

} // namespace cell

#endif // CELL_H