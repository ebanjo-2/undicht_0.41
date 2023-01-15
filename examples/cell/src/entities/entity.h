#ifndef ENTITY_H
#define ENTITY_H

#include "math/orientation_3d.h"
#include "cstdint"

namespace cell {

    const extern uint32_t ENTITY_TYPE_UNASSIGNED;
    const extern uint32_t ENTITY_TYPE_POINT_LIGHT;

    class Entity : public undicht::tools::Orientation3D {
      
      protected:

        uint32_t _type = ENTITY_TYPE_UNASSIGNED;

      public:

        void setType(uint32_t type);
        uint32_t getType() const;
        
    };

} // namespace cell

#endif // ENTITY_H