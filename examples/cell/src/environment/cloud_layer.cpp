#include "environment/cloud_layer.h"

#include "stb_perlin.h"
#include "debug.h"
#include "array"

namespace cell {

    CloudLayer::CloudLayer() {

    }

    void CloudLayer::initRandomMap() {
        // creating worley noise as described by sebastian lague in the "Coding Adventure: Clouds" video

        // init some random points
        const uint32_t random_offset_cube_size = 10;
        std::vector<std::vector<std::vector<glm::vec3>>> random_offsets(random_offset_cube_size);
        for(uint32_t x = 0; x < random_offset_cube_size; x++) {

            random_offsets.at(x).resize(random_offset_cube_size);
            for(uint32_t y = 0; y < random_offset_cube_size; y++) {

                random_offsets.at(x).at(y).resize(random_offset_cube_size);
                for(uint32_t z = 0; z < random_offset_cube_size; z++) {
                    
                    glm::vec3 random_offset;
                    random_offset.x = (rand() % 1001) / 1000.0f - 0.5f;
                    random_offset.y = (rand() % 1001) / 1000.0f - 0.5f;
                    random_offset.z = (rand() % 1001) / 1000.0f - 0.5f;
                    random_offsets[x][y][z] = random_offset;

                }
            }
        }

        // init the random map
        _random_map.setExtent(_random_map_size, _random_map_size, _random_map_height);
        _random_map.setNrChannels(1); // contains a single float

        for(uint32_t x = 0; x < _random_map_size; x++) {
            for(uint32_t y = 0; y < _random_map_size; y++) {
                for(uint32_t z = 0; z < _random_map_height; z++) {

                    // distance from pos to closest point
                    float closest_distance = 1.0f; 

                    glm::vec3 pos;
                    pos.x = float(x) / _random_map_size * random_offset_cube_size;
                    pos.y = float(y) / _random_map_size * random_offset_cube_size;
                    pos.z = float(z) / _random_map_height * random_offset_cube_size;

                    int cell_x = int(pos.x);
                    int cell_y = int(pos.y);
                    int cell_z = int(pos.z);

                    // checking the points in the neighbouring 26 cells and the one the pixel is in
                    for(int local_x = -1; local_x <= 1; local_x++) {
                        for(int local_y = -1; local_y <= 1; local_y++) {
                            for(int local_z = -1; local_z <= 1; local_z++) {

                                glm::ivec3 local_cell_pos;
                                local_cell_pos.x = (cell_x + local_x);
                                local_cell_pos.y = (cell_y + local_y);
                                local_cell_pos.z = (cell_z + local_z);

                                int local_cell_x = local_cell_pos.x % random_offset_cube_size;
                                int local_cell_y = local_cell_pos.y % random_offset_cube_size;
                                int local_cell_z = local_cell_pos.z % random_offset_cube_size;

                                glm::vec3& offset = random_offsets[local_cell_x][local_cell_y][local_cell_z];
                                glm::vec3 point = glm::vec3(local_cell_pos) + offset;
        
                                float point_distance = glm::length(pos - point);

                                if(point_distance < closest_distance)
                                    closest_distance = point_distance;

                            }
                        }
                    }

                    // store the closest distance to a point
                    _random_map.setPixel(&closest_distance, x, y, z);
                }
            }
        }

        UND_LOG << "initialized the random map \n";
    }

    void CloudLayer::initDensityMap() {

        _density_map.setExtent(_random_map_size, _random_map_size, _random_map_height);
        _density_map.setNrChannels(1); // contains a single float

        for(uint32_t x = 0; x < _random_map_size; x++) {
            for(uint32_t y = 0; y < _random_map_size; y++) {
                for(uint32_t z = 0; z < _random_map_height; z++) {

                    float density = 0.0f;
                    float max_density = 0.0f;

                    for(int i = 0; i < _detail_levels; i++) {

                        float level_weight = 1.0f / (1 << i);
                        float level_scale = (1 << i);

                        uint32_t level_x = uint32_t(x * level_scale) % _random_map_size;
                        uint32_t level_y = uint32_t(y * level_scale) % _random_map_size;
                        uint32_t level_z = uint32_t(z * level_scale) % _random_map_height;

                        density += *_random_map.getPixel(level_x, level_y, level_z) * level_weight;

                        max_density += level_weight;
                    }

                    // normalize the density
                    density /= max_density;
                    density = 1.0f - density;

                    _density_map.setPixel(&density, x, y, z);
                }
            }
        }

        UND_LOG << "initialized the density map \n";
    }

    float CloudLayer::getBaseHeight() const {

        return _base_height;
    }

    float CloudLayer::getCloudHeight() const {

        return _cloud_height;
    }

    float CloudLayer::sample(const glm::vec3& point) const {
        /// @brief sample the cloud layer at a point in the cloud
        /// @return the density of the cloud layer in the requested direction (0 to 1)
        /// @return 0.0f, if the point is below the cloud base height or above the cloud layer

        if((point.y < _base_height) || (point.y >= (_base_height + _cloud_height)))
            return 0.0f;

        float u = point.x;
        float v = point.z;
        float w = point.y - _base_height;

        u = u / _cloud_wrap;
        v = v / _cloud_wrap;
        w = w / _cloud_height;

        return _density_map.sampleLinear(u, v, w)[0];
        //return _density_map.getPixel(u, v, w)[0];
    }

} // cell