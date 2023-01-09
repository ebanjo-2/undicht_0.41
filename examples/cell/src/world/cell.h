#ifndef CELL_H
#define CELL_H
#include <cstdint>
#include "iostream"

#include "buffer_layout.h"

namespace cell {

    const extern undicht::BufferLayout CELL_LAYOUT;
    const extern uint8_t CELL_FACE_YP; // + y (00000001)
    const extern uint8_t CELL_FACE_YN; // - y (00000010)
    const extern uint8_t CELL_FACE_XP; // + x (00000100)
    const extern uint8_t CELL_FACE_XN; // - x (00001000)
    const extern uint8_t CELL_FACE_ZP; // + z (00010000)
    const extern uint8_t CELL_FACE_ZN; // - z (00100000)

    class Cell {

      protected:
        // first 3 bytes: position (x, y, z) (from 0 to 255)
        // last bytes (of both uint32s): position of tile on tile map (also identifying the type of cell)
        // at least that is how it can be accessed in the shader
        // if you are on a little endian system, the order of the position and uv will be swapped
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