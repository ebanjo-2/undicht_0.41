#ifndef BINARY_DATA_BUFFER_H
#define BINARY_DATA_BUFFER_H

#include "cstdlib" // size_t
#include "vector"

namespace undicht {

    namespace tools {

        class BinaryDataBuffer {
            // a base class for classes that want to store data in a binary buffer
            // and want to keep track of where things are in that buffer
          public:
            
            struct BufferEntry {
                // stores the location and offset of data stored in the buffer
                bool is_in_use = true;
                size_t offset = 0;
                size_t byte_size = 0;

                bool operator < (const BufferEntry& other) const {
                    return offset < other.offset;
                }
                bool operator == (size_t location) const {
                    return offset == location;
                }
            };

          protected:

            std::vector<BufferEntry> _buffer_entries;

          public:
          
          protected:
            // protected BinaryDataBuffer functions

            /// @brief remove buffer entries with a byte_size of 0 
            virtual void removeEmptyEntries();

            /// @brief make sure the _buffer_entries are sorted by their offsets
            virtual void sortBufferEntries();

            /// @brief will try to merge following entries marked as unused
            /// @warning entries have to be in a sorted state before calling this function !!!!
            virtual void mergeUnusedEntries();
            
            /// @brief tries to find an entry with the specified offset
            /// @return will return nullptr if no such entry was found
            virtual BufferEntry* findBufferEntry(size_t location) const;

            /// @brief tries to find a entry that is marked as unused with specified size of at least byte_size
            /// @return will return nullptr if no entry with a large enough unused memory could be found
            virtual BufferEntry* findUnusedMemory(size_t byte_size) const;

        };

    } // tools

} // undicht

#endif // BINARY_DATA_BUFFER_H