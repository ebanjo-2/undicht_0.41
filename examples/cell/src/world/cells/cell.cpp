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

    void Cell::getPos0(uint8_t &x, uint8_t &y, uint8_t &z) const {
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

    void Cell::getPos1(uint8_t &x, uint8_t &y, uint8_t &z) const {
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

    void Cell::getUV(uint8_t &u, uint8_t &v) const {
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
            return ((_pos_0 & 0xFF000000) >> 24) | ((_pos_1 & 0xFF000000) >> 16);
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

    bool Cell::hasVolume() const{
        // returns false, if the cell has a volume of 0

        // xor, if some coords are the same this will result in a 0 for that coord
        uint32_t tmp = _pos_0 ^ _pos_1;

        // check if any of the coords were the same
        #ifdef LITTLE_ENDIAN
            return (tmp & 0x000000FF) && (tmp & 0x0000FF00) && (tmp & 0x00FF0000);
        #else
            return (tmp & 0x0000FF00) && (tmp & 0x00FF0000) && (tmp & 0xFF000000);
        #endif
    } 

    bool Cell::sharedVolume(const Cell &c1, const Cell &c2) {
        /// @return true, if the cells have some shared volume ("touching" doesnt count)

        uint8_t x11, y11, z11, x12, y12, z12;
        c1.getPos0(x11, y11, z11);
        c1.getPos1(x12, y12, z12);

        uint8_t x21, y21, z21, x22, y22, z22;
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

    ///////////////////////////////////////////// operator to print out cell data /////////////////////////////////////////

    std::ostream& operator<< (std::ostream& out, const Cell& c) {

        uint8_t x1, y1, z1, x2, y2, z2;
        c.getPos0(x1, y1, z1);
        c.getPos1(x2, y2, z2);

        out << int(x1) << ":" << int(y1) << ":" << int(z1) << " / " << int(x2) << ":" << int(y2) << ":" << int(z2) << " id:" << c.getID();

        return out;
    }

} // namespace cell