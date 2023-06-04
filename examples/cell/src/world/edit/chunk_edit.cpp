#include "chunk_edit.h"
#include "debug.h"
#include "vector"

#include "math/math_tools.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
 

    ////////////////////////////////////////// place / remove cells from the chunk ////////////////////////////////////

    void ChunkEdit::add(CellChunk& chunk, const Cell& volume) {

        // subtract the volume from any cells that may exist at that position
        subtract(chunk, volume);

        chunk.addCell(volume);

    }

    void ChunkEdit::subtract(CellChunk& chunk, const Cell& volume) {

        // get the cells that will be effected by the subtraction
        std::vector<uint32_t> affected_cells = chunk.getCellIDsInVolume(volume);

        // subtract the volume from each of the cells
        for(uint32_t id : affected_cells) {

            subtractFromCell(chunk, id, volume);
        }

    }

    ////////////////////////////////////////// protected functions for ChunkEdit ///////////////////////////////////////

    void ChunkEdit::subtractFromCell(CellChunk& chunk, int cell_id, const Cell& c) {
        // edits the cell at the cell_id so that it doesnt overlap with c
        // the cell behind cell_id will be split into a maximum of 6 new cells

        // get the cell associated with the id
        const Cell* minuend_ptr = chunk.getCell(cell_id); // the one that volume is taken away from
        if(!minuend_ptr) return; // no cell at that id
        const uint32_t minuend_faces = minuend_ptr->getVisibleFaces();

        uint32_t material = minuend_ptr->getID();

        // calculate the overlapping volume between the two cells
        Cell shared_volume;
        if(!overlappingVolume(*minuend_ptr, c, shared_volume)) return;

        // get coords from the two cells
        uint8_t old_x0, old_x1, old_y0, old_y1, old_z0, old_z1; // coords of the old cell
        uint8_t new_x0, new_x1, new_y0, new_y1, new_z0, new_z1; // coords of the volume that gets subtracted
        minuend_ptr->getPos0(old_x0, old_y0, old_z0);
        minuend_ptr->getPos1(old_x1, old_y1, old_z1);
        shared_volume.getPos0(new_x0, new_y0, new_z0);
        shared_volume.getPos1(new_x1, new_y1, new_z1);

        // remove the old cell
        chunk.removeCell(cell_id);
        
        // calculate the new cells (some might have 0 volume)
        chunk.addCell(Cell(old_x0, old_y0, old_z0, new_x0, old_y1, old_z1, material, CELL_FACE_XP | (minuend_faces & (0xFF ^ CELL_FACE_XP)))); // -x
        chunk.addCell(Cell(new_x1, old_y0, old_z0, old_x1, old_y1, old_z1, material, CELL_FACE_XN | (minuend_faces & (0xFF ^ CELL_FACE_XN)))); // +x
        chunk.addCell(Cell(new_x0, old_y0, old_z0, new_x1, new_y0, old_z1, material, CELL_FACE_YP | (minuend_faces & (0xFF ^ CELL_FACE_YP)))); // -y
        chunk.addCell(Cell(new_x0, new_y1, old_z0, new_x1, old_y1, old_z1, material, CELL_FACE_YN | (minuend_faces & (0xFF ^ CELL_FACE_YN)))); // +y
        chunk.addCell(Cell(new_x0, new_y0, old_z0, new_x1, new_y1, new_z0, material, CELL_FACE_ZP | (minuend_faces & (0xFF ^ CELL_FACE_ZP)))); // -z
        chunk.addCell(Cell(new_x0, new_y0, new_z1, new_x1, new_y1, old_z1, material, CELL_FACE_ZN | (minuend_faces & (0xFF ^ CELL_FACE_ZN)))); // +z

    }

    bool ChunkEdit::overlappingVolume(const Cell& c0, const Cell& c1, Cell& volume) {
        /// calculates the volume shared by both cell
        /// @return false, if there is no shared volume

        // get positions from c0
        uint8_t c0_x0, c0_y0, c0_z0;
        uint8_t c0_x1, c0_y1, c0_z1;
        c0.getPos0(c0_x0, c0_y0, c0_z0);
        c0.getPos1(c0_x1, c0_y1, c0_z1);

        // get positions from c1
        uint8_t c1_x0, c1_y0, c1_z0;
        uint8_t c1_x1, c1_y1, c1_z1;
        c1.getPos0(c1_x0, c1_y0, c1_z0);
        c1.getPos1(c1_x1, c1_y1, c1_z1);

        // calc overlapping volume
        uint8_t vol_x0, vol_y0, vol_z0;
        uint8_t vol_x1, vol_y1, vol_z1;
        if(!overlappingRanges(c0_x0, c0_x1, c1_x0, c1_x1, vol_x0, vol_x1)) return false;
        if(!overlappingRanges(c0_y0, c0_y1, c1_y0, c1_y1, vol_y0, vol_y1)) return false;
        if(!overlappingRanges(c0_z0, c0_z1, c1_z0, c1_z1, vol_z0, vol_z1)) return false;

        // store the volume in the cell
        volume.setPos0(vol_x0, vol_y0, vol_z0);
        volume.setPos1(vol_x1, vol_y1, vol_z1);

        return true;
    }

} // cell