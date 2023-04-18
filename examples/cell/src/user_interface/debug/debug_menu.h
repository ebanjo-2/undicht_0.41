#ifndef DEBUG_MENU_H
#define DEBUG_MENU_H

#include "environment/environment.h"
#include "images/cube_map_data.h"

#include "core/vulkan/command_buffer.h"
#include "renderer/vulkan/transfer_buffer.h"

namespace cell {

    class DebugMenu {
    
      protected:

        bool _open = false;

        float _cloud_coverage = 0.7f;
        float _cloud_density = 1.5f;
        float _sky_brightness = 1.0f;
        float _cloud_brightness = 1.0f;
        undicht::tools::CubeMapData<float> _env_map_data;

        bool _update_environment = false;
        bool _update_light_maps = false;

      public:

        void open();
        void close();

        bool isOpen() const;

        void applyUpdates(Environment& env, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf);

        void display(double fps);

    };

} // cell

#endif // DEBUG_MENU_H