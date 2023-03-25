#include "environment_generator.h"

namespace cell {

    using namespace undicht;
    using namespace tools;

    void EnvironmentGenerator::init() {

        _cloud_layer.initRandomMap();
        _cloud_layer.initDensityMap();

    }

    void EnvironmentGenerator::cleanUp() {

    }

    void EnvironmentGenerator::setSunDir(const glm::vec3& dir) {

        _sun_dir = glm::normalize(dir);
    }

    void EnvironmentGenerator::setCloudCoverage(float coverage) {

        _cloud_threshold = 1.0f - coverage;
    }

    void EnvironmentGenerator::setCloudDensity(float density) {

        _cloud_density = density;
    }

    void EnvironmentGenerator::setSkyBrightness(float brightness) {

        _sky_brightness = brightness;
    }

    void EnvironmentGenerator::setCloudBrightness(float brightness) {

        _cloud_brightness = brightness;
    }

    void EnvironmentGenerator::generate(CubeMapData<float>& dst) {

        dst.setExtent(_target_extent);
        dst.setNrChannels(4);

        for(int face = 0; face < 6; face++) {

            for(int x = 0; x < _target_extent; x++) {

                for(int y = 0; y < _target_extent; y++) {
                    
                    glm::vec3 dir = dst.calcDir(x, y, (CubeMapData<float>::Face)face);

                    glm::vec3 final_color;
                    glm::vec3 sky_color = glm::vec3(0.2f, 0.2f, 0.6f) * _sky_brightness;
                    glm::vec3 cloud_color = glm::vec3(1.0f, 1.0f, 1.0f) * _cloud_brightness;
                    glm::vec3 fog_color = glm::vec3(0.7f, 0.7f, 0.7f);

                    glm::vec3 cloud_dir = dir + glm::vec3(0.0f, -0.07f, 0.0f); // a dirty trick to create a round looking sky

                    if(cloud_dir.y < 0) { // up

                        glm::vec3 point_in_cloud = cloud_dir / cloud_dir.y * _cloud_layer.getBaseHeight();
                        float distance_to_cloud = glm::length(point_in_cloud);
                        float cloud_density = sampleCloudDensity(cloud_dir, 5);
                        cloud_density = glm::min(1.0f, glm::max(0.0f, cloud_density - _cloud_threshold) / (1.0f - _cloud_threshold) * _cloud_density);

                        final_color = cloud_density * cloud_color + (1 - cloud_density) * sky_color;
                        final_color = glm::mix(final_color, fog_color, glm::min(1.0f, distance_to_cloud / 3000.0f));
                    } else {
                        final_color = fog_color;
                    }

                    float pixel[] = {final_color.r, final_color.g, final_color.b, 0.0f};
                    dst.getFace((CubeMapData<float>::Face)face).setPixel(pixel, x, y);
                }

            }

        }

    }

    ///////////////////////////////////////// protected EnvironmentGenerator functions //////////////////////////////////

    float EnvironmentGenerator::sampleCloudDensity(const glm::vec3& dir, uint32_t num_samples) const {
        
        // first point at the bottom of the cloud layer
        glm::vec3 point_in_cloud = dir / dir.y * _cloud_layer.getBaseHeight();
        glm::vec3 sample_step = dir / dir.y * (_cloud_layer.getCloudHeight() / float(num_samples));

        //point_in_cloud += 0.5f * sample_step; 

        float total_density = 0.0f;

        for(uint32_t sample = 0; sample < num_samples; sample++) {
            
            total_density += _cloud_layer.sample(point_in_cloud);
            point_in_cloud += sample_step;
        }

        return total_density / num_samples;
    }


} // cell