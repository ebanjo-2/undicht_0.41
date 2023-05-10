#include "cell_math.h"

namespace cell {

    glm::ivec3 calcFaceDir(uint8_t face) {

        if (face == CELL_FACE_XP)
            return glm::ivec3(1, 0, 0);
        if (face == CELL_FACE_XN)
            return glm::ivec3(-1, 0, 0);
        if (face == CELL_FACE_YP)
            return glm::ivec3(0, 1, 0);
        if (face == CELL_FACE_YN)
            return glm::ivec3(0, -1, 0);
        if (face == CELL_FACE_ZP)
            return glm::ivec3(0, 0, 1);
        if (face == CELL_FACE_ZN)
            return glm::ivec3(0, 0, -1);

        return glm::ivec3(0, 0, 0);
    }

    glm::ivec3 toCellPos(const glm::vec3 &pos) {
        /** const converts the float vector into an integer vector,
         * representing the position of a cell in which the pos is located
         * simply casting wouldnt work for negative components of pos, since int(-0.5f) is 0, not -1 */

        glm::ivec3 cell_pos = glm::ivec3(pos);

        if(pos.x < 0.0f) cell_pos.x--;
        if(pos.y < 0.0f) cell_pos.y--;
        if(pos.z < 0.0f) cell_pos.z--;

        return cell_pos;
    }

} // namespace cell