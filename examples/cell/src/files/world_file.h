#ifndef WORLD_FILE_H
#define WORLD_FILE_H

#include <string>
#include "world/world.h"
#include "world/chunk.h"
#include "xml/xml_file.h"

namespace cell {

    class WorldFile : protected undicht::tools::XmlFile {

      protected:

        std::string _file_path;
        std::string _file_name;
        std::string _world_file;

      public:

        WorldFile() = default;
        WorldFile(const std::string& file_name);

        /// @brief loads in the content stored in the world file
        /// @return true if the file exists and is a correct world file
        bool open(const std::string& file_name);

        void newWorldFile();

        // store / load single chunks
        /// @return true, if a chunk with the same chunk_pos existed before and is now overwritten
        bool write(const Chunk& chunk, const glm::ivec3& chunk_pos);
        /// @return true, if a chunk with the chunk_pos existed in the file and could be read
        bool read(Chunk& chunk, const glm::ivec3& chunk_pos);

      protected:
        // protected WorldFile functions

        std::string chunkPosToStr(const glm::ivec3& chunk_pos) const;
        glm::ivec3 strToChunkPos(std::string str) const;

        void writeChunkToFile(const std::string& file_name, const Chunk& chunk);
        bool readChunkFromFile(const std::string& file_name, Chunk& chunk);
    };

} // cell

#endif // WORLD_FILE_H