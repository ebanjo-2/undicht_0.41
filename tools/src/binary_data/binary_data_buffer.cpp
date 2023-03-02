#include "binary_data_buffer.h"
#include "algorithm"

namespace undicht {

    namespace tools {

        ////////////////////////// protected BinaryDataBuffer functions //////////////////////////

        BinaryDataBuffer::BufferEntry& BinaryDataBuffer::addBufferEntry(bool in_use, size_t offset, size_t byte_size) {

            BufferEntry entry;
            entry.is_in_use = in_use;
            entry.offset = offset;
            entry.byte_size = byte_size;

            _buffer_entries.push_back(entry);

            return _buffer_entries.back();
        }

        void BinaryDataBuffer::removeEmptyEntries() {
            /// @brief remove buffer entries with a byte_size of 0

            for(int i = _buffer_entries.size() - 1; i >= 0; i--) {
                if(_buffer_entries.at(i).byte_size == 0) {

                    _buffer_entries.erase(_buffer_entries.begin() + i);
                }
            }

        }

        void BinaryDataBuffer::sortBufferEntries() {
            /// @brief make sure the _buffer_entries are sorted by their offsets

            if(!_buffer_entries.size()) return;

            // sort the sections
            std::sort(_buffer_entries.begin(), _buffer_entries.end());
        }

        void BinaryDataBuffer::mergeUnusedEntries() {
            /// @brief will try to merge following entries marked as unused
            /// @warning entries have to be in a sorted state before calling this function !!!!

            // not using pointers since the _buffer_entries vector could shrink
            // and i am only 99.9% certain that the pointers will stay valid if that happens
            size_t unused_base = -1; 

            for(int i = 0; i < _buffer_entries.size(); i++) {

                if((!_buffer_entries.at(i).is_in_use) && (unused_base != -1)) {
                    // the current buffer entry can be merged with the "unused" entry
                    _buffer_entries.at(unused_base).byte_size = _buffer_entries.at(i).offset - _buffer_entries.at(unused_base).offset + _buffer_entries.at(i).byte_size; 
                    _buffer_entries.erase(_buffer_entries.begin() + i);
                    i--;
                } else if(!_buffer_entries.at(i).is_in_use){
                    // this entry could be used as a base for merging with the following entries
                    unused_base = i;
                } else {
                    // no merging with this entry possible, since it is in use
                    unused_base = -1;
                }

            }

        }

        BinaryDataBuffer::BufferEntry* BinaryDataBuffer::findBufferEntry(size_t location) const{
            /// @brief tries to find an entry with the specified offset
            /// @return will return nullptr if no such entry was found

            for(const BufferEntry& entry : _buffer_entries)
                if(entry.offset == location)
                    return (BufferEntry*)&entry;

            return nullptr;
        }

        BinaryDataBuffer::BufferEntry* BinaryDataBuffer::findUnusedMemory(size_t byte_size) const {
            /// @brief tries to find a entry that is marked as unused with specified size of at least byte_size
            /// @return will return nullptr if no entry with a large enough unused memory could be found

            for(const BufferEntry& entry : _buffer_entries)
                if((!entry.is_in_use) && (entry.byte_size <= byte_size))
                    return (BufferEntry*)&entry;

            return nullptr;
        }

    } // tools 

} // undicht