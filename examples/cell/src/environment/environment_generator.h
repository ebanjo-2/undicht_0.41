#ifndef ENVIRONMENT_GENERATOR
#define ENVIRONMENT_GENERATOR

#include "images/cube_map_data.h"
#include "cstdint"
#include "glm/glm.hpp"
#include "images/image_data_3d.h"
#include "environment/cloud_layer.h"

namespace cell {

    class EnvironmentGenerator {

      protected:

        uint32_t _target_extent = 1024; // target size of the generated environment maps

        // environment variables
        glm::vec3 _sun_dir = glm::vec3(0.0f,1.0f,0.0f); // straight down

        float _cloud_threshold = 0.45f; // controls the coverage of the clouds, 1.0f means no clouds
        float _cloud_density = 2.0f;
        float _sky_brightness = 1.0f;
        float _cloud_brightness = 1.0f;
        CloudLayer _cloud_layer;

      public:

        void init();
        void cleanUp();

        void setSunDir(const glm::vec3& dir);

        void setCloudCoverage(float coverage);
        void setCloudDensity(float density);
        void setSkyBrightness(float brightness);
        void setCloudBrightness(float brightness);

        void generate(undicht::tools::CubeMapData<float>& dst);

      protected:

        float sampleCloudDensity(const glm::vec3& dir, uint32_t num_samples) const;

    };

} // cell

#endif // ENVIRONMENT_GENERATOR