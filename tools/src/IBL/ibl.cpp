#include "IBL/ibl.h"
#include "glm/glm.hpp"
#include "debug.h"

namespace undicht {

    namespace tools {

        ////////////////////////////////////// universal implementation for all image types //////////////////////////////

        template<typename PIXEL_TYPE, typename IMAGE_TYPE>
        const PIXEL_TYPE* equirectangularLookup(const IMAGE_TYPE& source, const glm::vec3& dir) {
            // template function, since there can be more than one image type with different pixel types
            /// @brief sample the equirectangular map for a given direction
            /// @param source the equirectangular map
            /// @param dir the direction
            /// @return a pointer to the pixel for the given direction
            // math taken from https://learnopengl.com/PBR/IBL/Diffuse-irradiance
            const glm::vec2 invAtan = glm::vec2(0.1591, 0.3183);
            glm::vec2 uv = glm::vec2(glm::atan(dir.z, dir.x), glm::asin(dir.y));
            uv *= invAtan;
            uv += 0.5;

            // find the pixel at the calculated uv coord
            glm::ivec2 source_size = glm::ivec2(source._width, source._height);
            glm::ivec2 pixel_coords = glm::ivec2(uv) * source_size;

            // make the texture repeat itself for uv coords outside [0,1]
            pixel_coords = pixel_coords % source_size;

            return &source._pixels.at((pixel_coords.y * source._width + pixel_coords.x) * source._nr_channels);
        }

        template<typename PIXEL_TYPE, typename IMAGE_TYPE>
        void convertEquirectangularToCubemap(const IMAGE_TYPE& source, std::array<IMAGE_TYPE, 6>& dst, uint32_t cube_map_size) {
            // template function, since there can be more than one image type with different pixel types
            /// @brief convert an equirectangular map to a cubemap (both can describe 360 degrees of an environment)
            /// @param source the equirectangular map
            /// @param dst the six faces of the cubemap
            /// @param cube_map_size the width and height of the cubemap faces

            using glm::vec3; // (+x, -x, +y, -y, +z, -z)
            std::array<vec3, 6> face_dirs   =   {vec3(1,0,0), vec3(-1,0,0), vec3(0,1,0), vec3(0,-1,0), vec3(0,0,1),  vec3(0,0,-1)};
            std::array<vec3, 6> face_rights =   {vec3(0,0,1), vec3(0,0,-1), vec3(1,0,0), vec3(1,0,0),  vec3(-1,0,0), vec3(1,0,0)};
            std::array<vec3, 6> face_ups    =   {vec3(0,1,0), vec3(0,1,0),  vec3(0,0,1), vec3(0,0,-1), vec3(0,1,0),  vec3(0,1,0)};

            const float texel_size = 1.0f / cube_map_size; // size of one "pixel" on the cubemap (in uv coords)

            for(int face = 0; face < 6; face++) { // six faces of the cubemap

                UND_LOG << "processing face " << face << "\n";

                // resize the face
                dst.at(face)._width = cube_map_size;
                dst.at(face)._height = cube_map_size;
                dst.at(face)._nr_channels = source._nr_channels;
                dst.at(face)._pixels.resize(cube_map_size * cube_map_size * source._nr_channels);

                for(int x = 0; x < cube_map_size; x++) { // going through every pixel on the cube map face
                    for(int y = 0; y < cube_map_size; y++) {
                        
                        // calculate the direction in which the source map should be sampled
                        glm::vec3 dir = face_dirs.at(face);
                        glm::vec3 right = face_rights.at(face);
                        glm::vec3 up = face_ups.at(face);
                        glm::vec3 sample_dir = dir + ((x * texel_size) - 0.5f) * right + ((y * texel_size) - 0.5f) * up;
                        sample_dir = glm::normalize(sample_dir);

                        // store the pixel in the cubemap face
                        const PIXEL_TYPE* pixel = equirectangularLookup(source, sample_dir);
                        typename std::vector<PIXEL_TYPE>::iterator insert_pos = dst.at(face)._pixels.begin() + (y * cube_map_size + x) * source._nr_channels;
                        dst.at(face)._pixels.insert(insert_pos, pixel, pixel + source._nr_channels);
                    }
                }
            }
        }

        ////////////////////////////////////////// implementation for api functions declared in ibl.h //////////////////////////////////////////

        const char* equirectangularLookup(const ImageData& source, const glm::vec3& dir) {

            return equirectangularLookup<char, ImageData>(source, dir);
        }

        const float* equirectangularLookup(const HDRImageData& source, const glm::vec3& dir) {

            return equirectangularLookup<float, HDRImageData>(source, dir);
        }

        void convertEquirectangularToCubemap(const ImageData& source, std::array<ImageData, 6>& dst, uint32_t cube_map_size) {

            convertEquirectangularToCubemap<char, ImageData>(source, dst, cube_map_size);
        }

        void convertEquirectangularToCubemap(const HDRImageData& source, std::array<HDRImageData, 6>& dst, uint32_t cube_map_size) {

            convertEquirectangularToCubemap<float, HDRImageData>(source, dst, cube_map_size);
        }

    } // tools

} // undicht