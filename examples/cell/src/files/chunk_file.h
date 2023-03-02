#ifndef CHUNK_FILE_H
#define CHUNK_FILE_H

#include <cstdlib>
#include "fstream"
#include "string"
#include "world/chunk_system/chunk.h"
#include "binary_data/binary_data_file.h"

namespace cell {

    class ChunkFile : protected undicht::tools::BinaryDataFile {
        // binary file that stores all kinds of chunks

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

    };

} // cell

#endif // CHUNK_FILE_H