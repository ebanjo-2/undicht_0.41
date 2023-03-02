#include "files/chunk_file.h"
#include "debug.h"

namespace cell {

    using namespace undicht;
    using namespace tools;

    ChunkFile::ChunkFile(const std::string& file_name) {

        open(file_name);
    }

    ChunkFile::~ChunkFile() {

        close();
    }

    bool ChunkFile::open(const std::string& file_name) {

        return BinaryDataFile::open(file_name);
    }

    void ChunkFile::close() {

        BinaryDataFile::close();
    }

    void ChunkFile::newChunkFile() {
        /// @brief erase the contents of the currently opened file / create a new file

        BinaryDataFile::newBinaryFile();
    }

    template<typename T>
    size_t ChunkFile::store(const Chunk<T>& chunk) {
        /// @return the location under which the now stored chunk can be found

        // get the data from the chunk
        size_t buffer_size = chunk.fillBuffer(nullptr);
        std::vector<char> buffer(buffer_size);
        chunk.fillBuffer(buffer.data());

        return BinaryDataFile::store(buffer.data(), buffer.size());
    }

    template<typename T>
    size_t ChunkFile::update(const Chunk<T>& chunk, size_t location) {
        /// @return might return a new location

        // get the data from the chunk
        size_t buffer_size = chunk.fillBuffer(nullptr);
        std::vector<char> buffer(buffer_size);
        chunk.fillBuffer(buffer.data());

        return BinaryDataFile::update(buffer.data(), buffer.size(), location);
    }

    void ChunkFile::free(size_t location) {
        /// @brief mark the location (and the chunk that was stored there) as unused, so that the space can be recycled

        BinaryDataFile::free(location);
    }

    template<typename T>
    bool ChunkFile::read(Chunk<T>& chunk, size_t location) {
        /// @return whether or not reading the chunk at the specified location was a success

        // read the chunk data
        std::vector<char> buffer;
        if(!BinaryDataFile::read(location, buffer))
            return false; // reading failed

        // store the data in the chunk
        chunk.loadFromBuffer(buffer.data(), buffer.size());

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