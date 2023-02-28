#include "binary_data_file.h"

namespace undicht {

    namespace tools {

        void BinaryDataFile::open(const std::string& file_name) {

        }

        void BinaryDataFile::close() {

        }

        ////////////////////////////////// protected BinaryDataFile functions ////////////////////////////////

        void BinaryDataFile::enlargeFile(size_t new_size) {

        }

        /////////////////////// functions that should make the block headers easier to use ///////////////////

        uint64_t BinaryDataFile::createBlockHeader(bool in_use, uint64_t block_size) const {
            
            return (uint64_t(in_use) << 63) + block_size; // avoiding bitwise or because it didnt really work (?)
        }

        bool BinaryDataFile::isInUse(uint64_t block_header) const {

            return block_header >> 63; // highest bit is the only one that remains
        }

        uint64_t BinaryDataFile::getBlockSize(uint64_t block_header) const {

            return (block_header << 1) >> 1; // effectivly remove the highest bit
        }

    }

} // undicht