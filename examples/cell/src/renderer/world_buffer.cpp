#include "world_buffer.h"
#include "vector"

namespace cell {

    using namespace undicht;

    // cube vertices
    const BufferLayout CUBE_VERTEX_LAYOUT({UND_VEC3F, UND_VEC2F, UND_VEC3F});

    // position, uv,    normal
    const std::vector<float> cube_vertices = {
        0,0,0,   0,0,   -1,0,0, // x = 0
        0,0,1,   1,0,   -1,0,0,
        0,1,1,   1,1,   -1,0,0,
        0,0,0,   0,0,   -1,0,0,
        0,1,1,   1,1,   -1,0,0,
        0,1,0,   0,1,   -1,0,0,

        1,0,0,   0,0,   1,0,0, // x = 1
        1,1,0,   0,1,   1,0,0,
        1,1,1,   1,1,   1,0,0,
        1,0,0,   0,0,   1,0,0,
        1,1,1,   1,1,   1,0,0,
        1,0,1,   1,0,   1,0,0,
        
        0,0,0,   0,0,   0,-1,0, // y = 0
        0,0,1,   0,1,   0,-1,0,
        1,0,1,   1,1,   0,-1,0,
        0,0,0,   0,0,   0,-1,0,
        1,0,1,   1,1,   0,-1,0,
        1,0,0,   1,0,   0,-1,0,

        0,1,0,   0,0,   0,1,0, // y = 1
        1,1,1,   1,1,   0,1,0,
        0,1,1,   0,1,   0,1,0,
        0,1,0,   0,0,   0,1,0,
        1,1,0,   1,0,   0,1,0,
        1,1,1,   1,1,   0,1,0,

        0,0,0,   0,0,   0,0,-1, // z = 0
        0,1,0,   0,1,   0,0,-1,
        1,1,0,   1,1,   0,0,-1,
        0,0,0,   0,0,   0,0,-1,
        1,1,0,   1,1,   0,0,-1,
        1,0,0,   1,0,   0,0,-1,

        0,0,1,   0,0,   0,0,1, // z = 1
        1,1,1,   1,1,   0,0,1,
        0,1,1,   0,1,   0,0,1,
        0,0,1,   0,0,   0,0,1,
        1,0,1,   1,0,   0,0,1,
        1,1,1,   1,1,   0,0,1,
    };
        
    WorldBuffer::WorldBuffer(const vulkan::LogicalDevice& device) {

        _buffer.init(device);
        _buffer.setVertexData(cube_vertices.data(), cube_vertices.size() * sizeof(float), 0);

    }

    WorldBuffer::~WorldBuffer() {

        _buffer.cleanUp();
    }

    //////////////////////////////// storing data in the buffer ///////////////////////////////

    void WorldBuffer::addChunk(const Chunk &c) {

        uint32_t chunk_buffer_size = c.fillBuffer(nullptr);
        std::vector<char> chunk_buffer(chunk_buffer_size);
        c.fillBuffer(chunk_buffer.data());

        _buffer.setInstanceData(chunk_buffer.data(), chunk_buffer.size(), 0);

    }

    void WorldBuffer::updateChunk(const Chunk &c) {
    }
    
    void WorldBuffer::freeChunk(const Chunk &c) {
    }

    /////////////////////////////// accessing the vertex buffer ///////////////////////////////

    const undicht::vulkan::VertexBuffer &WorldBuffer::getBuffer() const {

        return _buffer;
    }

} // namespace cell