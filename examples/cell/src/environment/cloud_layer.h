#ifndef CLOUD_LAYER_H
#define CLOUD_LAYER_H

#include "images/image_data.h"
#include "images/image_data_3d.h"
#include "glm/glm.hpp"

namespace cell {

    class CloudLayer {
      
      protected:

        float _threshold_density = 0.2f; // 0 to 1
        float _base_height = 1000.0f;
        float _cloud_height = 50.0f;
        float _cloud_wrap = 100000.0f;

        uint32_t _sample_count = 5;

        uint32_t _detail_levels = 5;
        uint32_t _random_map_height = 64;
        uint32_t _random_map_size = 1024;
        undicht::tools::ImageData3D<float> _random_map;

      public:

        CloudLayer();

        /// @return the density of the cloud layer in the requested direction
        float sample(const glm::vec3& dir) const;

      protected:
        // protected CloudLayer functions

        // sample the _random_map at a point in the cloud
        float sample(const glm::vec3& point, uint32_t detail_level) const;
        
    };

} // cell

#endif // CLOUD_LAYER_H