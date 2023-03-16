#ifndef DEBUG_MENU_H
#define DEBUG_MENU_H

#include "environment/environment.h"
#include "images/cube_map_data.h"

namespace cell {

    class DebugMenu {
    
      protected:

        bool _open = false;

        float _cloud_coverage = 0.7f;
        float _cloud_density = 1.5f;
        float _sky_brightness = 1.0f;
        float _cloud_brightness = 1.0f;
        undicht::tools::CubeMapData<float> _env_map_data;

      public:

        void open();
        void close();

        bool isOpen() const;

        void display(double fps, Environment& env);

    };

} // cell

#endif // DEBUG_MENU_H