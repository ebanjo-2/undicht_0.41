#include "world_file.h"
#include "fstream"
#include "debug.h"
#include "file_tools.h"

namespace cell {

    using namespace undicht;
    using namespace tools;

    const std::string CURRENT_WORLD_VERSION = "0.0.1";
    const uint32_t CURRENT_CHUNK_VERSION = 0;

    WorldFile::WorldFile(const std::string& file_name) {

        open(file_name);
    }


    bool WorldFile::open(const std::string& file_name) {
        /// @brief loads in the content stored in the world file
        /// @return true if the file exists and is a correct world file

        _file_path = getFilePath(file_name);
        _file_name = getFileName(file_name);
        _world_file = file_name;

        if(!XmlFile::open(file_name)) return false;

        // check if the file is a correct world file

        // root element tag name
        if(getName().compare("WORLD_FILE")) return false;
        
        // version attribute
        XmlTagAttrib* version = getAttribute("version");
        if(!version) return false;
        if(version->m_value.compare(CURRENT_WORLD_VERSION)) UND_WARNING << "world file version does not match the current version (version " << CURRENT_WORLD_VERSION << ")\n";

        // world element
        XmlElement* world = getElement({"WORLD"});
        if(!world) return false;

        return true;
    }

    void WorldFile::newWorldFile() {
        
        // root element
        setName("WORLD_FILE");
        addTagAttrib("version=" + CURRENT_WORLD_VERSION);

        // add a world element
        addChildElement("WORLD");

        // should create a new file if it doesnt exist already
        XmlFile::write(_world_file);
    }

    //////////////////////////////////////// store / load single chunks ///////////////////////////////////////////

    bool WorldFile::write(const Chunk& chunk, const glm::ivec3& chunk_pos) {
        /// @return true, if a chunk with the same chunk_pos existed before and is now overwritten

        std::string chunk_pos_str = chunkPosToStr(chunk_pos);

        // get world element
        XmlElement* world = getElement({"WORLD"});
        if(!world) return false;

        // looking for a chunk entry with the same chunk_pos
        XmlElement* c = world->getElement({"CHUNK chunk_pos=" + chunk_pos_str});
        if(!c) c = world->addChildElement("CHUNK", {"chunk_pos=" + chunk_pos_str}); // create an entry for the chunk
        
        // writing the chunk data to a file
        std::string chunk_file = chunk_pos_str + "_" + _file_name;
        writeChunkToFile(chunk_file, chunk);
        c->setContent(chunk_file);

        // write the changes to the world file
        XmlFile::write(_file_path + _file_name);

        return true;
    }

    bool WorldFile::read(Chunk& chunk, const glm::ivec3& chunk_pos) {
        /// @return true, if a chunk with the chunk_pos existed in the file and could be read

        std::string chunk_pos_str = chunkPosToStr(chunk_pos);

        // get world element
        XmlElement* world = getElement({"WORLD"});
        if(!world) return false;

        // looking for a chunk entry with the same chunk_pos
        XmlElement* c = world->getElement({"CHUNK chunk_pos=" + chunk_pos_str});
        if(!c) return false;

        // read the chunk from the specified chunk file
        return readChunkFromFile(_file_path + c->getContent(), chunk);
    }

    ////////////////////////////////////// protected WorldFile functions /////////////////////////////////////////

    std::string WorldFile::chunkPosToStr(const glm::ivec3& chunk_pos) const {

        return tools::toStr(chunk_pos.x) + "_" + tools::toStr(chunk_pos.y) + "_" + tools::toStr(chunk_pos.z);
    }

    glm::ivec3 WorldFile::strToChunkPos(std::string str) const {
        // expecting a string structured like this x_y_z

        glm::ivec3 pos;

        char* tmp = (char*)str.data();
        pos.x = std::strtol(tmp, &tmp, 0);
        pos.y = std::strtol(tmp + 1, &tmp, 0);
        pos.z = std::strtol(tmp + 1, &tmp, 0);

        return pos;
    }

    void WorldFile::writeChunkToFile(const std::string& file_name, const Chunk& chunk) {

        std::ofstream file(file_name, std::ios::binary | std::ios::out | std::ios::trunc);

        // writing the file "header"
        file << "Chunk ";
        file << CURRENT_CHUNK_VERSION << " "; // chunk file version
        file << chunk.getCellCount() << " ";
        file << sizeof(Cell) << "\n";

        // storing the cell data in a buffer
        size_t buffer_size = chunk.fillBuffer(nullptr);
        std::vector<char> buffer(buffer_size);
        chunk.fillBuffer(buffer.data());

        // writing the cell binary data
        file.write(buffer.data(), buffer.size());
        file.close();
    }

    bool WorldFile::readChunkFromFile(const std::string& file_name, Chunk& chunk) {

        std::fstream file(file_name, std::ios::binary | std::ios::in);

        if(!file.is_open()) return false;
        
        // reading the file "header"
        std::string header_type; // should just contain "Chunk"
        uint32_t version, cell_count, cell_size;
        file >> header_type >> version >> cell_count >> cell_size;

        // moving past the newline
        file.get();

        // reading the cell binary data
        std::vector<char> buffer(cell_count * cell_size);
        file.read(buffer.data(), buffer.size());
        file.close();

        // storing the cell data in the chunk
        chunk.initFromData((const Cell*)buffer.data(), buffer.size());

        return true;
    }

} // cell