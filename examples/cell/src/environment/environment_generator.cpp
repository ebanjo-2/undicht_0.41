#include "environment_generator.h"

namespace cell {

    using namespace undicht;
    using namespace tools;


    void EnvironmentGenerator::setSunDir(const glm::vec3& dir) {

        _sun_dir = glm::normalize(dir);
    }

    void EnvironmentGenerator::generate(CubeMapData<float>& dst) {

        dst.setExtent(_target_extent);
        dst.setNrChannels(4);

        for(int face = 0; face < 6; face++) {

            for(int x = 0; x < _target_extent; x++) {

                for(int y = 0; y < _target_extent; y++) {
                    
                    glm::vec3 dir = dst.calcDir(x, y, (CubeMapData<float>::Face)face);

                    glm::vec3 sun_color(23.47, 21.31, 20.79);
                   //glm::vec3 sky_color(128.0f / 255.0f, 218.0f / 255.0f, 235.0f / 255.0f); // "Medium sky blue" (wikipedia)
                    glm::vec3 sky_color(0.0f / 255.0f, 204.0f / 255.0f, 255.0f / 255.0f); // "Vivid sky blue" (wikipedia)

                    float dir_dot_sun = glm::max(0.0f, glm::dot(dir, -_sun_dir) - 0.75f) * 4.0f;
                    float sun_intensity = glm::pow(dir_dot_sun, 10.0f) * 0.05f; 
                    float sky_intensity = 0.02f;

                    glm::vec3 final_color = sun_color * sun_intensity + sky_intensity * sky_color;

                    float pixel[] = {final_color.r, final_color.g, final_color.b, 0.0f};

                    dst.getFace((CubeMapData<float>::Face)face).setPixel(pixel, x, y);
                }

            }

        }

    }

} // cell