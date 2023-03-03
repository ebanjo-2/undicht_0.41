#ifndef BRDF_INTEGRATION_MAP_H
#define BRDF_INTEGRATION_MAP_H

#include "renderer/vulkan/texture.h"

namespace cell {

    class BRDFIntegrationMap {
      
      protected:

        const int _brdf_integration_map_size = 512;
        undicht::vulkan::Texture _brdf_integration_map;

      public:

        void init(const undicht::vulkan::LogicalDevice& gpu);
        void cleanUp();

        const undicht::vulkan::Texture& getMap() const;

    };

} // cell

#endif // BRDF_INTEGRATION_MAP_H