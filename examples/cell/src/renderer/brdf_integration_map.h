#ifndef BRDF_INTEGRATION_MAP_H
#define BRDF_INTEGRATION_MAP_H

#include "renderer/vulkan/texture.h"
#include "core/vulkan/command_buffer.h"
#include "renderer/vulkan/transfer_buffer.h"

namespace cell {

    class BRDFIntegrationMap {
      
      protected:

        const int _brdf_integration_map_size = 512;
        undicht::vulkan::Texture _brdf_integration_map;

      public:

        void init(const undicht::vulkan::LogicalDevice& gpu, undicht::vulkan::CommandBuffer& cmd, undicht::vulkan::TransferBuffer& buf);
        void cleanUp();

        const undicht::vulkan::Texture& getMap() const;

    };

} // cell

#endif // BRDF_INTEGRATION_MAP_H