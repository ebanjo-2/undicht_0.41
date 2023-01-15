#include "entity_chunk.h"

namespace cell {

    ////////////////////////////// updating entities (adding, changing, removing) //////////////////////////////

    uint32_t EntityChunk::addEntity(const Entity& e) {
        // returns an id with which the entity can be accessed

        uint32_t entity_id;

        if (_unused_entities.size()) { // trying to recycle an unused entity
            _entities.at(_unused_entities.back()) = e;
            entity_id = _unused_entities.back();
            _unused_entities.pop_back();
        } else { // adding the entity at the back of the entity vector
            _entities.push_back(e);
            entity_id = _entities.size() - 1;
        }

        _has_changed = true;

        return entity_id;
    } 

    void EntityChunk::setEntity(uint32_t id, const Entity& e) {

        if (id >= _entities.size())
            return;

        _entities.at(id) = e;

        _has_changed = true;
    }

    void EntityChunk::removeEntity(uint32_t id) {

        if (id >= _entities.size())
            return;

        // giving the entity up for recycling
        _unused_entities.push_back(id);
        _entities.at(id) = Entity(); // clearing the contents stored there

        _has_changed = true;
    }

    /////////////////////////////////////////// getting entities //////////////////////////////////////////////

    const Entity* EntityChunk::getEntity(uint32_t id) const {
        // returns nullptr if there is no entity with that id

        if (id >= _entities.size())
            return nullptr;

        return &_entities.at(id);
    } 

    uint32_t EntityChunk::getEntityCount() const {

        return _entities.size();
    }

    const std::vector<Entity>& EntityChunk::getAllEntities() const {

        return _entities;
    }

    bool EntityChunk::getWasEdited() const {

        return _has_changed;
    }

    void EntityChunk::markAsUnEdited() {

        _has_changed = false;
    }

    ////////////////////////////////// loading the chunk from existing data //////////////////////////////////
    // will remove the current data in the chunks _entities buffer

    void EntityChunk::initFromData(const std::vector<Entity>& entities) {

        _entities = entities;
    }

} // namespace cell