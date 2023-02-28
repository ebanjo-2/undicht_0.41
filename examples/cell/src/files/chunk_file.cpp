#include "files/chunk_file.h"
#include "debug.h"

namespace cell {

    const uint32_t CURRENT_CHUNK_VERSION = 0;

    ChunkFile::ChunkFile(const std::string& file_name) {

        open(file_name);
    }

    ChunkFile::~ChunkFile() {

        close();
    }

    bool ChunkFile::open(const std::string& file_name) {

        _file_name = file_name;

        // close any file that might have been open
        close();

        // open the file
        _file.open(file_name, std::fstream::binary | std::fstream::in | std::fstream::out);
            if(!_file.is_open()) return false;

        // read the file header
        std::string header_type; // should just contain "Chunk"
        uint32_t version;
        _file >> header_type >> version;

        if(header_type.compare("Chunk")) return false;
        if(version != CURRENT_CHUNK_VERSION) UND_WARNING << "Chunk file version is not the current one\n";

        // moving past the newline at the end of the header
        _file.get();

        // updating the header size
        _header_size = _file.tellg();

        // moving to the first properly aligned position for a chunk block
        _file.seekg(align(_file.tellg()));

        // reading the structure of the file
        while(!_file.eof()) {

            BufferEntry e;
            e.offset = _file.tellg();

            // read the size of the chunk block
            uint64_t chunk_size; // the chunk size is stored in a fixed size 8 byte long uint
            _file.read((char*)&chunk_size, 8);
            if(_file.eof() || _file.fail()) break; // no chunk size could be read

            // store the size of the chunk block
            e.byte_size = (chunk_size << 1) >> 1; // the first bit indicates whether or not the chunk is in use
            e.is_in_use = chunk_size >> 63; // for some reason bitwise operations (&) with uint64_t didnt really work

            // move past the chunk block
            _file.seekg(align(int64_t(e.offset + e.byte_size))); // just using the size_t produces some weird warning
            if(_file.fail()) break; // chunk cant be read properly

            _buffer_entries.push_back(e);

            UND_LOG << "found chunk at " << e.offset << " with a size of " << e.byte_size << " bytes, is in use: " << e.is_in_use << "\n";
        }
        
        return true;
    }

    void ChunkFile::close() {

        _buffer_entries.clear();
        _file.close();
    }

    void ChunkFile::newChunkFile() {
        /// @brief erase the contents of the currently opened file / create a new file

        // close any file that might have been open
        close();

        // open the file and clear its contents (trunc flag)
        _file.open(_file_name, std::fstream::trunc | std::fstream::binary | std::fstream::in | std::fstream::out);

        // write the chunk header
        _file << "Chunk" << " ";
        _file << CURRENT_CHUNK_VERSION << "\n";

        // updating the header size
        _header_size = _file.tellp();
    }

    template<typename T>
    size_t ChunkFile::store(const Chunk<T>& chunk) {
        /// @return the location under which the now stored chunk can be found

        size_t buffer_size = chunk.fillBuffer(nullptr);
        std::vector<char> buffer(buffer_size);
        chunk.fillBuffer(buffer.data());

        // will write the chunk header, the fstream write position will be right behind it
        BufferEntry entry = reserveMemory(buffer_size + 8); // 8 bytes for the chunk header 
        _file.write(buffer.data(), buffer_size);

        UND_LOG << "stored the chunk at " << entry.offset << ", size: " << entry.byte_size << "\n";

        return entry.offset;
    }

    template<typename T>
    size_t ChunkFile::update(const Chunk<T>& chunk, size_t location) {
        /// @return might return a new location

        BufferEntry* entry = findEntry(location);
        if(!entry) {
            UND_LOG << "failed to update chunk (chunk was not found)\n";
            return -1;
        }

        free(location);
        return store(chunk);
    }

    void ChunkFile::free(size_t location) {
        /// @brief mark the location (and the chunk that was stored there) as unused, so that the space can be recycled

        BufferEntry* entry = findEntry(location);
        if(!entry) {
            UND_LOG << "failed to free chunk (chunk was not found)\n";
            return;
        }

        entry->is_in_use = false;
    }

    template<typename T>
    bool ChunkFile::read(Chunk<T>& chunk, size_t location) {
        /// @return whether or not reading the chunk at the specified location was a success

        // clear previous error flags
        _file.clear();

        // go to the location of the chunk
        _file.seekg(location);
        if(_file.fail()) return false;

        // read the 8 bytes containing the length of the chunk
        uint64_t chunk_size;
        _file.read((char*)&chunk_size, 8);
        if(_file.fail()) return false;

        // extract the readable size and whether or not the chunk is in use
        size_t read_size = (chunk_size << 1) >> 1;
        bool in_use = chunk_size >> 63;
        if(!in_use) return false; // chunk data is not in use

        // read the chunk data
        std::vector<char> buffer(read_size);
        _file.read(buffer.data(), read_size);
        if(_file.fail()) return false;

        // store the data in the chunk
        chunk.loadFromBuffer(buffer.data(), read_size);

        return true;
    }

    //////////////////////////////// protected ChunkFile functions ////////////////////////////////

    int64_t ChunkFile::align(size_t position) const {
        // returns the next possible position for a chunk block to start
        return position + position % 8;
    }

    /// returns nullptr if no entry with the location was found
    ChunkFile::BufferEntry* ChunkFile::findEntry(size_t location) const {

        for(const BufferEntry& entry : _buffer_entries) {

            if(entry.offset == location)
                return (BufferEntry*)&entry;

        }

        return nullptr;
    }

    ChunkFile::BufferEntry ChunkFile::reserveMemory(size_t byte_size) {
        /// @brief reserve memory (use currently unused memory or append the file) to store a chunk of the specified size
        /// will also update the chunk header / buffer entry in the file (file write position will be right after the chunk header)

        for(int i = 0; i < _buffer_entries.size(); i++) {

            BufferEntry& entry = _buffer_entries.at(i);

            if(entry.is_in_use)
                continue;

            if(entry.byte_size >= byte_size) {
                // reuse unused entry
                
                // mark left over memory as unused
                if(align(entry.offset + entry.byte_size) - align(entry.offset + byte_size) > 8) {
                    BufferEntry unused;
                    unused.byte_size = entry.byte_size - byte_size;
                    unused.offset = entry.offset + byte_size;
                    unused.is_in_use = false;
                    writeBufferEntry(unused);
                    _buffer_entries.insert(_buffer_entries.begin() + i + 1, unused); // the order should stay correct
                }

                // return modified entry
                entry.byte_size = byte_size;
                entry.is_in_use = true; 
                writeBufferEntry(entry);

                return entry;
            }
        }

        // append file
        BufferEntry entry;
        if(_buffer_entries.size())
            entry.offset = align(_buffer_entries.back().offset + _buffer_entries.back().byte_size);
        else
            entry.offset = align(_header_size); // first entry of the file

        entry.byte_size = byte_size;
        entry.is_in_use = true;
        writeBufferEntry(entry);
        return entry;
    }

    bool ChunkFile::writeBufferEntry(const BufferEntry& entry) {
        // write a chunk header to the file

        uint64_t chunk_size = entry.is_in_use;
        chunk_size <<= 63; // placing the is_in_use bit as the highest bit
        chunk_size |= uint64_t(entry.byte_size);

        _file.clear();
        _file.seekp(entry.offset); // try to go to the position
        if(_file.fail()) {
            // the offset specified was propably behind the file end
            // extending the file
            _file.close();
            _file.open(_file_name, std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::ate); // fastest way to move to the end of the file
            
            UND_LOG << "file size: " << _file.tellp() << "\n";
            std::vector<char> buffer(entry.offset -  _file.tellp());
            _file.write(buffer.data(), buffer.size()); // write undefined chars until the specified offset is reached
        }

        _file.write((char*)&chunk_size, 8);

        return true;
    }

    ////////////////// explizit template instanziations for the template functions //////////////////
    class Cell;
    class Light;
    template size_t ChunkFile::store<Cell>(const Chunk<Cell>& chunk);
    template size_t ChunkFile::store<Light>(const Chunk<Light>& chunk);
    template size_t ChunkFile::update<Cell>(const Chunk<Cell>& chunk, size_t location);
    template size_t ChunkFile::update<Light>(const Chunk<Light>& chunk, size_t location);
    template bool ChunkFile::read<Cell>(Chunk<Cell>& chunk, size_t location);
    template bool ChunkFile::read<Light>(Chunk<Light>& chunk, size_t location);

}