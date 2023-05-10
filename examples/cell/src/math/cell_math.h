#ifndef CELL_MATH_H
#define CELL_MATH_H

#include "world/cells/cell.h"
#include "glm/glm.hpp"

namespace cell {

    /** @brief calculates the normal vector for a cell face (i.e. Cell_FACE_XP) */
    glm::ivec3 calcFaceDir(uint8_t face);

    /** const converts the float vector into an integer vector, 
     * representing the position of a cell in which the pos is located
     * simply casting wouldnt work for negative components of pos, since int(-0.5f) is 0, not -1 */
    glm::ivec3 toCellPos(const glm::vec3& pos);

} // cell

#endif // CELL_MATH_H