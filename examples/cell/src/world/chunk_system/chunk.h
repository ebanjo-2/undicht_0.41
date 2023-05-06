#ifndef CHUNK_H
#define CHUNK_H

#include "cstdint"
#include "vector"

namespace cell {

    template<typename T>
    class Chunk {
      // base class for chunks that store all kinds of objects of type T
      // doesnt hold any information about position in world, managed by ChunkSystem class
      // covers the ranges from [0 to 255] for the x,y and z direction, so 255 units included per Axis

      protected:

        bool _has_changed = false;

      public:
        // common functions that should be implemented by all chunk classes

        virtual void markAsChanged(bool has_changed);
        virtual bool getHasChanged() const;

        virtual uint32_t fillBuffer(char* buffer) const = 0; // store the contents of the chunk in the buffer (if buffer != nullptr), return size of elements stored
        virtual void loadFromBuffer(const char* buffer, uint32_t byte_size) = 0; // initialize the complete data of the chunk from the buffer
        virtual void loadFromBuffer(const std::vector<T>& buffer) = 0;

    };

} // cell

#endif // CHUNK_H