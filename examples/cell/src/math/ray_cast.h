#ifndef RAY_CAST_H
#define RAY_CAST_H

#include "glm/glm.hpp"
#include "cstdint"
#include "world/cells/cell.h"


namespace cell {

    /// @brief calculates the next sample point for doing a ray cast in voxels
    /// @param voxel_size tells the function what size of voxels youre casting through
    /// for a cell, the voxel_size would be (1,1,1), for a chunk (255,255,255)
    /// @param face the face of the voxel at which the ray arrived (i.e. CELL_FACE_XP)
    glm::vec3 rayCastSamplePoint(const glm::vec3& last_sample_point, const glm::vec3& dir, uint8_t& face, const glm::vec3& voxel_size);


}

#endif // RAY_CAST_H