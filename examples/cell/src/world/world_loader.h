#ifndef WORLD_LOADER_H
#define WORLD_LOADER_H

#include "drawable_world.h"
#include "files/world_file.h"
#include "environment/environment_generator.h"
#include "glm/glm.hpp"

namespace cell {

    class WorldLoader {

      protected:

        DrawableWorld _world;
        WorldFile _world_file;
        EnvironmentGenerator _env_gen;

      public:

        void init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);
        void cleanUp();

        bool openWorldFile(const std::string& world_file);

        void updateMaterials(undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);

        /** @brief updates the environment from an environment map referenced in the world file
         * or generates a new environment */
        void updateEnvironment(undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);

        /** @brief load the missing chunks around the player */
        void loadChunks(const glm::vec3& player_pos, int32_t chunk_distance, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);  

        DrawableWorld& getWorld();

    };

} // cell

#endif // WORLD_LOADER_H