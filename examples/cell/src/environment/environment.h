#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "core/vulkan/logical_device.h"
#include "renderer/vulkan/texture.h"
#include "string"
#include "images/cube_map_data.h"

namespace cell {

    class Environment {

      protected:

        const int _env_cube_map_size = 1024; // width and height of the cubemap faces
        undicht::vulkan::Texture _env_cube_map; // "sky box" (the environment map that the image based lighting is based on)

        const int _irradiance_map_size = 16; // small size should be enough
        undicht::vulkan::Texture _irradiance_map; // a cube map that contains the diffuse light for every surface normal direction

        const int _specular_prefilter_map_size = 64; // size of the highest mip level (reflections for the smoothest surface)
        const int _specular_prefilter_mip_levels = 5;
        undicht::vulkan::Texture _specular_prefilter_map; // contains the specular light for various roughness levels

      public:

        void init(const undicht::vulkan::LogicalDevice& gpu);
        void cleanUp();

        /// loads the environment map from the hdr spherical map
        /// and calculates the lighting maps based on it
        void load(const std::string& file_name);
        void load(const undicht::tools::CubeMapData<float>& env_map);

        const undicht::vulkan::Texture& getSkyBox() const;
        const undicht::vulkan::Texture& getIrradiance() const;
        const undicht::vulkan::Texture& getSpecular() const;

    };

} // cell

#endif // ENVIRONMENT_H