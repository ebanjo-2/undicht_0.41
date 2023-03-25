#include "world/cells/cell_buffer.h"


namespace cell {

    using namespace undicht;

    // cube vertices
    const BufferLayout CUBE_VERTEX_LAYOUT({UND_VEC3UI8, UND_UINT8});

    // position, FACE
    const std::vector<uint8_t> cube_vertices = {
        0,1,0,  CELL_FACE_XN, // x = 0
        0,1,1,  CELL_FACE_XN,
        0,0,1,  CELL_FACE_XN,
        0,1,0,  CELL_FACE_XN,
        0,0,1,  CELL_FACE_XN,
        0,0,0,  CELL_FACE_XN,

        1,1,0,  CELL_FACE_XP, // x = 1
        1,0,0,  CELL_FACE_XP,
        1,0,1,  CELL_FACE_XP,
        1,1,0,  CELL_FACE_XP,
        1,0,1,  CELL_FACE_XP,
        1,1,1,  CELL_FACE_XP,
        
        0,0,0,  CELL_FACE_YN, // y = 0
        0,0,1,  CELL_FACE_YN,
        1,0,1,  CELL_FACE_YN,
        0,0,0,  CELL_FACE_YN,
        1,0,1,  CELL_FACE_YN,
        1,0,0,  CELL_FACE_YN,

        0,1,0,  CELL_FACE_YP, // y = 1
        1,1,1,  CELL_FACE_YP,
        0,1,1,  CELL_FACE_YP,
        0,1,0,  CELL_FACE_YP,
        1,1,0,  CELL_FACE_YP,
        1,1,1,  CELL_FACE_YP,

        0,1,0,  CELL_FACE_ZN, // z = 0
        0,0,0,  CELL_FACE_ZN,
        1,0,0,  CELL_FACE_ZN,
        0,1,0,  CELL_FACE_ZN,
        1,0,0,  CELL_FACE_ZN,
        1,1,0,  CELL_FACE_ZN,

        0,1,1,  CELL_FACE_ZP, // z = 1
        1,0,1,  CELL_FACE_ZP,
        0,0,1,  CELL_FACE_ZP,
        0,1,1,  CELL_FACE_ZP,
        1,1,1,  CELL_FACE_ZP,
        1,0,1,  CELL_FACE_ZP,
    };

    void CellBuffer::init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {

        ChunkBuffer<Cell>::init(device);
        setBaseModel((char*)cube_vertices.data(), cube_vertices.size() * sizeof(uint8_t), load_cmd, load_buf);

    }

    uint32_t CellBuffer::getCellModelVertexCount() const {

        return 36;
    }

} // namespace cel 
