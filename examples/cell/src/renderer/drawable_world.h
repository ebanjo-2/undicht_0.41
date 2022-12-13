#ifndef CELL_DRAWABLE_WORLD
#define CELL_DRAWABLE_WORLD

#include "world/world.h"
#include "renderer/world_buffer.h"

namespace cell {

    class DrawableWorld : public World {

      protected:

        WorldBuffer _world_buffer;

      public:
      
        void init(const undicht::vulkan::LogicalDevice& device);
        void cleanUp();

        // make sure the chunk is correctly stored in the world buffer
        void updateWorldBuffer(const glm::ivec3& chunk_pos);

        const WorldBuffer& getWorldBuffer() const;
    };

} // namespace cell

#endif // CELL_DRAWABLE_WORLD