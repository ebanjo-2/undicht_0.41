#include "chunk_buffer.h"
#include "vector"
#include "algorithm"
#include "debug.h"

#include "world/lights/light.h"
#include "world/cells/cell.h"
#include "renderer/vulkan/immediate_command.h"
#include "renderer/vulkan/transfer_buffer.h"

namespace cell {

    // to avoid linker errors
    // Tell the C++ compiler which instantiations to make while it is compiling the template class’s .cpp file.
    // https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl
    template class ChunkBuffer<Cell>;
    template class ChunkBuffer<Light>;

    template<typename T>
    void ChunkBuffer<T>::init(const undicht::vulkan::LogicalDevice& device) {

        _device_handle = device;

        _buffer.init(device);
    }

    template<typename T>
    void ChunkBuffer<T>::cleanUp() {

        _buffer.cleanUp();
    }

    //////////////////////////////// setting the base model //////////////////////////////////

    template<typename T>
    void ChunkBuffer<T>::setBaseModel(const std::vector<float>& vertices, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {
        
        _buffer.allocateVertexBuffer(vertices.size() * sizeof(float));
        _buffer.setVertexData(vertices.data(), vertices.size() * sizeof(float), 0, load_cmd, load_buf);

    }

    template<typename T>
    void ChunkBuffer<T>::setBaseModel(const char* vertices, uint32_t byte_size, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {
        
        _buffer.allocateVertexBuffer(byte_size);
        _buffer.setVertexData(vertices, byte_size, 0, load_cmd, load_buf);
    }

    //////////////////////////////// storing data in the buffer ///////////////////////////////

    template<typename T>
    void ChunkBuffer<T>::addChunk(const Chunk<T> &c, const glm::ivec3& chunk_pos, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {

        /*BufferEntry* entry = findBufferEntry(chunk_pos);
        if(entry != nullptr) {
            updateChunk(c, chunk_pos);
            return;
        }*/

        if(!c.fillBuffer(nullptr))
            return; // c has no data it wants to store

        // filling a buffer with the chunk data
        uint32_t chunk_buffer_size = c.fillBuffer(nullptr);
        std::vector<char> chunk_buffer(chunk_buffer_size);
        c.fillBuffer(chunk_buffer.data());

        // finding a suitable location in the vertex buffer for the data to be stored
        BufferEntry free_memory = findFreeMemory(chunk_buffer_size);

        // allocating memory if necessary
        if(free_memory.offset + free_memory.byte_size > _buffer.getInstanceBuffer().getAllocatedSize())
            _buffer.allocateInstanceBuffer((free_memory.offset + free_memory.byte_size) * 1.5f);

        // storing the data
        _buffer.setInstanceData(chunk_buffer.data(), chunk_buffer.size(), free_memory.offset, load_cmd, load_buf);

        // storing the buffer entry
        free_memory._chunk_pos = chunk_pos;
        _buffer_sections.push_back(free_memory);

        sortBufferEntries();
    }

    template<typename T>
    void ChunkBuffer<T>::updateChunk(const Chunk<T> &c, const glm::ivec3& chunk_pos, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {
        
        BufferEntry* entry = findBufferEntry(chunk_pos);
        if(entry == nullptr) {
            addChunk(c, chunk_pos, load_cmd, load_buf);
            return;
        }

        // there are definitly better solutions
        // removing and readding should "update" the chunk
        freeChunk(c, chunk_pos);
        addChunk(c, chunk_pos, load_cmd, load_buf);

    }

    template<typename T>
    void ChunkBuffer<T>::freeChunk(const Chunk<T> &c, const glm::ivec3& chunk_pos) {

        BufferEntry* entry = findBufferEntry(chunk_pos);
        if(entry != nullptr) {
            entry->offset = 0;
            entry->byte_size = 0;
        }

        // will remove the entry, because its byte_size is 0
        sortBufferEntries();
    }

    template<typename T>
    void ChunkBuffer<T>::allocate(uint32_t byte_size) {

        _buffer.allocateInstanceBuffer(byte_size, true);
    }

    template<typename T>
    uint32_t ChunkBuffer<T>::getAllocatedSize() {
        /// @return the size of allocated memory for chunks primitives

        return _buffer.getInstanceBuffer().getAllocatedSize();
    }

    /////////////////////////////// accessing the vertex buffer ///////////////////////////////

    template<typename T>
    const undicht::vulkan::VertexBuffer &ChunkBuffer<T>::getBuffer() const {

        return _buffer;
    }

    // getting the areas of the buffer that contain data that should be drawn
    template<typename T>
    const std::vector<typename ChunkBuffer<T>::BufferEntry>& ChunkBuffer<T>::getDrawAreas() const {

        return _buffer_sections;
    }

    /////////////////////////////// private ChunkBuffer functions ////////////////////////////

    template<typename T>
    void ChunkBuffer<T>::sortBufferEntries() {

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

    template<typename T>
    typename ChunkBuffer<T>::BufferEntry* ChunkBuffer<T>::findBufferEntry(const glm::ivec3& chunk_pos) {
        
        for(BufferEntry& entry : _buffer_sections) {
            
            // since this is an internal function
            // and only clearly defined chunk positions are expected,
            // i think its not necessary to check if the chunk_pos is within the chunk
            if(entry._chunk_pos == chunk_pos)
                return &entry;
        }

        return nullptr;
    }

    template<typename T>
    typename ChunkBuffer<T>::BufferEntry ChunkBuffer<T>::findFreeMemory(uint32_t byte_size) const {
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

} // cell