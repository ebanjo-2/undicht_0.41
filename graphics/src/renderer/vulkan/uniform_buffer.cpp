#include "uniform_buffer.h"
#include "debug.h"

namespace undicht {

    namespace vulkan {

        void UniformBuffer::init(const LogicalDevice& device, const BufferLayout& layout) {

            // calculating the correct allignment for the attributes
            _offsets = calcOffsets(layout);

            // initializing the buffer
            _buffer.init(device.getDevice(), {device.getGraphicsQueueFamily()}, true, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
            _buffer.allocate(device, _offsets.back() + layout.m_types.back().getSize());

        }

        void UniformBuffer::cleanUp() {
            
            _buffer.cleanUp();

        }

        void UniformBuffer::setAttribute(uint32_t index, const void* data, uint32_t byte_size) {
            
            _buffer.setData(byte_size, _offsets.at(index), data);

        }

        const Buffer& UniformBuffer::getBuffer() const {

            return _buffer;
        }

        ///////////////////////////// protected helper functions /////////////////////////////

        std::vector<uint32_t> UniformBuffer::calcOffsets(const BufferLayout& layout) {
            // calculates the offsets for each of the types in the layout to
            // comply to the alignment rules of uniform buffers

            std::vector<uint32_t> offsets;

            uint32_t offset = 0;
            uint32_t last_size = 0;

            for(int i = 0; i < layout.m_types.size(); i++) {

                const FixedType& type = layout.getType(i);

                // moving past the last type
                offset += last_size;

                // calculating the correct offset for the current type
                uint32_t current_size = layout.getType(i).getSize();
                uint32_t alignment = current_size;

                // alignment rules taken from https://stackoverflow.com/questions/45638520/ubos-and-their-alignments-in-vulkan
                if(type.getNumComp() == 1) 
                    alignment = type.getCompSize(); // A scalar of size N has a base alignment of N.
                else if(type.getNumComp() == 2) 
                    alignment = 2 * type.getCompSize(); // A two-component vector, with components of size N, has a base alignment of 2 N.
                else if(type.getNumComp() == 3 || type.getNumComp() == 4)
                    alignment = 4 * type.getCompSize(); // A three- or four-component vector, with components of size N, has a base alignment of 4 N.
                else // for 3*3 and 4*4 matrices this should work
                    alignment = 16;

                //alignment = std::max(alignment, 4u); // nothing smaller than 4 bytes
                //alignment = std::min(alignment, 16u); // types bigger than 16 bytes still have to be aligned as if they were 16 bytes

                if(offset % alignment)
                    offset += alignment - (offset % alignment);

                offsets.push_back(offset);

                last_size = current_size;
            }

            return offsets;
        }

    } // vulkan

} // undicht