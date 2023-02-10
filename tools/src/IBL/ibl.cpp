#include "IBL/ibl.h"
#include "glm/glm.hpp"
#include "debug.h"

namespace undicht {

    namespace tools {

        // defining the directions on every cube map face
        using glm::vec3; // right, left, up, down, back, front
        const std::array<vec3, 6> face_dirs   =   {vec3(1,0,0), vec3(-1,0,0), vec3(0,-1,0), vec3(0,1,0), vec3(0,0,1),  vec3(0,0,-1)};
        const std::array<vec3, 6> face_rights =   {vec3(0,0,-1), vec3(0,0,1), vec3(1,0,0), vec3(1,0,0),  vec3(1,0,0), vec3(-1,0,0)};
        const std::array<vec3, 6> face_ups    =   {vec3(0,-1,0), vec3(0,-1,0),  vec3(0,0,-1), vec3(0,0,1), vec3(0,-1,0),  vec3(0,-1,0)};

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
            glm::ivec2 pixel_coords = glm::ivec2(uv * glm::vec2(source_size));

            // make the texture repeat itself for uv coords outside [0,1]
            pixel_coords = pixel_coords % source_size;

            return &source._pixels.at((pixel_coords.y * source._width + pixel_coords.x) * source._nr_channels);
        }

        template<typename PIXEL_TYPE, typename IMAGE_TYPE>
        const PIXEL_TYPE* cubeMapLookup(const std::array<IMAGE_TYPE, 6>& source, const glm::vec3& dir) {
            /// @brief sample the cubemap for a given direction
            /// @param source the six faces of the cubemap (+x, -x, -y, +y, +z, -z)
            /// @param dir the direction in which to sample the cube-map
            /// @return a pointer to the pixel for the given direction

            // determine the face the dir is pointing at
            int face = -1;
            glm::vec3 abs_dir = glm::abs(dir); // absolute values of the components in the dir vector
            if((abs_dir.x >= abs_dir.y) && (abs_dir.x >= abs_dir.z) && (dir.x > 0)) face = 0; // +x
            if((abs_dir.x >= abs_dir.y) && (abs_dir.x >= abs_dir.z) && (dir.x < 0)) face = 1; // -x
            if((abs_dir.y >= abs_dir.x) && (abs_dir.y >= abs_dir.z) && (dir.y > 0)) face = 2; // +y
            if((abs_dir.y >= abs_dir.x) && (abs_dir.y >= abs_dir.z) && (dir.y < 0)) face = 3; // -y
            if((abs_dir.z >= abs_dir.x) && (abs_dir.z >= abs_dir.y) && (dir.z > 0)) face = 4; // +z
            if((abs_dir.z >= abs_dir.x) && (abs_dir.z >= abs_dir.y) && (dir.z < 0)) face = 5; // -z

            // making sure the dir vector "touches" the cube map face
            glm::vec3 point_on_face = dir / glm::dot(dir, face_dirs.at(face));

            // calculating the uv coords for the point_on_face
            float u = glm::dot(point_on_face, face_rights.at(face)) * 0.5f + 0.5f;
            float v = glm::dot(point_on_face, face_ups.at(face)) * 0.5f + 0.5f;

            // calculating the pixel coords
            int cube_map_size = source.at(face)._width; // should equal the height otherwise it would qualify as a "cube" in my opinion
            int x = int(u * cube_map_size) % cube_map_size; // modulo because im scared of float inaccuracies
            int y = int(v * cube_map_size) % cube_map_size;

            return &source.at(face)._pixels.at((y * cube_map_size + x) * source.at(face)._nr_channels);
        }

        template<typename PIXEL_TYPE, typename IMAGE_TYPE>
        void convertEquirectangularToCubeMap(const IMAGE_TYPE& source, std::array<IMAGE_TYPE, 6>& dst, uint32_t cube_map_size) {
            // template function, since there can be more than one image type with different pixel types
            /// @brief convert an equirectangular map to a cubemap (both can describe 360 degrees of an environment)
            /// @param source the equirectangular map
            /// @param dst the six faces of the cubemap
            /// @param cube_map_size the width and height of the cubemap faces

            const float texel_size = 1.0f / cube_map_size; // size of one "pixel" on the cubemap (in uv coords)

            

            for(int face = 0; face < 6; face++) { // six faces of the cubemap

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
                        glm::vec3 sample_dir = dir + ((x * texel_size) - 0.5f) * 2.0f * right + ((y * texel_size) - 0.5f) * 2.0f * up;
                        sample_dir = glm::normalize(sample_dir);

                        // store the pixel in the cubemap face
                        const PIXEL_TYPE* pixel = equirectangularLookup(source, sample_dir);
                        typename std::vector<PIXEL_TYPE>::iterator insert_pos = dst.at(face)._pixels.begin() + (y * cube_map_size + x) * source._nr_channels;

                        // i found this to be a lot faster than using insert()
                        for (int i = 0; i < source._nr_channels; i++)
                            *(insert_pos + i) = *(pixel + i);
                        
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

        const char* cubeMapLookup(const std::array<ImageData, 6>& source, const glm::vec3& dir) {
            
            return cubeMapLookup<char, ImageData>(source, dir);
        }

        const float* cubeMapLookup(const std::array<HDRImageData, 6>& source, const glm::vec3& dir) {

            return cubeMapLookup<float, HDRImageData>(source, dir);
        }

        void convertEquirectangularToCubeMap(const ImageData& source, std::array<ImageData, 6>& dst, uint32_t cube_map_size) {

            convertEquirectangularToCubeMap<char, ImageData>(source, dst, cube_map_size);
        }

        void convertEquirectangularToCubeMap(const HDRImageData& source, std::array<HDRImageData, 6>& dst, uint32_t cube_map_size) {

            convertEquirectangularToCubeMap<float, HDRImageData>(source, dst, cube_map_size);
        }

    } // tools

} // undicht