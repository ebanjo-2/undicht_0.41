#ifndef WORLD_FILE_H
#define WORLD_FILE_H

#include <string>
#include "world/cells/cell_world.h"
#include "world/cells/cell_chunk.h"
#include "world/lights/light_chunk.h"
#include "xml/xml_file.h"
#include "materials/material_atlas.h"
#include "environment/environment.h"
#include "files/chunk_file.h"
#include "core/vulkan/command_buffer.h"
#include "renderer/vulkan/transfer_buffer.h"

namespace cell {

    class WorldFile : protected undicht::tools::XmlFile {

      protected:

        std::string _file_path; // path to the file that is currently open
        std::string _file_name; // name of the current file (without the path)

        ChunkFile _chunk_file; // to store the binary data of all kinds of chunks

      public:

        WorldFile() = default;
        WorldFile(const std::string& file_name);

        /// @brief loads in the content stored in the world file
        /// @return true if the file exists and is a correct world file
        bool open(const std::string& file_name);

        void newWorldFile();

        // store / load single chunks

        /// @return true, if a chunk with the same chunk_pos existed before and is now overwritten
        bool write(const CellChunk& chunk, const glm::ivec3& chunk_pos);
        bool write(const LightChunk& chunk, const glm::ivec3& chunk_pos);

        /// @return true, if a chunk with the chunk_pos existed in the file and could be read
        bool read(CellChunk& chunk, const glm::ivec3& chunk_pos);
        bool read(LightChunk& chunk, const glm::ivec3& chunk_pos);

        // load other stuff from the file
        bool readMaterials(MaterialAtlas& atlas, undicht::vulkan::CommandBuffer& cmd, undicht::vulkan::TransferBuffer& buf);
        bool readEnvironment(Environment& env, undicht::vulkan::CommandBuffer& cmd, undicht::vulkan::TransferBuffer& buf);

      protected:
        // protected WorldFile functions

        template<typename T>
        bool write(const Chunk<T>& chunk, const glm::ivec3& chunk_pos, const std::string& world_name);
        
        template<typename T>
        bool read(Chunk<T>& chunk, const glm::ivec3& chunk_pos, const std::string& world_name);

        std::string chunkPosToStr(const glm::ivec3& chunk_pos) const;
        glm::ivec3 strToChunkPos(std::string str) const;

    };

} // cell

#endif // WORLD_FILE_H