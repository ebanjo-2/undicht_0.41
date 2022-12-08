#ifndef CELL_PLAYER_H

#define CELL_PLAYER_H
#include "3D/camera/perspective_camera_3d.h"

namespace cell {

    class Player : public undicht::tools::PerspectiveCamera3D {

      protected:

      public:

        void init();
        void cleanUp();

        void move();

      protected:

    };

} // namespace cell

#endif // CELL_PLAYER_H