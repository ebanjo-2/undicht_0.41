#ifndef WORLD_LOADER_H
#define WORLD_LOADER_H

#include "drawable_world.h"
#include "files/world_file.h"
#include "environment/environment_generator.h"
#include "glm/glm.hpp"
#include "core/vulkan/fence.h"

namespace cell {

    class WorldLoader {

      protected:

        WorldFile _world_file;
        EnvironmentGenerator _env_gen;

      public:

        void init();
        void cleanUp();

        bool openWorldFile(const std::string& world_file);

        void updateMaterials(DrawableWorld& world, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);

        /** @brief updates the environment from an environment map referenced in the world file
         * or generates a new environment */
        void updateEnvironment(DrawableWorld& world, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);

        /** @brief load the missing chunks around the player 
         * @param prev_frame_finished the fence will be waited on if vulkan buffers are modified
        */
        void loadChunks(const glm::vec3& player_pos, DrawableWorld& world, int32_t chunk_distance, undicht::vulkan::Fence& prev_frame_finished, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);  

    };

} // cell

#endif // WORLD_LOADER_H