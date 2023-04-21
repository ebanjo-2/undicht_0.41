#include "chunk_edit.h"
#include "debug.h"
#include "vector"

#include "math/math_tools.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
 
    ////////////////////////////////// before editing a chunk, you have to load it ////////////////////////////////////

    void ChunkEdit::loadChunk(CellChunk& chunk) {
        // might do more in the future

        _chunk = &chunk; 
    }

    void ChunkEdit::unloadChunk() {

        _chunk = nullptr;
    }

    ////////////////////////////////////////// place / remove cells from the chunk ////////////////////////////////////

    void ChunkEdit::add(const Cell& volume) {

        if(!_chunk) { 
            UND_ERROR << "failed to edit chunk: no chunk loaded to the ChunkEdit class\n";
            return;
        }

    }

    void ChunkEdit::subtract(const Cell& volume) {

        if(!_chunk) { 
            UND_ERROR << "failed to edit chunk: no chunk loaded to the ChunkEdit class\n";
            return;
        }

        // get the cells that will be effected by the subtraction
        std::vector<uint32_t> affected_cells = _chunk->getCellIDsInVolume(volume);

    }

    ////////////////////////////////////////// protected functions for ChunkEdit ///////////////////////////////////////

    void ChunkEdit::subtractFromCell(int cell_id, const Cell& c) {
        // edits the cell at the cell_id so that it doesnt overlap with c
        // the cell behind cell_id will be split into a maximum of 6 new cells

        // get the cell associated with the id
        const Cell* minuend_ptr = _chunk->getCell(cell_id); // the one that volume is taken away from
        if(!minuend_ptr) return; // no cell at that id
        Cell minuend = *minuend_ptr; // cell that can be modified

        // calculate the overlapping volume between the two cells
        Cell shared_volume;
        if(!overlappingVolume(minuend, c, shared_volume)) return;

        // calculate the new cells


    }


    bool ChunkEdit::overlappingVolume(const Cell& c0, const Cell& c1, Cell& volume) {
        /// calculates the volume shared by both cell
        /// @return false, if there is no shared volume

        // get positions from c0
        uint32_t c0_x0, c0_y0, c0_z0;
        uint32_t c0_x1, c0_y1, c0_z1;
        c0.getPos0(c0_x0, c0_y0, c0_z0);
        c0.getPos1(c0_x1, c0_y1, c0_z1);

        // get positions from c1
        uint32_t c1_x0, c1_y0, c1_z0;
        uint32_t c1_x1, c1_y1, c1_z1;
        c1.getPos0(c1_x0, c1_y0, c1_z0);
        c1.getPos1(c1_x1, c1_y1, c1_z1);

        // calc overlapping volume
        uint32_t vol_x0, vol_y0, vol_z0;
        uint32_t vol_x1, vol_y1, vol_z1;
        if(!overlappingRanges(c0_x0, c0_x1, c1_x0, c1_x1, vol_x0, vol_x1)) return false;
        if(!overlappingRanges(c0_y0, c0_y1, c1_y0, c1_y1, vol_y0, vol_y1)) return false;
        if(!overlappingRanges(c0_z0, c0_z1, c1_z0, c1_z1, vol_z0, vol_z1)) return false;

        // store the volume in the cell
        volume.setPos0(vol_x0, vol_y0, vol_z0);
        volume.setPos1(vol_x1, vol_y1, vol_z1);

        return true;
    }

} // cell