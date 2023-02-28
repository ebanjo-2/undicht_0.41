#ifndef CHUNK_FILE_H
#define CHUNK_FILE_H

#include <cstdlib>
#include "fstream"
#include "string"
#include "world/chunk_system/chunk.h"

namespace cell {

    class ChunkFile {
        // binary file that stores all kinds of chunks
        // the file has a text based header, which starts with the word "Chunk", after that comes the version (uint)
        // the file header ends with a newline character (\n)
        // after that comes the binary data
        // the chunks are stored wherever they fit, in an attempt to minimize having to resize the file
        // the location to access the chunks is not stored in the file, it has to be stored externally (in the world file)
        // there is a "chunk header" in front of each raw chunk data block (8 bytes containing the size of the following chunk block in bytes)
        // the highest bit of the "chunk header" does not count into the size of the chunk, but is instead used to indicate whether the following block is in use (bit set to 1) or not
        // empty areas in the file also have this "chunk header", but they dont contain anything useful
        // the beginning of chunk data blocks has to be alignened to a multiple of 8 bytes from the beginning of the file
      protected:

        struct BufferEntry {
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

        std::vector<BufferEntry> _buffer_entries;

        std::fstream _file;
        std::string _file_name;

        size_t _header_size = 0; // size of the file header in bytes

      public:

        ChunkFile() = default;
        ChunkFile(const std::string& file_name);
        virtual ~ChunkFile();

        bool open(const std::string& file_name);
        void close();

        /// @brief erase the contents of the currently opened file / create a new file
        void newChunkFile();

        /// @return the location under which the now stored chunk can be found
        template<typename T>
        size_t store(const Chunk<T>& chunk);

        /// @return might return a new location
        template<typename T> 
        size_t update(const Chunk<T>& chunk, size_t location);

        /// @brief mark the location (and the chunk that was stored there) as unused, so that the space can be recycled
        void free(size_t location);

        /// @return whether or not reading the chunk at the specified location was a success
        template<typename T>
        bool read(Chunk<T>& chunk, size_t location);

      protected:
        // protected ChunkFile functions

        // returns the next possible position for a chunk block to start
        int64_t align(size_t position) const;

        /// returns nullptr if no entry with the location was found
        BufferEntry* findEntry(size_t location) const;

        /// @brief reserve memory (use currently unused memory or append the file) to store a chunk of the specified size
        /// will also update the chunk header / buffer entry in the file (file write position will be right after the chunk header)
        BufferEntry reserveMemory(size_t byte_size);

        // write a chunk header to the file
        bool writeBufferEntry(const BufferEntry& entry);


    };

} // cell

#endif // CHUNK_FILE_H