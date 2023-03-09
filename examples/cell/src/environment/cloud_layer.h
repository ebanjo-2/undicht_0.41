#ifndef CLOUD_LAYER_H
#define CLOUD_LAYER_H

#include "images/image_data.h"
#include "images/image_data_3d.h"
#include "glm/glm.hpp"

namespace cell {

    class CloudLayer {
      
      protected:

        float _base_height = 100.0f;
        float _cloud_height = 50.0f;
        float _cloud_wrap = 2000.0f;

        uint32_t _detail_levels = 5;
        uint32_t _random_map_height = 24;
        uint32_t _random_map_size = 256;
        undicht::tools::ImageData3D<float> _random_map;
        undicht::tools::ImageData3D<float> _density_map; // combines the detail levels, same size as random map

      public:

        CloudLayer();

        void initRandomMap();
        void initDensityMap();

        float getBaseHeight() const;
        float getCloudHeight() const;

        /// @brief sample the cloud layer at a point in the cloud
        /// @return the density of the cloud layer in the requested direction (0 to 1)
        /// @return 0.0f, if the point is below the cloud base height or above the cloud layer
        float sample(const glm::vec3& point) const;
        
    };

} // cell

#endif // CLOUD_LAYER_H