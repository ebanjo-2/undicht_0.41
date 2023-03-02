#include "binary_data_file.h"
#include "debug.h"

#define BINARY_FILE_OPTIONS std::fstream::in | std::fstream::out | std::fstream::binary

namespace undicht {

    namespace tools {

        bool BinaryDataFile::open(const std::string& file_name) {

            _file_name = file_name;
            _file.open(_file_name, BINARY_FILE_OPTIONS);

            if(!_file.is_open())
                return false;
            
            // reading in the structure of the data stored in the file
            while(!_file.eof()) {

                // read block header
                uint64_t block_header;
                size_t offset = _file.tellg();
                _file.read((char*)&block_header, 8);
                if(_file.fail()) break;

                // create buffer entry
                BufferEntry entry;
                entry.byte_size = getBlockSize(block_header);
                entry.offset = offset;
                entry.is_in_use = isInUse(block_header);

                // move past current block
                _file.seekg(entry.offset + entry.byte_size);
                if(_file.fail()) break;

                // if the position after the block could be reached, the entry is seen as valid
                _buffer_entries.push_back(entry);

                // move to next properly aligned block header
                _file.seekg(nextBlockHeaderPos(_file.tellg()));
                if(_file.fail()) break;

            }

            return true;
        }

        void BinaryDataFile::close() {

            _file.close();
            _file_name.clear();
            _buffer_entries.clear();
        }


        void BinaryDataFile::newBinaryFile() {
            /// @brief if the opened file didnt exist before, it will be created
            /// if it existed, its contents will be "reset", as if it was a newly created binary file

            _file.close();
            _file.open(_file_name, BINARY_FILE_OPTIONS | std::fstream::trunc);
            _buffer_entries.clear();

        }


        uint64_t BinaryDataFile::store(char* data, size_t byte_size) {
            /// @brief store the data in the file
            /// @return the location at which the data was stored

            
            size_t total_size = byte_size + 8; // 8 additional bytes will be used for the block header

            // try finding unused memory
            BufferEntry* entry = findUnusedMemory(total_size);
            size_t write_location;

            if(entry) {
                // update the unused entry

                // marking the memory that is still unused
                size_t last_next_offset = nextBlockHeaderPos(entry->offset + entry->byte_size);
                size_t new_next_offset = nextBlockHeaderPos(entry->offset + total_size);

                if((last_next_offset - new_next_offset) > 0)
                    writeBlockHeader(addBufferEntry(false, new_next_offset, last_next_offset - new_next_offset));
                
                // storing the data in the unused block
                entry->is_in_use = true;
                entry->byte_size = total_size;
                writeBlockHeader(*entry);

                sortBufferEntries();
                write_location = entry->offset;

            } else {
                // create a block header to extent the file

                sortBufferEntries();
                write_location = _buffer_entries.size() ? _buffer_entries.back().offset + _buffer_entries.back().byte_size : 0;
                write_location = nextBlockHeaderPos(write_location);

                writeBlockHeader(addBufferEntry(true, write_location, total_size));
            }

            UND_LOG << "writing at location " << write_location << ", size: " << total_size << "\n";

            // write the data
            _file.write(data, byte_size);

            return write_location;
        }

        uint64_t BinaryDataFile::update(char* data, size_t byte_size, size_t location) {
            /// @brief stores the data in the file just as store() does,
            /// but marks the memory at the specified location as unused before that
            /// so maybe it gets reused

            free(location);
            return store(data, byte_size);
        }

        void BinaryDataFile::free(size_t location) {
            /// @brief marks the memory at the location as unused

            BufferEntry* entry = findBufferEntry(location);

            if(entry) {
                entry->is_in_use = false;
                writeBlockHeader(*entry);
            }

        }

        bool BinaryDataFile::read(size_t location, std::vector<char>& data) {
            /// @brief tries to read the block of data at the specified location
            /// @param location has to be a valid location at which a block header starts
            /// @param data the vector will be resized to fit all the data stored in the block
            /// @return whether or not reading from the location was a success

            // clear previous errors
            _file.clear();

            // find the buffer entry
            BufferEntry* entry = findBufferEntry(location);
            if(!entry) {
                UND_ERROR << "failed to read binary block at location " << location << ", no buffer entry for this location\n";
                return false;
            } 

            // move the read cursor to the position
            _file.seekg(location);
            if(_file.fail()) {
                UND_ERROR << "failed to read binary block at location " << location << ", cant reach read position\n";
                return false; // couldnt go to the position
            } 

            // read the block header
            uint64_t block_header;
            _file.read((char*)&block_header, 8);
            if(_file.fail() || (getBlockSize(block_header) != entry->byte_size)) {
                UND_ERROR << "failed to read binary block header at location / block header indicates wrong block size " << location << "\n";
                return false;
            }

            // read the data
            data.resize(entry->byte_size - 8); // not storing the block header
            _file.read(data.data(), data.size());
            if(_file.fail()) {
                UND_ERROR << "failed to read binary block data at location " << location << "\n";
                return false;
            }

            return true;
        }

        ////////////////////////////////// protected BinaryDataFile functions ////////////////////////////////

        void BinaryDataFile::writeBlockHeader(const BufferEntry& entry) {
            // write the entry as a block header to the file

            // try to go to the location specified by the entry
            _file.clear(); // clear previous errors
            _file.seekp(entry.offset);

            if(_file.fail()) // assume the reason it failed was that the file wasnt big enough
                enlargeFile(entry.offset);

            if(_file.fail())
                UND_ERROR << "failed to write binary file block header\n";

            // write the block header
            uint64_t block_header = createBlockHeader(entry.is_in_use, entry.byte_size);
            _file.write((char*)&block_header, 8);

        }

        void BinaryDataFile::enlargeFile(size_t new_size) {
            
            _file.close();
            _file.open(_file_name, BINARY_FILE_OPTIONS | std::fstream::ate | std::fstream::app);

            size_t file_size = _file.tellg();

            if(file_size < new_size) {
                std::vector<char> buffer(new_size - file_size);
                _file.write(buffer.data(), buffer.size());
            }

        }


        size_t BinaryDataFile::nextBlockHeaderPos(size_t position) const {
            // calculate the next properly aligned block header position
            // following the position given as a parameter

            return position + position % 8;
        }

        /////////////////////// functions that should make the block headers easier to use ///////////////////

        uint64_t BinaryDataFile::createBlockHeader(bool in_use, size_t block_size) const {
            
            return (uint64_t(in_use) << 63) + block_size; // avoiding bitwise or because it didnt really work (?)
        }

        bool BinaryDataFile::isInUse(uint64_t block_header) const {

            return block_header >> 63; // highest bit is the only one that remains
        }

        size_t BinaryDataFile::getBlockSize(uint64_t block_header) const {

            return (block_header << 1) >> 1; // effectivly remove the highest bit
        }

    }

} // undicht