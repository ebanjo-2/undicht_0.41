#include "world_buffer.h"
#include "vector"
#include "algorithm"
#include "world/cell.h"

namespace cell {

    using namespace undicht;

    // cube vertices
    const BufferLayout CUBE_VERTEX_LAYOUT({UND_VEC3UI8, UND_UINT8});

    // position, FACE
    const std::vector<uint8_t> cube_vertices = {
        0,0,0,  CELL_FACE_XN, // x = 0
        0,0,1,  CELL_FACE_XN,
        0,1,1,  CELL_FACE_XN,
        0,0,0,  CELL_FACE_XN,
        0,1,1,  CELL_FACE_XN,
        0,1,0,  CELL_FACE_XN,

        1,0,0,  CELL_FACE_XP, // x = 1
        1,1,0,  CELL_FACE_XP,
        1,1,1,  CELL_FACE_XP,
        1,0,0,  CELL_FACE_XP,
        1,1,1,  CELL_FACE_XP,
        1,0,1,  CELL_FACE_XP,
        
        0,0,0,  CELL_FACE_YN, // y = 0
        1,0,1,  CELL_FACE_YN,
        0,0,1,  CELL_FACE_YN,
        0,0,0,  CELL_FACE_YN,
        1,0,0,  CELL_FACE_YN,
        1,0,1,  CELL_FACE_YN,

        0,1,0,  CELL_FACE_YP, // y = 1
        0,1,1,  CELL_FACE_YP,
        1,1,1,  CELL_FACE_YP,
        0,1,0,  CELL_FACE_YP,
        1,1,1,  CELL_FACE_YP,
        1,1,0,  CELL_FACE_YP,

        0,0,0,  CELL_FACE_ZN, // z = 0
        0,1,0,  CELL_FACE_ZN,
        1,1,0,  CELL_FACE_ZN,
        0,0,0,  CELL_FACE_ZN,
        1,1,0,  CELL_FACE_ZN,
        1,0,0,  CELL_FACE_ZN,

        0,0,1,  CELL_FACE_ZP, // z = 1
        1,1,1,  CELL_FACE_ZP,
        0,1,1,  CELL_FACE_ZP,
        0,0,1,  CELL_FACE_ZP,
        1,0,1,  CELL_FACE_ZP,
        1,1,1,  CELL_FACE_ZP,
    };
        
    void WorldBuffer::init(const undicht::vulkan::LogicalDevice& device) {

        _buffer.init(device);
        _buffer.setVertexData(cube_vertices.data(), cube_vertices.size() * sizeof(float), 0);
    }

    void WorldBuffer::cleanUp() {

        _buffer.cleanUp();
    }

    //////////////////////////////// storing data in the buffer ///////////////////////////////

    void WorldBuffer::addChunk(const Chunk &c, const glm::ivec3& chunk_pos) {

        BufferEntry* entry = findBufferEntry(chunk_pos);
        if(entry != nullptr) {
            updateChunk(c, chunk_pos);
            return;
        }

        // filling a buffer with the chunk data
        uint32_t chunk_buffer_size = c.fillBuffer(nullptr);
        std::vector<char> chunk_buffer(chunk_buffer_size);
        c.fillBuffer(chunk_buffer.data());

        // finding a suitable location in the vertex buffer for the data to be stored
        BufferEntry free_memory = findFreeMemory(chunk_buffer_size);

        // storing the data
        _buffer.setInstanceData(chunk_buffer.data(), chunk_buffer.size(), free_memory.offset);

        // storing the buffer entry
        free_memory._chunk_pos = chunk_pos;
        _buffer_sections.push_back(free_memory);

        sortBufferEntries();
    }

    void WorldBuffer::updateChunk(const Chunk &c, const glm::ivec3& chunk_pos) {
        
        BufferEntry* entry = findBufferEntry(chunk_pos);
        if(entry == nullptr) {
            addChunk(c, chunk_pos);
            return;
        }

        // there are definitly better solutions
        // removing and readding should "update" the chunk
        freeChunk(c, chunk_pos);
        addChunk(c, chunk_pos);

    }
    
    void WorldBuffer::freeChunk(const Chunk &c, const glm::ivec3& chunk_pos) {

        BufferEntry* entry = findBufferEntry(chunk_pos);
        if(entry != nullptr) {
            
            entry->offset = 0;
            entry->byte_size = 0;
        }

        sortBufferEntries();
    }

    /////////////////////////////// accessing the vertex buffer ///////////////////////////////

    const undicht::vulkan::VertexBuffer &WorldBuffer::getBuffer() const {

        return _buffer;
    }

    // getting the areas of the buffer that contain data that should be drawn
    const std::vector<WorldBuffer::BufferEntry>& WorldBuffer::getDrawAreas() const {

        return _buffer_sections;
    }

    /////////////////////////////// private WorldBuffer functions ////////////////////////////

    void WorldBuffer::sortBufferEntries() {

        if(!_buffer_sections.size())
            return;

        // removing sections without an actual size
        for(int i = _buffer_sections.size() - 1; i >= 0; i--) {
            if(_buffer_sections.at(i).byte_size == 0) {

                _buffer_sections.erase(_buffer_sections.begin() + i);
            }
        }

        // sorting the remaining sections
        std::sort(_buffer_sections.begin(), _buffer_sections.end());

    }

    WorldBuffer::BufferEntry* WorldBuffer::findBufferEntry(const glm::ivec3& chunk_pos) {
        
        for(BufferEntry& entry : _buffer_sections) {
            
            // since this is an internal function
            // and only clearly defined chunk positions are expected,
            // i think its not necessary to check if the chunk_pos is within the chunk
            if(entry._chunk_pos == chunk_pos)
                return &entry;
        }

        return nullptr;
    }

    WorldBuffer::BufferEntry WorldBuffer::findFreeMemory(uint32_t byte_size) const {
        // searches through the buffer trying to find a memory spot at least the size of byte_size
        // if no such spot can be found a section of memory will be declared that reaches outside the buffer
        // in which case new memory for the buffer needs to be allocated (the buffer needs to be resized)

        BufferEntry entry;
        entry.byte_size = byte_size;
        entry.offset = 0;

        // expecting the sections to be sorted
        for(int i = 0; i < _buffer_sections.size(); i++) {

            if(_buffer_sections.at(i).offset > entry.offset + entry.byte_size) {
                // found a gap between entries that is big enough
                break;
            } else {
                // moving past the entry
                entry.offset = _buffer_sections.at(i).offset + _buffer_sections.at(i).byte_size;
            }

        }

        return entry;
    }

} // namespace cell