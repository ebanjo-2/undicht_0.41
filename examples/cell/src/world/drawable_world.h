#ifndef CELL_DRAWABLE_WORLD
#define CELL_DRAWABLE_WORLD

#include "world/cells/cell_world.h"
#include "world/cells/cell_buffer.h"

#include "materials/material_atlas.h"

#include "world/lights/light_world.h"
#include "world/lights/light_buffer.h"

#include "environment/environment.h"


namespace cell {

    class DrawableWorld {
        // holds and updates the graphics api objects necessary to display the world (including lighting)

      protected:

        // cells
        CellWorld _cell_world;

        // lights
        Light _sun; // cant be put into any chunk
        LightWorld _light_world;

        MaterialAtlas _materials;
        Environment _environment; // "skybox"

      public:
      
        void init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);
        void cleanUp();

      public:
        // update parts of the drawable world

        CellWorld& getCellWorld();
        LightWorld& getLightWorld();
        MaterialAtlas& getMaterialAtlas();
        Environment& getEnvironment();

        // sun / directional light
        void setSunDirection(const glm::vec3& dir);
        void setSunColor(const glm::vec3& color);

        // make sure each chunk is correctly stored in the vulkan buffers
        void applyUpdates(undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);
      
      public:
        // access parts of the drawable world (for rendering)

        const CellBuffer& getCellBuffer() const;
        const LightBuffer& getLightBuffer() const;
        const Light& getSun() const;

    };

} // namespace cell

#endif // CELL_DRAWABLE_WORLD