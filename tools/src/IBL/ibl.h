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
        const char* equirectangularLookup(const ImageData& source, const glm::highp_vec3& dir);
        const float* equirectangularLookup(const HDRImageData& source, const glm::highp_vec3& dir);

        /// @brief sample the cubemap for a given direction
        /// @param source the six faces of the cubemap (+x, -x, +y(down), -y(up), +z, -z)
        /// @param dir the direction in which to sample the cube-map
        /// @return a pointer to the pixel for the given direction
        const char* cubeMapLookup(const std::array<ImageData, 6>& source, const glm::highp_vec3& dir);
        const float* cubeMapLookup(const std::array<HDRImageData, 6>& source, const glm::highp_vec3& dir);

        /// @brief convert an equirectangular map to a cubemap (both can describe 360 degrees of an environment)
        /// @param source the equirectangular map
        /// @param dst the six faces of the cubemap (+x, -x, +y(down), -y(up), +z, -z)
        /// @param cube_map_size the width and height of the cubemap faces
        void convertEquirectangularToCubeMap(const ImageData& source, std::array<ImageData, 6>& dst, uint32_t cube_map_size);
        void convertEquirectangularToCubeMap(const HDRImageData& source, std::array<HDRImageData, 6>& dst, uint32_t cube_map_size);

        /// @brief pre-compute the integral for determining the incoming irradiance for any surface direction (diffuse light)
        /// @param cube_map the environment which to convolute
        /// @param dst the cubemap in which to store the result
        /// @param dst_size the targeted size of the resulting cube map 
        void convoluteCubeMap(const std::array<ImageData, 6>& cube_map, std::array<ImageData, 6>& dst, uint32_t dst_size);
        void convoluteCubeMap(const std::array<HDRImageData, 6>& cube_map, std::array<HDRImageData, 6>& dst, uint32_t dst_size);

    } // tools

} // undicht