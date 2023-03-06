#include "environment/cloud_layer.h"

#include "stb_perlin.h"
#include "debug.h"

namespace cell {

    CloudLayer::CloudLayer() {

        // init the random map
        _random_map.setExtent(_random_map_size, _random_map_size, _random_map_height);
        _random_map.setNrChannels(1); // contains a single float

        for(uint32_t x = 0; x < _random_map_size; x++) {
            for(uint32_t y = 0; y < _random_map_size; y++) {
                for(uint32_t z = 0; z < _random_map_height; z++) {

                    float perlin_x = x / 16.0f;
                    float perlin_y = y / 16.0f;
                    float perlin_z = z / 16.0f;

                    float noise = glm::max(0.0f, stb_perlin_noise3(perlin_x, perlin_y, perlin_z, 0, 0, 0));
                    _random_map.setPixel(&noise, x, y, z);
                }
            }
        }

        UND_LOG << "initialized the random map \n";

    }

    float CloudLayer::sample(const glm::vec3& dir) const {
        /// @return the density of the cloud layer in the requested direction

        if(dir.y >= 0) return 0.0f; // pointing down

        float density_sum = 0.0f;

        glm::vec3 sample_point = dir / dir.y * _base_height;
        glm::vec3 sample_point_increment = dir / dir.y * (_cloud_height / _sample_count);

        for(uint32_t i = 0; i < _sample_count; i++) {

            for(uint32_t level = 0; level < _detail_levels; level++) {
                
                density_sum += sample(sample_point, level) * (1 << (_detail_levels - level));
            }

            sample_point += sample_point_increment;
        }

        density_sum /= ((1 << (_detail_levels + 1)) - 1); // normalizing the density

        return glm::max(0.0f, density_sum - _threshold_density);
        //return sample(sample_point, 0);
    }

    /////////////////////////////////// protected CloudLayer functions ///////////////////////////////////

    float CloudLayer::sample(const glm::vec3& point, uint32_t detail_level) const {
        // sample the _random_map at a point in the cloud

        float x = point.x * (detail_level + 1);
        float y = point.z * (detail_level + 1);
        float z = (point.y - _cloud_height) * (detail_level + 1);

        x = x * (_random_map_size / _cloud_wrap);
        y = y * (_random_map_size / _cloud_wrap);
        z = z * (_random_map_height / _cloud_height);

        uint32_t sample_x = uint32_t(int(x) + 10000000) % _random_map_size;
        uint32_t sample_y = uint32_t(int(y) + 10000000) % _random_map_size;
        uint32_t sample_z = uint32_t(z) % _random_map_height;

        return *_random_map.getPixel(sample_x, sample_y, sample_z);
    }

} // cell