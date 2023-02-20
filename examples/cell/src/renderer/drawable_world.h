#ifndef CELL_DRAWABLE_WORLD
#define CELL_DRAWABLE_WORLD

#include "world/world.h"
#include "renderer/world_buffer.h"
#include "renderer/light_buffer.h"
#include "materials/material_atlas.h"
#include "entities/lights/direct_light.h"
#include "entities/lights/point_light.h"

namespace cell {

    class DrawableWorld : public World {
        // holds and updates the graphics api objects necessary to display the world (including lighting)

      protected:

        DirectLight _sun;
        std::vector<PointLight> _lights;

        WorldBuffer _world_buffer;
        LightBuffer _light_buffer; // point lights
        MaterialAtlas _materials;

      public:
      
        void init(const undicht::vulkan::LogicalDevice& device);
        void cleanUp();

      public:
        // update parts of the drawable world

        // point lights
        uint32_t addLight(const PointLight& light);
        void removeLight(uint32_t light_id);
        void removeAllLights();

        // sun / directional light
        void setSunDirection(const glm::vec3& dir);
        void setSunColor(const glm::vec3& color);
        void setSunTarget(const glm::vec3& target); // for shadow mapping

        // make sure the chunk is correctly stored in the world buffer
        void updateWorldBuffer();
        void updateLightBuffer();
      
      public:
        // access parts of the drawable world (for rendering)

        const WorldBuffer& getWorldBuffer() const;
        const LightBuffer& getLightBuffer() const;
        const MaterialAtlas& getMaterialAtlas() const;
        const DirectLight& getSun() const;

    };

} // namespace cell

#endif // CELL_DRAWABLE_WORLD