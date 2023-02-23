#ifndef CELL_DRAWABLE_WORLD
#define CELL_DRAWABLE_WORLD

#include "world/cells/cell_world.h"
#include "world/cells/cell_buffer.h"

#include "materials/material_atlas.h"

#include "world/lights/light_world.h"
#include "world/lights/light_buffer.h"


namespace cell {

    class DrawableWorld {
        // holds and updates the graphics api objects necessary to display the world (including lighting)

      protected:

        // cells
        CellWorld _cell_world;
        CellBuffer _cell_buffer;

        // lights
        Light _sun; // cant be put into any chunk
        LightWorld _light_world;
        LightBuffer _light_buffer; // point lights

        MaterialAtlas _materials;

      public:
      
        void init(const undicht::vulkan::LogicalDevice& device);
        void cleanUp();

      public:
        // update parts of the drawable world

        CellWorld& getCellWorld();
        LightWorld& getLightWorld();

        // sun / directional light
        void setSunDirection(const glm::vec3& dir);
        void setSunColor(const glm::vec3& color);

        // make sure the chunk is correctly stored in the world buffer
        void updateWorldBuffer();
        void updateLightBuffer();
      
      public:
        // access parts of the drawable world (for rendering)

        const CellBuffer& getCellBuffer() const;
        const LightBuffer& getLightBuffer() const;
        MaterialAtlas& getMaterialAtlas();
        const Light& getSun() const;

    };

} // namespace cell

#endif // CELL_DRAWABLE_WORLD