#ifndef ENVIRONMENT_GENERATOR
#define ENVIRONMENT_GENERATOR

#include "images/cube_map_data.h"
#include "cstdint"
#include "glm/glm.hpp"

namespace cell {

    class EnvironmentGenerator {

      protected:

        uint32_t _target_extent = 1024; // target size of the generated environment maps

        // environment variables
        glm::vec3 _sun_dir = glm::vec3(0.0f,1.0f,0.0f); // straight down

      public:

        void setSunDir(const glm::vec3& dir);

        void generate(undicht::tools::CubeMapData<float>& dst);

    };

} // cell

#endif // ENVIRONMENT_GENERATOR