#ifndef LIGHT_WORLD_H
#define LIGHT_WORLD_H

#include "world/lights/light.h"
#include "world/lights/light_chunk.h"
#include "world/chunk_system/chunk_system.h"
#include "light_buffer.h"

namespace cell {

    class LightWorld : public ChunkSystem<Light> {

      protected:

        LightBuffer _buffer;

      public:

        void init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);
        void cleanUp();

        /** updates the vulkan buffer with the changes made since the last call to applyUpdates() 
         * @param load_cmd records the commands necessary to move the data from the transfer buffer to the internal vulkan buffer 
         * @param load_buf used as a staging buffer to transfer data to memory that is not directly visible to the cpu */
        void applyUpdates(undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);

        const LightBuffer& getBuffer() const;

    };

} // cell

#endif // LIGHT_WORLD_H