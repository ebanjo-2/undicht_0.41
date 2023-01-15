#ifndef ENTITY_CHUNK_H
#define ENTITY_CHUNK_H

#include "vector"
#include "entity.h"

namespace cell {

    class EntityChunk {

    protected:

        std::vector<Entity> _entities;

        // keeping track of which cells are no longer used and can be recycled
        std::vector<uint32_t> _unused_entities;

        // keeping track of whether the chunk was edited (and needs to be updated in gpu memory)
        bool _has_changed = false;

    public:

        // updating entities (adding, changing, removing)
        uint32_t addEntity(const Entity& e); // returns an id with which the entity can be accessed
        void setEntity(uint32_t id, const Entity& e);
        void removeEntity(uint32_t id);

        // getting entities
        // when adding entities to the chunk, the internal vector containing the entities may resize
        // this will effect the const Entity* pointers, but not the ids (something to keep in mind)
        const Entity* getEntity(uint32_t id) const; // returns nullptr if there is no entity with that id

        uint32_t getEntityCount() const;
        const std::vector<Entity>& getAllEntities() const;

        bool getWasEdited() const;
        void markAsUnEdited();
        
        // loading the chunk from existing data
        // will remove the current data in the chunks _entities buffer
        void initFromData(const std::vector<Entity>& entities);
    };

} // cell

#endif // ENTITY_CHUNK_H