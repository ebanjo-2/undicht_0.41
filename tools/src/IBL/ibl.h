// ibl = image based lighting
#include "images/image_data.h"
#include "images/cube_map_data.h"
#include "glm/glm.hpp"
#include "array"


namespace undicht {

    namespace tools {

 /*       /// @brief sample the equirectangular map for a given direction
        /// @param source the equirectangular map
        /// @param dir the direction
        /// @return a pointer to the pixel for the given direction
        template<typename PIXEL_TYPE>
        PIXEL_TYPE* equirectangularLookup(ImageData<PIXEL_TYPE>& source, const glm::vec3& dir);

        /// @brief sample the cubemap for a given direction
        /// @param source the six faces of the cubemap (+x, -x, +y(down), -y(up), +z, -z)
        /// @param dir the direction in which to sample the cube-map
        /// @return a pointer to the pixel for the given direction
        template<typename PIXEL_TYPE>
        float* cubeMapLookup(std::array<ImageData<PIXEL_TYPE>, 6>& source, const glm::vec3& dir);

        /// @brief convert an equirectangular map to a cubemap (both can describe 360 degrees of an environment)
        /// @param source the equirectangular map
        /// @param dst the six faces of the cubemap (+x, -x, +y(down), -y(up), +z, -z)
        /// @param cube_map_size the width and height of the cubemap faces
        template<typename PIXEL_TYPE>
        void convertEquirectangularToCubeMap(ImageData<PIXEL_TYPE>& source, CubeMapData<PIXEL_TYPE>& dst, uint32_t cube_map_size);
*/
        /// @brief pre-compute the integral for determining the incoming irradiance for any surface direction (diffuse light)
        /// @param cube_map the environment which to convolute
        /// @param dst the cubemap in which to store the result
        /// @param dst_size the targeted size of the resulting cube map
        void convoluteCubeMap(const CubeMapData<float>& cube_map, CubeMapData<float>& dst, uint32_t dst_size);

        /// @brief precalculate the specular reflections resulting from the environment for varying roughness levels
        /// @param cube_map environment map
        /// @param dst cube map mip-levels that contain the result of the prefiltering, the mip map level 0 is the highest resolution (=dst_size) and contains the reflections for the lowest roughness
        /// @param dst_size size of mip-level 0
        /// @param mip_levels number of mip-levels to generate
        void prefilterSpecularReflections(const CubeMapData<float>& cube_map, std::vector<CubeMapData<float>>& dst, uint32_t dst_size, uint32_t mip_levels);

        /// @brief the brdf map contains a precalculated scale and a bias for combinations of a fresnel factor (the dot product of view and normal vector) and roughness(y-Axis)
        /// @param dst the 2D image to fill with the precalculated values (pixel format is vec2f)
        /// @param dst_size size of that image
        void createBRDFIntegrationMap(ImageData<float>& dst, uint32_t dst_size);

    } // tools

} // undicht