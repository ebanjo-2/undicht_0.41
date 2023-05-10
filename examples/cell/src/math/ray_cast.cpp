#include "ray_cast.h"

namespace cell {

    glm::vec3 rayCastSamplePoint(const glm::vec3 &last_sample_point, const glm::vec3 &dir, uint8_t& face, const glm::vec3 &voxel_size) {
        /// @brief calculates the next sample point for doing a ray cast in voxels
        /// @param voxel_size tells the function what size of voxels youre casting through
        /// @param dir assumed to be normalized
        /// for a cell, the voxel_size would be (1,1,1), for a chunk (255,255,255)

        // normalize the voxels, as if they were 1*1*1 in size
        glm::vec3 sample_point = last_sample_point / voxel_size;

        // calculate the position of the current sample point relative to the voxel we're in
        glm::vec3 pos_in_voxel = sample_point - glm::floor(sample_point);

        // distance to cross the border with the next voxel in each direction (x, y and z, positive and negative)
        // if dir has a negative component, that same component will also be negative in dist_to_edge
        glm::vec3 dist_to_edge = ((glm::sign(dir) + 1.0f) * 0.5f) - pos_in_voxel;

        // steps to take in the direction to hit the edge of the voxel / the next voxel
        glm::vec3 steps = dist_to_edge / dir;

        // a step of 0 units will just end up in the same voxel again
        // which might cause an infinite loop
        if (steps.x < 0.00000001f)
            steps.x = 0.001f;
        if (steps.y < 0.00000001f)
            steps.y = 0.001f;
        if (steps.z < 0.00000001f)
            steps.z = 0.001f;

        // taking the smallest step to get to the next voxel
        // and to not skip a voxel
        if ((steps.x < steps.y) && (steps.x < steps.z)) { // the next voxel will be reached in x direction
            sample_point += steps.x * dir;
            face = dir.x < 0 ? CELL_FACE_XP : CELL_FACE_XN;
        } else if (steps.y < steps.z) { // the next voxel will be reached in y direction
            sample_point += steps.y * dir;
            face = dir.y < 0 ? CELL_FACE_YP : CELL_FACE_YN;
        } else { // the next voxel will be reached in z direction
            sample_point += steps.z * dir;
            face = dir.z < 0 ? CELL_FACE_ZP : CELL_FACE_ZN;
        }

        // undo the normalization
        return sample_point * voxel_size;
    }

} // namespace cell