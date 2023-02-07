// ibl = image based lighting
#include "images/image_file.h"
#include "glm/glm.hpp"
#include "array"


namespace undicht {

    namespace tools {

        /// @brief sample the equirectangular map for a given direction
        /// @param source the equirectangular map
        /// @param dir the direction
        /// @return a pointer to the pixel for the given direction
        const char* equirectangularLookup(const ImageData& source, const glm::vec3& dir);
        const float* equirectangularLookup(const HDRImageData& source, const glm::vec3& dir);

        /// @brief convert an equirectangular map to a cubemap (both can describe 360 degrees of an environment)
        /// @param source the equirectangular map
        /// @param dst the six faces of the cubemap (+x, -x, +y, -y, +z, -z)
        /// @param cube_map_size the width and height of the cubemap faces
        void convertEquirectangularToCubemap(const ImageData& source, std::array<ImageData, 6>& dst, uint32_t cube_map_size);
        void convertEquirectangularToCubemap(const HDRImageData& source, std::array<HDRImageData, 6>& dst, uint32_t cube_map_size);

    } // tools

} // undicht