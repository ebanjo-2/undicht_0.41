#define _USE_MATH_DEFINES
#include "math.h"

#include "IBL/ibl.h"
#include "glm/glm.hpp"
#include "debug.h"


namespace undicht {

    namespace tools {
        // do what is described by: https://learnopengl.com/PBR/IBL/
        // but on the cpu (should propably also do it on the gpu, but am not 100% confident in my vulkan skills at this time)
        
        void convoluteCubeMap(const CubeMapData<float>& cube_map, CubeMapData<float>& dst, uint32_t dst_size) {
            /// @brief pre-compute the integral for determining the incoming irradiance for any surface direction (diffuse light)
            /// @param cube_map the environment which to convolute
            /// @param dst the cubemap in which to store the result
            /// @param dst_size the targeted size of the resulting cube map 

            const float texel_size = 1.0f / dst_size; // size of one "pixel" on the cubemap (in uv coords)

            // resize the cube map
            dst.setExtent(dst_size);
            dst.setNrChannels(cube_map.getNrChannels());

            for(int face = 0; face < 6; face++) { // six faces of the cubemap

                for(int x = 0; x < dst_size; x++) {
                    for(int y = 0; y < dst_size; y++) { // going through every pixel on the dst cubemap's face

                        // calculating the direction corresponding to the current pixel (normal)
                        glm::vec3 normal = glm::vec3(CUBE_MAP_DIRS.at(face) + ((x * texel_size) - 0.5f) * 2.0f * CUBE_MAP_RIGHTS.at(face) + ((y * texel_size) - 0.5f) * 2.0f * CUBE_MAP_UPS.at(face));
                        normal = glm::normalize(normal);

                        // going through a finite amount of points on the hemisphere pointing in the normal direction
                        // to discretely solve the irradiance integral
                        // source: https://learnopengl.com/PBR/IBL/Diffuse-irradiance
                        glm::vec3 irradiance = glm::vec3(0.0);
                        glm::vec3 right = glm::normalize(glm::cross(CUBE_MAP_UPS.at(face), normal)); // local coord system for the current hemisphere
                        glm::vec3 up    = glm::normalize(glm::cross(normal, right));

                        float sample_delta = 0.05f; // 0.025f;
                        uint32_t nr_samples = 0;
                        for(float phi = 0.0f; phi < 2.0f * M_PI; phi += sample_delta) {
                            for(float theta = 0.0f; theta < 0.5f * M_PI; theta += sample_delta) {
                                // spherical to cartesian (in tangent space)
                                float sin_theta = glm::sin(theta); // used twice
                                float cos_theta = glm::cos(theta); // used twice
                                glm::vec3 tangent_sample = glm::vec3(sin_theta * glm::cos(phi), sin_theta * glm::sin(phi), cos_theta);
                                // tangent space to world
                                glm::vec3 sample_vec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal;

                                // get the pixel from the cube map
                                float* pixels = cube_map.getPixel(sample_vec);
                                irradiance += glm::vec3(pixels[0], pixels[1], pixels[2]) * cos_theta * sin_theta;
                                nr_samples++;
                            }
                        }

                        irradiance = float(M_PI) * irradiance * (1.0f / float(nr_samples));
                        
                        // storing the irradiance in the dst cubemap
                        const float pixel[] = {
                            irradiance.r,
                            irradiance.g,
                            irradiance.b,
                            0.0f
                        };

                        dst.getFace((CubeMapData<float>::Face)face).setPixel(pixel, x, y);

                    }
                }

            }
        }

        float radicalInverseVdC(uint32_t bits) {
            // Van Der Corput sequence 
            // as much as i understand it these are basically pseudo random numbers
            // the Hammersley sequence is based on this, which creates low-discrepancy pseudo random numbers
            // copied from: https://learnopengl.com/PBR/IBL/Specular-IBL
            bits = (bits << 16u) | (bits >> 16u);
            bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
            bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
            bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
            bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
            return float(bits) * 2.3283064365386963e-10; // / 0x100000000
        }

        glm::vec2 hammersley(uint32_t i, uint32_t N) {
            // The [...] Hammersley function gives us the low-discrepancy sample i of the total sample set of size N
            // copied from https://learnopengl.com/PBR/IBL/Specular-IBL
            return glm::vec2(float(i)/float(N), radicalInverseVdC(i));
        }

        glm::vec3 importanceSampleGGX(glm::vec2 Xi, glm::vec3 N, float roughness) {
            // copied from https://learnopengl.com/PBR/IBL/Specular-IBL
            // creates sample vectors
            float a = roughness*roughness;
            
            float phi = 2.0 * M_PI * Xi.x;
            float cosTheta = glm::sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
            float sinTheta = glm::sqrt(1.0 - cosTheta*cosTheta);
            
            // from spherical coordinates to cartesian coordinates
            glm::vec3 H;
            H.x = glm::cos(phi) * sinTheta;
            H.y = glm::sin(phi) * sinTheta;
            H.z = cosTheta;
            
            // from tangent-space vector to world-space sample vector
            glm::vec3 up        = glm::abs(N.z) < 0.999 ? glm::vec3(0.0, 0.0, 1.0) : glm::vec3(1.0, 0.0, 0.0);
            glm::vec3 tangent   = glm::normalize(glm::cross(up, N));
            glm::vec3 bitangent = glm::cross(N, tangent);
            
            glm::vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
            return glm::normalize(sampleVec);
        }  

        void prefilterSpecularReflections(const CubeMapData<float>& cube_map, std::vector<CubeMapData<float>>& dst, uint32_t dst_size, uint32_t mip_levels) {
            /// @brief precalculate the specular reflections resulting from the environment for varying roughness levels
            /// @param cube_map environment map
            /// @param dst cube map mip-levels that contain the result of the prefiltering, the mip map level 0 is the highest resolution (=dst_size) and contains the reflections for the lowest roughness
            /// @param dst_size size of mip-level 0
            /// @param mip_levels number of mip-levels to generate

            dst.resize(mip_levels);

            for(int mip_level = 0; mip_level < mip_levels; mip_level++) {

                uint32_t mip_level_size = dst_size / (1u << mip_level);
                float texel_size = 1.0f / mip_level_size; // size of one "pixel" on the cubemap (in uv coords)

                // calculating the corresponding roughness for the current mip_level
                float roughness = (float)mip_level / (float)(mip_levels - 1); // mip_level 0 should have a roughness of 0

                // resize the cubemap mip level
                dst.at(mip_level).setExtent(mip_level_size);
                dst.at(mip_level).setNrChannels(cube_map.getNrChannels());

                for(int face = 0; face < 6; face++) { // six faces of the cubemap

                    for(int x = 0; x < mip_level_size; x++) {
                        for(int y = 0; y < mip_level_size; y++) { // going through every pixel on the dst cubemap's face for the current mip_level

                            // calculating the direction corresponding to the current pixel (normal)
                            glm::vec3 normal = glm::vec3(CUBE_MAP_DIRS.at(face) + ((x * texel_size) - 0.5f) * 2.0f * CUBE_MAP_RIGHTS.at(face) + ((y * texel_size) - 0.5f) * 2.0f * CUBE_MAP_UPS.at(face));
                            normal = glm::normalize(normal);

                            // prefiltering the specular reflection in the direction of the current pixel
                            // source: https://learnopengl.com/PBR/IBL/Specular-IBL
                            glm::vec3 N = normal;
                            glm::vec3 R = N;
                            glm::vec3 V = R;

                            const uint32_t SAMPLE_COUNT = 1024u * roughness + 1; // the * roughness + 1 part is experimental 
                            float totalWeight = 0.0f;
                            glm::vec3 prefilteredColor = glm::vec3(0.0f);
                            for(int i = 0; i < SAMPLE_COUNT; i++){
                                glm::vec2 Xi = hammersley(i, SAMPLE_COUNT);
                                glm::vec3 H  = importanceSampleGGX(Xi, N, roughness);
                                glm::vec3 L  = glm::normalize(2.0f * glm::dot(V, H) * H - V);

                                float NdotL = glm::max(glm::dot(N, L), 0.0f);
                                if(NdotL > 0.0f) {
                                    // get the color value in the direction L from the environment map
                                    float* pixel = cube_map.getPixel(L);
                                    prefilteredColor += glm::vec3(pixel[0], pixel[1], pixel[2]) * NdotL;
                                    totalWeight      += NdotL;
                                }
                            }
                            prefilteredColor = prefilteredColor / totalWeight;

                            // write the prefiltered color to the dst cubemap at the current mip level
                            float pixel[] = {
                                prefilteredColor.r,
                                prefilteredColor.g,
                                prefilteredColor.b,
                                0.0f,
                            };

                            dst.at(mip_level).getFace((CubeMapData<float>::Face)face).setPixel(pixel, x, y);

                        }
                    }
                }

            }

        }

        float geometrySchlickGGX(float NdotV, float roughness) {

            float a = roughness;
            float k = (a * a) / 2.0;

            float nom   = NdotV;
            float denom = NdotV * (1.0 - k) + k;

            return nom / denom;
        }

        float geometrySmith(glm::vec3 N, glm::vec3 V, glm::vec3 L, float roughness) {
            float NdotV = glm::max(glm::dot(N, V), 0.0f);
            float NdotL = glm::max(glm::dot(N, L), 0.0f);
            float ggx2 = geometrySchlickGGX(NdotV, roughness);
            float ggx1 = geometrySchlickGGX(NdotL, roughness);

            return ggx1 * ggx2;
        }

        void createBRDFIntegrationMap(ImageData<float>& dst, uint32_t dst_size) {
            /// @brief the brdf map contains a precalculated scale and a bias for combinations of a fresnel factor (the dot product of view and normal vector) and roughness(y-Axis)
            /// @param dst the 2D image to fill with the precalculated values (pixel format is vec2f)
            /// @param dst_size size of that image

            // resize the dst texture
            dst.setExtent(dst_size, dst_size);
            dst.setNrChannels(2);

            float texel_size = 1.0f / dst_size;

            for(uint32_t x = 0; x < dst_size; x++) {
                for(uint32_t y = 0; y < dst_size; y++) { // going through every pixel on the texture
                    // source: https://learnopengl.com/PBR/IBL/Specular-IBL

                    float NdotV = x * texel_size;
                    float roughness = y * texel_size;
        
                    glm::vec3 V;
                    V.x = glm::sqrt(1.0f - NdotV*NdotV);
                    V.y = 0.0f;
                    V.z = NdotV;

                    float A = 0.0f;
                    float B = 0.0f;

                    glm::vec3 N = glm::vec3(0.0f, 0.0f, 1.0f);

                    const uint32_t SAMPLE_COUNT = 128u; // originally: 1024u
                    for(uint32_t i = 0u; i < SAMPLE_COUNT; ++i) {
                        glm::vec2 Xi = hammersley(i, SAMPLE_COUNT);
                        glm::vec3 H  = importanceSampleGGX(Xi, N, roughness);
                        glm::vec3 L  = glm::normalize(2.0f * glm::dot(V, H) * H - V);

                        float NdotL = glm::max(L.z, 0.0f);
                        float NdotH = glm::max(H.z, 0.0f);
                        float VdotH = glm::max(glm::dot(V, H), 0.0f);

                        if(NdotL > 0.0f) {
                            float G = geometrySmith(N, V, L, roughness);
                            float G_Vis = (G * VdotH) / (NdotH * NdotV);
                            float Fc = glm::pow(1.0f - VdotH, 5.0f);

                            A += (1.0f - Fc) * G_Vis;
                            B += Fc * G_Vis;
                        }
                    }

                    A /= float(SAMPLE_COUNT);
                    B /= float(SAMPLE_COUNT);

                    // storing the A and B values in the texture
                    dst.getPixel(x, y)[0] = A;
                    dst.getPixel(x, y)[1] = B;
                }
            }

        }

    } // tools

} // undicht