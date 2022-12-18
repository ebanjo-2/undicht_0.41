#ifndef CELL_PLAYER_H

#define CELL_PLAYER_H
#include "3D/camera/perspective_camera_3d.h"
#include "window/glfw/window.h"

namespace cell {

    class Player : public undicht::tools::PerspectiveCamera3D {

      protected:

      public:

        void init();
        void cleanUp();

        bool _use_mouse_input = true;

        bool _cursor_initialized = false;
        float _last_cursor_x = 0;
        float _last_cursor_y = 0;

        float _yaw = 0;
        float _pitch = 0;

        void enableMouseInput(bool enable);

        /// @brief move the player
        /// @param delta_t time since last call in milliseconds
        void move(double delta_t, const undicht::graphics::Window& input_window);

      protected:

    };

} // namespace cell

#endif // CELL_PLAYER_H