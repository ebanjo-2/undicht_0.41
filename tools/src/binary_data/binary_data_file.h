#ifndef BINARY_DATA_FILE_H
#define BINARY_DATA_FILE_H

#include "cstdint"
#include "vector"
#include "fstream"
#include "string"

namespace undicht {

    namespace tools {

        class BinaryDataFile {
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
            size_t _file_size;

          public:

            void open(const std::string& file_name);
            void close();

          protected:
            // protected BinaryDataFile functions

            void enlargeFile(size_t new_size);

            // functions that should make the block headers easier to use
            uint64_t createBlockHeader(bool in_use, uint64_t block_size) const;
            bool isInUse(uint64_t block_header) const;
            uint64_t getBlockSize(uint64_t block_header) const;

        };

    } // tools

} // undicht

#endif // BINARY_DATA_FILE_H