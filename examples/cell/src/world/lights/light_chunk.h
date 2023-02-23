#ifndef LIGHT_CHUNK_H
#define LIGHT_CHUNK_H

#include "world/chunk_system/chunk.h"
#include "world/lights/light.h"
#include "vector"

namespace cell {

    class LightChunk : public Chunk<Light> {

      protected:
        
        std::vector<Light> _lights;

      public:

        /// @brief add a light to the buffer
        /// @return an unique id with which the light can be accessed
        uint32_t addLight(const Light& light);
        void updateLight(uint32_t id, const Light& light);
        void freeLight(uint32_t id);

        uint32_t fillBuffer(char* buffer) const; // store the contents of the chunk in the buffer (if buffer != nullptr), return size of elements stored
        void loadFromBuffer(const char* buffer, uint32_t byte_size); // initialize the complete data of the chunk from the buffer
        void loadFromBuffer(const std::vector<Light>& buffer);

    };

} // cell

#endif // LIGHT_CHUNK_H