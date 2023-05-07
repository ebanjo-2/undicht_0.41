#include "world_edit.h"
#include "debug.h"

namespace cell {

    using namespace undicht;

    void WorldEdit::place(CellWorld& world, const glm::ivec3& pos0, const glm::ivec3& pos1, uint32_t material) {
        /// @brief fills the volume defined by pos0 and pos1

        // get the chunks that are affected 
        std::vector<Chunk<Cell>*> chunks = world.getChunksAt(pos0, pos1);
        std::vector<glm::ivec3> positions = world.getChunkPositionsAt(pos0, pos1);

        // editing each of the chunks
        for(int i = 0; i < chunks.size(); i++) {
                
            // check if the chunk is loaded
            if(chunks.at(i) == nullptr) {
                UND_ERROR << "failed to place material: chunk not loaded\n";
                continue;
            }
            
            // calculate the volume that needs to be placed in the chunk
            Cell cell = calcVolumeInChunk(pos0, pos1, positions.at(i));
            cell.setID(material);

            // place the cell
            _chunk_edit.add(*(CellChunk*)chunks.at(i), cell);

        }

    }

    void WorldEdit::remove(CellWorld& world, const glm::ivec3& pos0, const glm::ivec3& pos1) {
        /// @brief removes all cells within the volume

        // get the chunks that are affected
        std::vector<Chunk<Cell>*> chunks = world.getChunksAt(pos0, pos1);
        std::vector<glm::ivec3> positions = world.getChunkPositionsAt(pos0, pos1);

        // editing each of the chunks
        for(int i = 0; i < chunks.size(); i++) {

            // check if the chunk is loaded
            if(chunks.at(i) == nullptr) {
                UND_ERROR << "failed to remove material: chunk not loaded\n";
                continue;
            }
            
            // calculate the volume that needs to be removed from the chunk
            Cell cell = calcVolumeInChunk(pos0, pos1, positions.at(i));

            // place the cell
            _chunk_edit.subtract(*(CellChunk*)chunks.at(i), cell);

        }
    }

    //////////////////////////////////// protected function of the WorldEdit class ////////////////////////////////////

    Cell WorldEdit::calcVolumeInChunk(const glm::ivec3& pos0, const glm::ivec3& pos1, const glm::ivec3& chunk_pos) {
        // calculates the part of the volume defined by pos0 and pos1
        // that is inside the chunk at the specified position

        glm::ivec3 cell_pos0 = glm::clamp(pos0 - chunk_pos, glm::ivec3(0), glm::ivec3(255));
        glm::ivec3 cell_pos1 = glm::clamp(pos1 - chunk_pos, glm::ivec3(0), glm::ivec3(255));

        return Cell(cell_pos0.x, cell_pos0.y, cell_pos0.z, cell_pos1.x, cell_pos1.y, cell_pos1.z);
    }

} // cell