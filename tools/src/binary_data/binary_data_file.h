#ifndef BINARY_DATA_FILE_H
#define BINARY_DATA_FILE_H

#include "cstdint"
#include "vector"
#include "fstream"
#include "string"
#include "binary_data/binary_data_buffer.h"

namespace undicht {

    namespace tools {

        class BinaryDataFile : public BinaryDataBuffer {
            /// a file that blindly stores any kind of data
            /// the data is stored in "blocks"
            /// a block begins with a 64 bit / 8 byte "header"
            /// the first bit of the header (highest bit of the 64 bit uint) indicates whether or not the following block is in use
            /// the other 63 bits of the header are the size of the block (including the 8 bytes from the header)
            /// after the header starts the data
            /// blocks can only start every 8 bytes (counting from the beginning of the file)

          protected:

            std::string _file_name;
            std::fstream _file; // used for reading and writing

          public:

            BinaryDataFile() = default;
            BinaryDataFile(const std::string& file_name);
            virtual~ BinaryDataFile();


            bool open(const std::string& file_name);
            void close();

            /// @brief if the opened file didnt exist before, it will be created
            /// if it existed, its contents will be "reset", as if it was a newly created binary file
            void newBinaryFile();

            /// @brief store the data in the file
            /// @return the location at which the data was stored
            uint64_t store(char* data, size_t byte_size);

            /// @brief stores the data in the file just as store() does,
            /// but marks the memory at the specified location as unused before that
            /// so maybe it gets reused
            uint64_t update(char* data, size_t byte_size, size_t location);

            /// @brief marks the memory at the location as unused
            void free(size_t location);

            /// @brief tries to read the block of data at the specified location
            /// @param location has to be a valid location at which a block header starts
            /// @param data the vector will be resized to fit all the data stored in the block
            /// @return whether or not reading from the location was a success
            bool read(size_t location, std::vector<char>& data);

          protected:
            // protected BinaryDataFile functions

            // write the entry as a block header to the file
            void writeBlockHeader(const BufferEntry& entry);

            // write enough empty data to get the file to the specified size 
            void enlargeFile(size_t new_size);

            // calculate the next properly aligned block header position
            // following the position given as a parameter
            size_t nextBlockHeaderPos(size_t position) const;

            // functions that should make the block headers easier to use
            uint64_t createBlockHeader(bool in_use, size_t block_size) const;
            bool isInUse(uint64_t block_header) const;
            size_t getBlockSize(uint64_t block_header) const;

        };

    } // tools

} // undicht

#endif // BINARY_DATA_FILE_H