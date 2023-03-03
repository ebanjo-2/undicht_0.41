#include "world_file.h"
#include "fstream"
#include "debug.h"
#include "file_tools.h"
#include "material_file.h"

namespace cell {

    using namespace undicht;
    using namespace tools;

    const std::string CURRENT_WORLD_VERSION = "0.0.2";

    WorldFile::WorldFile(const std::string& file_name) {

        open(file_name);
    }

    bool WorldFile::open(const std::string& file_name) {
        /// @brief loads in the content stored in the world file
        /// @return true if the file exists and is a correct world file

        _file_path = getFilePath(file_name);
        _file_name = getFileName(file_name);

        if(!XmlFile::open(file_name)) return false;
        if(!_chunk_file.open(_file_path + getFileName(file_name, true) + ".chunk")) return false;

        // check if the file is a correct world file

        // root element tag name
        if(getName().compare("WORLD_FILE")) return false;
        
        // version attribute
        XmlTagAttrib* version = getAttribute("version");
        if(!version) return false;
        if(version->m_value.compare(CURRENT_WORLD_VERSION)) UND_WARNING << "world file version does not match the current version (version " << CURRENT_WORLD_VERSION << ")\n";

        // expected elements
        if(!getElement({"WORLD"})) return false;
        if(!getElement({"LIGHTS"})) return false;
        if(!getElement({"MATERIALS"})) return false;
        if(!getElement({"ENVIRONMENT"})) return false;

        return true;
    }

    void WorldFile::newWorldFile() {
        
        // root element
        setName("WORLD_FILE");
        addTagAttrib("version=" + CURRENT_WORLD_VERSION);

        // add a world element
        addChildElement("WORLD");
        addChildElement("LIGHTS");
        addChildElement("MATERIALS");
        addChildElement("ENVIRONMENT");

        // should create a new file if it doesnt exist already
        XmlFile::write(_file_path + _file_name);

        // new chunk file
        _chunk_file.newChunkFile();
    }

    //////////////////////////////////////// store / load single chunks ///////////////////////////////////////////

    bool WorldFile::write(const CellChunk& chunk, const glm::ivec3& chunk_pos) {
        /// @return true, if a chunk with the same chunk_pos existed before and is now overwritten

        return write<Cell>(chunk, chunk_pos, "WORLD");
    }

    bool WorldFile::write(const LightChunk& chunk, const glm::ivec3& chunk_pos) {
        /// @return true, if a chunk with the same chunk_pos existed before and is now overwritten

        return write<Light>(chunk, chunk_pos, "LIGHTS");
    }

    bool WorldFile::read(CellChunk& chunk, const glm::ivec3& chunk_pos) {
        /// @return true, if a chunk with the chunk_pos existed in the file and could be read

        return read<Cell>(chunk, chunk_pos, "WORLD");
    }

    bool WorldFile::read(LightChunk& chunk, const glm::ivec3& chunk_pos) {
        /// @return true, if a chunk with the chunk_pos existed in the file and could be read

        return read<Light>(chunk, chunk_pos, "LIGHTS");
    }

    ///////////////////////////////////// load other stuff from the file////////////////////////////////////////

    bool WorldFile::readMaterials(MaterialAtlas& atlas) {
        
        XmlElement* materials = getElement({"MATERIALS"});
        if(!materials) return false;

        std::vector<XmlElement*> sources = materials->getAllElements({"SOURCE"});

        // load all source files
        std::vector<Material> all_materials;
        for(XmlElement* source : sources) {

            MaterialFile file(_file_path + source->getContent());

            if(!file.loadAllMaterials(all_materials))
                return false;
        }

        // store the materials in the atlas
        for(Material& m : all_materials)
            atlas.setMaterial(m);

        UND_LOG << "loaded " << all_materials.size() << " materials\n";

        return true;
    }

    bool WorldFile::readEnvironment(Environment& env) {

        XmlElement* materials = getElement({"ENVIRONMENT"});
        if(!materials) return false;

        XmlElement* source = materials->getElement({"SOURCE"});

        if(source) {

            env.load(_file_path + source->getContent());
        }

        return true;
    }


    ////////////////////////////////////// protected WorldFile functions /////////////////////////////////////////

    template<typename T>
    bool WorldFile::write(const Chunk<T>& chunk, const glm::ivec3& chunk_pos, const std::string& world_name) {
        /// @return true, if a chunk with the same chunk_pos existed before and is now overwritten

        std::string chunk_pos_str = chunkPosToStr(chunk_pos);

        // get world element
        XmlElement* world = getElement({world_name});
        if(!world) return false;

        // looking for a chunk entry with the same chunk_pos
        XmlElement* c = world->getElement({"CHUNK chunk_pos=" + chunk_pos_str});
        size_t store_location;

        if(c) {
            // update existing chunk
            size_t old_location = std::strtol(c->getContent().data(), nullptr, 10);
            store_location = _chunk_file.update(chunk, old_location);
        } else {
            // new chunk
            c = world->addChildElement("CHUNK", {"chunk_pos=" + chunk_pos_str}); // create an entry for the chunk
            store_location = _chunk_file.store(chunk);
        }
        
        // storing the location of the chunk in the chunk file
        c->setContent(toStr(store_location));

        // write the changes to the world file
        XmlFile::write(_file_path + _file_name);

        return true;
    }
        
    template<typename T>
    bool WorldFile::read(Chunk<T>& chunk, const glm::ivec3& chunk_pos, const std::string& world_name) {
        /// @return true, if a chunk with the chunk_pos existed in the file and could be read

        std::string chunk_pos_str = chunkPosToStr(chunk_pos);

        // get world element
        XmlElement* world = getElement({world_name});
        if(!world) return false;

        // looking for a chunk entry with the same chunk_pos
        XmlElement* c = world->getElement({"CHUNK chunk_pos=" + chunk_pos_str});
        if(!c) return false;

        // read the chunk from the chunk file
        size_t store_location = std::strtol(c->getContent().data(), nullptr, 10);
        return _chunk_file.read(chunk, store_location);
    }

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

} // cell