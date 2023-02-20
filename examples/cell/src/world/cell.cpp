#include "cell.h"

// seems like this only gets defined on some systems / compilers
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif

namespace cell {

    using namespace undicht;
    const BufferLayout CELL_LAYOUT = BufferLayout({UND_VEC4UI8, UND_VEC4UI8, UND_UINT32});

    const uint8_t CELL_FACE_YP = 0x01; // + y (00000001)
    const uint8_t CELL_FACE_YN = 0x02; // - y (00000010)
    const uint8_t CELL_FACE_XP = 0x04; // + x (00000100)
    const uint8_t CELL_FACE_XN = 0x08; // - x (00001000)
    const uint8_t CELL_FACE_ZP = 0x10; // + z (00010000)
    const uint8_t CELL_FACE_ZN = 0x20; // - z (00100000)

    Cell::Cell(uint32_t x0, uint32_t y0, uint32_t z0, uint32_t x1, uint32_t y1, uint32_t z1, uint32_t id, uint32_t faces) {

        setPos0(x0, y0, z0);
        setPos1(x1, y1, z1);
        setID(id);
        setVisibleFaces(faces);
    }

    void Cell::getPos0(uint32_t &x, uint32_t &y, uint32_t &z) const {
        #ifdef LITTLE_ENDIAN
            x = (_pos_0 & 0x000000FF) >> 0;
            y = (_pos_0 & 0x0000FF00) >> 8;
            z = (_pos_0 & 0x00FF0000) >> 16;
        #else
            x = (_pos_0 & 0xFF000000) >> 24;
            y = (_pos_0 & 0x00FF0000) >> 16;
            z = (_pos_0 & 0x0000FF00) >> 8;
        #endif
    }

    void Cell::getPos1(uint32_t &x, uint32_t &y, uint32_t &z) const {
        #ifdef LITTLE_ENDIAN 
            x = (_pos_1 & 0x000000FF) >> 0;
            y = (_pos_1 & 0x0000FF00) >> 8;
            z = (_pos_1 & 0x00FF0000) >> 16;
        #else
            x = (_pos_1 & 0xFF000000) >> 24;
            y = (_pos_1 & 0x00FF0000) >> 16;
            z = (_pos_1 & 0x0000FF00) >> 8;
        #endif
    }

    void Cell::getUV(uint32_t &u, uint32_t &v) const {
        #ifdef LITTLE_ENDIAN 
            u = _pos_0 & 0xFF000000;
            v = _pos_1 & 0xFF000000;
        #else
            u = _pos_0 & 0x000000FF;
            v = _pos_1 & 0x000000FF;
        #endif
    }

    uint32_t Cell::getID() const {
        // combining the uv parts of pos_0 and pos_1 to form an id that is unique for the type of cell
        #ifdef LITTLE_ENDIAN 
            return ((_pos_0 & 0xFF000000) >> 16) | ((_pos_1 & 0xFF000000) >> 24);
        #else
            return ((_pos_0 & 0x000000FF) << 8) | (_pos_1 & 0x000000FF);
        #endif
    }

    uint32_t Cell::getVisibleFaces() const {
        return _faces;
    }

    void Cell::setPos0(uint32_t x, uint32_t y, uint32_t z) {
        #ifdef LITTLE_ENDIAN
            _pos_0 &= 0xFF000000;
            _pos_0 |= (x << 0);
            _pos_0 |= (y << 8);
            _pos_0 |= (z << 16);
        #else
            _pos_0 &= 0x000000FF;
            _pos_0 |= (x << 24);
            _pos_0 |= (y << 16);
            _pos_0 |= (z << 8);
        #endif
    }

    void Cell::setPos1(uint32_t x, uint32_t y, uint32_t z) {
        #ifdef LITTLE_ENDIAN
            _pos_1 &= 0xFF000000;
            _pos_1 |= (x << 0);
            _pos_1 |= (y << 8);
            _pos_1 |= (z << 16);
        #else
            _pos_1 &= 0x000000FF;
            _pos_1 |= (x << 24);
            _pos_1 |= (y << 16);
            _pos_1 |= (z << 8);
        #endif
    }

    void Cell::setUV(uint32_t u, uint32_t v) {
        #ifdef LITTLE_ENDIAN
            _pos_0 &= 0x00FFFFFF;
            _pos_0 |= (u << 24);
            _pos_1 &= 0x00FFFFFF;
            _pos_1 |= (v << 24);
        #else
            _pos_0 &= 0xFFFFFF00;
            _pos_0 |= u;
            _pos_1 &= 0xFFFFFF00;
            _pos_1 |= v;
        #endif
    }

    void Cell::setID(uint32_t id) {
        // storing the id as the uv in the last bytes of pos_0 and pos_1
        #ifdef LITTLE_ENDIAN
            setUV((id & 0x000000FF), ((id & 0x0000FF00) >> 8));
        #else
            setUV(((id & 0x0000FF00) >> 8), (id & 0x000000FF));
        #endif
    }

    void Cell::setVisibleFaces(uint32_t faces) {
        // combine the faces with bit wise ors: CELL_FACE_YP | CELL_FACE_YN | CELL_FACE_XP ...
        _faces = faces;
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