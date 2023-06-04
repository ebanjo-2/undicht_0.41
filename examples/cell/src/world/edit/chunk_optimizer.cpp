#include <world/edit/chunk_optimizer.h>

namespace cell {

    void optimizeChunk(CellChunk old, CellChunk* optimized){
        /** tries to minimize the number of cells in the chunk */

        // creating the buffers used by the ChunkOptimizer on the stack
        // seams to be to much, that why it gets created with new

        ChunkOptimizer* o = new ChunkOptimizer;
        o->optimizeChunk(old, optimized);
        delete o;

    }

    void ChunkOptimizer::optimizeChunk(CellChunk old_chunk, CellChunk* optimized) {
        /** tries to minimize the number of cells in the chunk */

        CellChunk& chunk = *optimized;

        loadChunk(old_chunk);

        for(int x = 0; x < 255; x++) {

            for(int y = 0; y < 255; y++) {

                for(int z = 0; z < 255; z++) {

                    if(!_cell_set_buffer[x * 255 * 255 + y * 255 + z]) {

                        Cell c = findCell(glm::ivec3(x,y,z));
                        c.setVisibleFaces(calcVisibleFaces(c));

                        if(c.getID() != VOID_CELL)
                            chunk.addCell(c);

                        markCellAsSet(c);
                        
                        // moving the z coordinate of where to look next
                        uint8_t tmp_x, tmp_y, tmp_z;
                        c.getPos1(tmp_x, tmp_y, tmp_z);

                        z = tmp_z - 1;

                    }
                }

            }

        }

    }

    void ChunkOptimizer::loadChunk(const CellChunk& chunk) {

        std::fill(_cell_mat_buffer.begin(), _cell_mat_buffer.end(), VOID_CELL);

        for(const Cell& c : chunk.getAllCells()) {

            uint8_t tmp_x, tmp_y, tmp_z;

            c.getPos0(tmp_x, tmp_y, tmp_z);
            glm::ivec3 pos0 = glm::ivec3(tmp_x, tmp_y, tmp_z);

            c.getPos1(tmp_x, tmp_y, tmp_z);
            glm::ivec3 pos1 = glm::ivec3(tmp_x, tmp_y, tmp_z);

            for(int x = pos0.x; x < pos1.x; x++) {

                for(int y = pos0.y; y < pos1.y; y++) {

                    std::array<unsigned short, 255 * 255 * 255>::iterator i1 = _cell_mat_buffer.begin() + (x * 255 * 255) + (y * 255) + pos0.z;
                    std::array<unsigned short, 255 * 255 * 255>::iterator i2 = _cell_mat_buffer.begin() + (x * 255 * 255) + (y * 255) + pos1.z;


                    std::fill(i1, i2, c.getID());

                }

            }

        }

        std::fill(_cell_set_buffer.begin(), _cell_set_buffer.end(), false);

    }

    /*Cell findCell(const u8vec3& pos0) {

        bool search_x = true;
        bool search_y = true;
        bool search_z = true;

        u8vec3 pos1 = pos0 + u8vec3(1,1,1);

        unsigned short mat = cell_mat_buffer[pos0.x * 255 * 255 + pos0.y * 255 + pos0.z];

        while(search_x || search_y || search_z) {

            if(search_x && (pos1.x != 255)) {
                // searching in +x direction
                if(sameMaterial(u8vec3(pos1.x, pos0.y, pos0.z), u8vec3(pos1.x + 1, pos1.y, pos1.z), mat)) {

                    pos1.x += 1;
                } else {

                    search_x = false;
                }

            } else {

                search_x = false;
            }

            if(search_y && (pos1.y != 255)) {
                // searching in +y direction
                if(sameMaterial(u8vec3(pos0.x, pos1.y, pos0.z), u8vec3(pos1.x, pos1.y + 1, pos1.z), mat)) {

                    pos1.y += 1;
                } else {

                    search_y = false;
                }

            } else {

                search_y = false;
            }

            if(search_z && (pos1.z != 255)) {
                // searching in +z direction
                if(sameMaterial(u8vec3(pos0.x, pos0.y, pos1.z), u8vec3(pos1.x, pos1.y, pos1.z + 1), mat)) {

                    pos1.z += 1;
                } else {

                    search_z = false;
                }

            } else {

                search_z = false;
            }

        }

        return Cell(pos0, pos1, mat);
    }*/

	Cell ChunkOptimizer::findCell(const glm::ivec3& p0) {

		glm::ivec3 p1 = p0 + glm::ivec3(1);

		unsigned short mat = _cell_mat_buffer[p0.x * 255 * 255 + p0.y * 255 + p0.z];
		bool has_visible_faces = containsVoid(p0 - glm::ivec3(1,1,1), p1 + glm::ivec3(1, 1, 1));

		const glm::ivec3 directions[] = {
			glm::ivec3( 0, 1, 0),
			glm::ivec3( 1, 0, 0),
			glm::ivec3( 0, 0, 1),
		};

		unsigned char left_search_directions = CELL_FACE_YP | CELL_FACE_XP | CELL_FACE_ZP;

		while (left_search_directions) {

			for (int i = 0; i < 3; i++) {

				unsigned char current_dir = 0x01 << (i * 2);

				if (current_dir & left_search_directions) {

					glm::ivec3 test_pos0 = p1 * directions[i] + p0 * (1 - directions[i]);
					glm::ivec3 test_pos1 = p1 + directions[i];

					if (test_pos1.x > 255 || test_pos1.y > 255 || test_pos1.z > 255) {

						left_search_directions -= current_dir;
						continue;
					}

					glm::ivec3 visible_check0 = test_pos0 - (1 - directions[i]);
					glm::ivec3 visible_check1 = test_pos1 + 1;

					if ((mat != VOID_CELL) && (containsVoid(visible_check0, visible_check1) != has_visible_faces)) {

						left_search_directions -= current_dir;
						continue;
					}

					if (sameMaterial(glm::ivec3(test_pos0), glm::ivec3(test_pos1), mat)) {

						p1 += directions[i];
					} else {

						left_search_directions -= current_dir;
					}

				}

			}

		}

		return Cell(p0.x, p0.y, p0.z, p1.x, p1.y, p1.z, mat);
	}

    unsigned char ChunkOptimizer::calcVisibleFaces(const Cell& c) {

        if(c.getID() == VOID_CELL) return 0x00; // void cell

        uint8_t tmp_x, tmp_y, tmp_z;

        c.getPos0(tmp_x, tmp_y, tmp_z);
        glm::ivec3 pos0 = glm::ivec3(tmp_x, tmp_y, tmp_z);

        c.getPos1(tmp_x, tmp_y, tmp_z);
        glm::ivec3 pos1 = glm::ivec3(tmp_x, tmp_y, tmp_z);

        // vertices of all the faces along which the neighbouring cells need to be checked
        std::array<glm::ivec3, 12> faces = {
            glm::ivec3(pos0.x, pos1.y + 0, pos0.z), glm::ivec3(pos1.x, pos1.y + 1, pos1.z), // +y
            glm::ivec3(pos0.x, pos0.y - 1, pos0.z), glm::ivec3(pos1.x, pos0.y - 0, pos1.z), // -y
            glm::ivec3(pos1.x + 0, pos0.y, pos0.z), glm::ivec3(pos1.x + 1, pos1.y, pos1.z), // +x
            glm::ivec3(pos0.x - 1, pos0.y, pos0.z), glm::ivec3(pos0.x - 0, pos1.y, pos1.z), // -x
            glm::ivec3(pos0.x, pos0.y, pos1.z + 0), glm::ivec3(pos1.x, pos1.y, pos1.z + 1), // +z
            glm::ivec3(pos0.x, pos0.y, pos0.z - 1), glm::ivec3(pos1.x, pos1.y, pos0.z - 0), // -z
        };

        unsigned char visible_faces = 0x00;

        for(int i = 0; i < 6; i++) {

            if(containsVoid(faces[i * 2 + 0], faces[i * 2 + 1]))
                visible_faces |= (1 << i); // make the face visible

        }

        return visible_faces;
    }


    void ChunkOptimizer::markCellAsSet(const Cell& c) {

        uint8_t tmp_x, tmp_y, tmp_z;

        c.getPos0(tmp_x, tmp_y, tmp_z);
        glm::ivec3 pos0 = glm::ivec3(tmp_x, tmp_y, tmp_z);

        c.getPos1(tmp_x, tmp_y, tmp_z);
        glm::ivec3 pos1 = glm::ivec3(tmp_x, tmp_y, tmp_z);

        for(int x = pos0.x; x < pos1.x; x++) {

            for(int y = pos0.y; y < pos1.y; y++) {

                std::array<bool, 255 * 255 * 255>::iterator i1 = _cell_set_buffer.begin() + (x * 255 * 255) + (y * 255) + pos0.z;
                std::array<bool, 255 * 255 * 255>::iterator i2 = _cell_set_buffer.begin() + (x * 255 * 255) + (y * 255) + pos1.z;


                std::fill(i1, i2, true);

            }

        }
    }


    bool ChunkOptimizer::sameMaterial(const glm::ivec3& pos0, const glm::ivec3& pos1, unsigned short mat) {
        /** if the volume is completely filled with that material */

        for(int x = pos0.x; x < pos1.x; x++) {

            for(int y = pos0.y; y < pos1.y; y++) {

                for(int z = pos0.z; z < pos1.z; z++) {

                    if(_cell_mat_buffer[x * 255 * 255 + y * 255 + z] != mat)
                        return false;

                    if(_cell_set_buffer[x * 255 * 255 + y * 255 + z])
                        return false;
                }
            }
        }

        return true;
    }


    bool ChunkOptimizer::containsVoid(const glm::ivec3& pos0, const glm::ivec3& pos1) {

        if(pos0.x < 0 || pos0.y < 0 || pos0.z < 0) return true;
        if(pos1.x > 255 || pos1.y > 255 || pos1.z > 255) return true;


        for(int x = pos0.x; x < pos1.x; x++) {

            for(int y = pos0.y; y < pos1.y; y++) {

                for(int z = pos0.z; z < pos1.z; z++) {

                    if(_cell_mat_buffer[x * 255 * 255 + y * 255 + z] == VOID_CELL)
                        return true;

                }
            }
        }

        return false;
    }


} // cell