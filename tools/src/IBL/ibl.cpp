#include "IBL/ibl.h"
#include "glm/glm.hpp"
#include "debug.h"

namespace undicht {

    namespace tools {

        // defining the directions on every cube map face
        using glm::highp_vec3; // right, left, up, down, back, front
        const std::array<highp_vec3, 6> face_dirs   =   {highp_vec3(1,0,0), highp_vec3(-1,0,0), highp_vec3(0,1,0), highp_vec3(0,-1,0), highp_vec3(0,0,1),  highp_vec3(0,0,-1)};
        const std::array<highp_vec3, 6> face_rights =   {highp_vec3(0,0,-1), highp_vec3(0,0,1), highp_vec3(1,0,0), highp_vec3(1,0,0),  highp_vec3(1,0,0), highp_vec3(-1,0,0)};
        const std::array<highp_vec3, 6> face_ups    =   {highp_vec3(0,-1,0), highp_vec3(0,-1,0),  highp_vec3(0,0,1), highp_vec3(0,0,-1), highp_vec3(0,-1,0),  highp_vec3(0,-1,0)};

        ////////////////////////////////////// universal implementation for all image types //////////////////////////////

        template<typename PIXEL_TYPE, typename IMAGE_TYPE>
        const PIXEL_TYPE* equirectangularLookup(const IMAGE_TYPE& source, const glm::highp_vec3& dir) {
            // template function, since there can be more than one image type with different pixel types
            /// @brief sample the equirectangular map for a given direction
            /// @param source the equirectangular map
            /// @param dir the direction
            /// @return a pointer to the pixel for the given direction
            // math taken from https://learnopengl.com/PBR/IBL/Diffuse-irradiance
            const glm::highp_vec2 invAtan = glm::highp_vec2(0.1591, 0.3183);
            glm::highp_vec2 uv = glm::highp_vec2(glm::atan(dir.z, dir.x), glm::asin(dir.y));
            uv *= invAtan;
            uv += 0.5;

            // find the pixel at the calculated uv coord
            glm::ivec2 source_size = glm::ivec2(source._width, source._height);
            glm::ivec2 pixel_coords = glm::ivec2(uv * glm::highp_vec2(source_size));

            // make the texture repeat itself for uv coords outside [0,1]
            pixel_coords = pixel_coords % source_size;

            return &source._pixels.at((pixel_coords.y * source._width + pixel_coords.x) * source._nr_channels);
        }

        template<typename PIXEL_TYPE, typename IMAGE_TYPE>
        const PIXEL_TYPE* cubeMapLookup(const std::array<IMAGE_TYPE, 6>& source, const glm::highp_vec3& dir) {
            /// @brief sample the cubemap for a given direction
            /// @param source the six faces of the cubemap (+x, -x, +y(down), -y(up), +z, -z)
            /// @param dir the direction in which to sample the cube-map
            /// @return a pointer to the pixel for the given direction

            // determine the face the dir is pointing at
            int face = -1;
            glm::highp_vec3 abs_dir = glm::abs(dir); // absolute values of the components in the dir vector
            if((abs_dir.x >= abs_dir.y) && (abs_dir.x >= abs_dir.z) && (dir.x > 0)) face = 0; // +x
            if((abs_dir.x >= abs_dir.y) && (abs_dir.x >= abs_dir.z) && (dir.x < 0)) face = 1; // -x
            if((abs_dir.y >= abs_dir.x) && (abs_dir.y >= abs_dir.z) && (dir.y > 0)) face = 2; // +y
            if((abs_dir.y >= abs_dir.x) && (abs_dir.y >= abs_dir.z) && (dir.y < 0)) face = 3; // -y
            if((abs_dir.z >= abs_dir.x) && (abs_dir.z >= abs_dir.y) && (dir.z > 0)) face = 4; // +z
            if((abs_dir.z >= abs_dir.x) && (abs_dir.z >= abs_dir.y) && (dir.z < 0)) face = 5; // -z

            // making sure the dir vector "touches" the cube map face
            glm::highp_vec3 point_on_face = dir / glm::dot(dir, face_dirs.at(face));

            // calculating the uv coords for the point_on_face
            double u = glm::dot(point_on_face, face_rights.at(face)) * 0.5f + 0.5f;
            double v = glm::dot(point_on_face, face_ups.at(face)) * 0.5f + 0.5f;

            // calculating the pixel coords
            int cube_map_size = source.at(face)._width; // should equal the height otherwise it would not qualify as a "cube" in my opinion
            int x = int(u * (cube_map_size - 1)) % cube_map_size; // modulo because im scared of float inaccuracies
            int y = int(v * (cube_map_size - 1)) % cube_map_size;

            return &source.at(face)._pixels.at((y * cube_map_size + x) * source.at(face)._nr_channels);
        }

        template<typename PIXEL_TYPE, typename IMAGE_TYPE>
        void convertEquirectangularToCubeMap(const IMAGE_TYPE& source, std::array<IMAGE_TYPE, 6>& dst, uint32_t cube_map_size) {
            // template function, since there can be more than one image type with different pixel types
            /// @brief convert an equirectangular map to a cubemap (both can describe 360 degrees of an environment)
            /// @param source the equirectangular map
            /// @param dst the six faces of the cubemap
            /// @param cube_map_size the width and height of the cubemap faces

            const double texel_size = 1.0 / cube_map_size; // size of one "pixel" on the cubemap (in uv coords)

            for(int face = 0; face < 6; face++) { // six faces of the cubemap

                // resize the face
                dst.at(face)._width = cube_map_size;
                dst.at(face)._height = cube_map_size;
                dst.at(face)._nr_channels = source._nr_channels;
                dst.at(face)._pixels.resize(cube_map_size * cube_map_size * source._nr_channels);

                for(int x = 0; x < cube_map_size; x++) { // going through every pixel on the cube map face
                    for(int y = 0; y < cube_map_size; y++) {
                        
                        // calculate the direction in which the source map should be sampled
                        glm::highp_vec3 dir = face_dirs.at(face);
                        glm::highp_vec3 right = face_rights.at(face);
                        glm::highp_vec3 up = face_ups.at(face);
                        glm::highp_vec3 sample_dir = dir + ((x * texel_size) - 0.5) * 2.0 * right + ((y * texel_size) - 0.5) * 2.0 * up;
                        sample_dir = glm::normalize(sample_dir);

                        // store the pixel in the cubemap face
                        const PIXEL_TYPE* pixel = equirectangularLookup(source, sample_dir);
                        typename std::vector<PIXEL_TYPE>::iterator insert_pos = dst.at(face)._pixels.begin() + (y * cube_map_size + x) * source._nr_channels;
                        // PIXEL_TYPE* insert_pos = (PIXEL_TYPE*)cubeMapLookup(dst, sample_dir);

                        // i found this to be a lot faster than using insert()
                        for (int i = 0; i < source._nr_channels; i++)
                            *(insert_pos + i) = *(pixel + i);
                        
                    }
                }
            }
        }

        template<typename PIXEL_TYPE, typename IMAGE_TYPE>
        void convoluteCubeMap(const std::array<IMAGE_TYPE, 6>& cube_map, std::array<IMAGE_TYPE, 6>& dst, uint32_t dst_size) {
            /// @brief pre-compute the integral for determining the incoming irradiance for any surface direction (diffuse light)
            /// @param cube_map the environment which to convolute
            /// @param dst the cubemap in which to store the result
            /// @param dst_size the targeted size of the resulting cube map 

            const double texel_size = 1.0 / dst_size; // size of one "pixel" on the cubemap (in uv coords)

            for(int face = 0; face < 6; face++) { // six faces of the cubemap

                // resize the face
                dst.at(face)._width = dst_size;
                dst.at(face)._height = dst_size;
                dst.at(face)._nr_channels = cube_map.at(face)._nr_channels;
                dst.at(face)._pixels.resize(dst_size * dst_size * cube_map.at(face)._nr_channels);

                for(int x = 0; x < dst_size; x++) {
                    for(int y = 0; y < dst_size; y++) { // going through every pixel on the dst cubemap's face

                        // calculating the direction corresponding to the current pixel (normal)
                        glm::vec3 normal = glm::vec3(face_dirs.at(face) + ((x * texel_size) - 0.5) * 2.0 * face_rights.at(face) + ((y * texel_size) - 0.5) * 2.0 * face_ups.at(face));
                        normal = glm::normalize(normal);

                        // going through a finite amount of points on the hemisphere pointing in the normal direction
                        // to discretely solve the irradiance integral
                        // source: https://learnopengl.com/PBR/IBL/Diffuse-irradiance
                        glm::vec3 irradiance = glm::vec3(0.0);
                        glm::vec3 right = glm::normalize(glm::cross(glm::vec3(face_ups.at(face)), normal)); // local coord system for the current hemisphere
                        glm::vec3 up    = glm::normalize(glm::cross(normal, right));

                        float sample_delta = 0.025f;
                        float nr_samples = 0.0f;
                        for(float phi = 0.0; phi < 2.0 * M_PI; phi += sample_delta) {
                            for(float theta = 0.0; theta < 0.5 * M_PI; theta += sample_delta) {
                                // spherical to cartesian (in tangent space)
                                glm::vec3 tangent_sample = glm::vec3(glm::sin(theta) * glm::cos(phi),  glm::sin(theta) * glm::sin(phi), glm::cos(theta));
                                // tangent space to world
                                glm::vec3 sample_vec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal; 

                                // get the pixel from the cube map
                                PIXEL_TYPE* pixels = (PIXEL_TYPE*)cubeMapLookup(cube_map, glm::highp_vec3(sample_vec));
                                irradiance += glm::vec3(pixels[0], pixels[1], pixels[2]) * glm::cos(theta) * glm::sin(theta);
                                nr_samples++;
                            }
                        }

                        irradiance = M_PIf * irradiance * (1.0f / float(nr_samples));
                        
                        // storing the irradiance in the dst cubemap
                        typename std::vector<PIXEL_TYPE>::iterator insert_pos = dst.at(face)._pixels.begin() + (y * dst_size + x) * dst.at(face)._nr_channels;
                        insert_pos[0] = irradiance.r;
                        insert_pos[1] = irradiance.g;
                        insert_pos[2] = irradiance.b;

                    }
                }

            }
        }

        ////////////////////////////////////////// implementation for api functions declared in ibl.h //////////////////////////////////////////

        const char* equirectangularLookup(const ImageData& source, const glm::highp_vec3& dir) {

            return equirectangularLookup<char, ImageData>(source, dir);
        }

        const float* equirectangularLookup(const HDRImageData& source, const glm::highp_vec3& dir) {

            return equirectangularLookup<float, HDRImageData>(source, dir);
        }

        const char* cubeMapLookup(const std::array<ImageData, 6>& source, const glm::highp_vec3& dir) {
            
            return cubeMapLookup<char, ImageData>(source, dir);
        }

        const float* cubeMapLookup(const std::array<HDRImageData, 6>& source, const glm::highp_vec3& dir) {

            return cubeMapLookup<float, HDRImageData>(source, dir);
        }

        void convertEquirectangularToCubeMap(const ImageData& source, std::array<ImageData, 6>& dst, uint32_t cube_map_size) {

            convertEquirectangularToCubeMap<char, ImageData>(source, dst, cube_map_size);
        }

        void convertEquirectangularToCubeMap(const HDRImageData& source, std::array<HDRImageData, 6>& dst, uint32_t cube_map_size) {

            convertEquirectangularToCubeMap<float, HDRImageData>(source, dst, cube_map_size);
        }

        void convoluteCubeMap(const std::array<ImageData, 6>& cube_map, std::array<ImageData, 6>& dst, uint32_t dst_size) {

            convoluteCubeMap<char, ImageData>(cube_map, dst, dst_size);
        }

        void convoluteCubeMap(const std::array<HDRImageData, 6>& cube_map, std::array<HDRImageData, 6>& dst, uint32_t dst_size) {

            convoluteCubeMap<float, HDRImageData>(cube_map, dst, dst_size);
        }

    } // tools

} // undicht