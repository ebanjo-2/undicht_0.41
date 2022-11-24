#include "cell.h"

namespace cell {

    using namespace undicht;
    const BufferLayout CELL_LAYOUT = BufferLayout({UND_INT32, UND_INT32});


    Cell::Cell(uint32_t x0, uint32_t y0, uint32_t z0, uint32_t x1, uint32_t y1, uint32_t z1, uint32_t id) {

        setPos0(x0, y0, z0);
        setPos1(x1, y1, z1);
        setID(id);
    }

    void Cell::getPos0(uint32_t &x, uint32_t &y, uint32_t &z) const {
        x = (_pos_0 & 0xFF000000) >> 24;
        y = (_pos_0 & 0x00FF0000) >> 16;
        z = (_pos_0 & 0x0000FF00) >> 8;
    }

    void Cell::getPos1(uint32_t &x, uint32_t &y, uint32_t &z) const {
        x = (_pos_1 & 0xFF000000) >> 24;
        y = (_pos_1 & 0x00FF0000) >> 16;
        z = (_pos_1 & 0x0000FF00) >> 8;
    }

    void Cell::getUV(uint32_t &u, uint32_t &v) const {
        u = _pos_0 & 0x000000FF;
        v = _pos_1 & 0x000000FF;
    }

    uint32_t Cell::getID() const {
        // combining the uv parts of pos_0 and pos_1 to form an id that is unique for the type of cell
        return ((_pos_0 & 0x000000FF) << 8) | (_pos_1 & 0x000000FF);
    }

    void Cell::setPos0(uint32_t x, uint32_t y, uint32_t z) {
        _pos_0 = 0x00000000;
        _pos_0 |= (x << 24);
        _pos_0 |= (y << 16);
        _pos_0 |= (z << 8);
    }

    void Cell::setPos1(uint32_t x, uint32_t y, uint32_t z) {
        _pos_1 = 0x00000000;
        _pos_1 |= (x << 24);
        _pos_1 |= (y << 16);
        _pos_1 |= (z << 8);
    }

    void Cell::setUV(uint32_t u, uint32_t v) {
        _pos_0 &= 0xFFFFFF00;
        _pos_0 |= u;
        _pos_1 &= 0xFFFFFF00;
        _pos_1 |= v;
    }

    void Cell::setID(uint32_t id) {
        // storing the id as the uv in the last bytes of pos_0 and pos_1
        setUV(((id & 0x0000FF00) >> 8), (id & 0x000000FF));
    }

    ///////////////////////////////////////////// operator to print out cell data /////////////////////////////////////////

    std::ostream& operator<< (std::ostream& out, const Cell& c) {

        uint32_t x1, y1, z1, x2, y2, z2;
        c.getPos0(x1, y1, z1);
        c.getPos1(x2, y2, z2);

        out << x1 << ":" << y1 << ":" << z1 << " / " << x2 << ":" << y2 << ":" << z2 << " id:" << c.getID();

        return out;
    }

} // namespace cell