#include "entity.h"

namespace cell {

    const uint32_t ENTITY_TYPE_UNASSIGNED = 0;
    const uint32_t ENTITY_TYPE_POINT_LIGHT = 1;

    void Entity::setType(uint32_t type) {

        _type = type;
    }

    uint32_t Entity::getType() const {
        
        return _type;
    }

} // cell