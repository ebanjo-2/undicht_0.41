#include "environment_generator.h"

namespace cell {

    using namespace undicht;
    using namespace tools;

    void EnvironmentGenerator::setSunDir(const glm::vec3& dir) {

        _sun_dir = glm::normalize(dir);
    }

    void EnvironmentGenerator::addCloudLayer() {

        _cloud_layers.emplace_back(CloudLayer());
    }

    void EnvironmentGenerator::generate(CubeMapData<float>& dst) {

        dst.setExtent(_target_extent);
        dst.setNrChannels(4);

        for(int face = 0; face < 6; face++) {

            for(int x = 0; x < _target_extent; x++) {

                for(int y = 0; y < _target_extent; y++) {
                    
                    glm::vec3 dir = dst.calcDir(x, y, (CubeMapData<float>::Face)face);

                    glm::vec3 final_color;
                    
                    if(_cloud_layers.size()) {
                        
                        float cloud_density = _cloud_layers.at(0).sample(dir);

                        glm::vec3 sky_color = glm::vec3(0.6f, 0.6f, 1.0f);
                        glm::vec3 cloud_color = glm::vec3(1.0f, 1.0f, 1.0f);
                        final_color = cloud_density * cloud_color + (1 - cloud_density) * sky_color;
                    }

                    float pixel[] = {final_color.r, final_color.g, final_color.b, 0.0f};
                    dst.getFace((CubeMapData<float>::Face)face).setPixel(pixel, x, y);
                }

            }

        }

    }

} // cell