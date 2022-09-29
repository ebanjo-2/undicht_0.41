#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include "core/vulkan/buffer.h"
#include "core/vulkan/logical_device.h"
#include "buffer_layout.h"


namespace undicht {

    namespace vulkan {

        class UniformBuffer {

        protected:

            Buffer _buffer;

            // there are special allignment rules for uniform buffers
            // offset for each type to follow these rules
            std::vector<uint32_t> _offsets;

        public:

            void init(const LogicalDevice& device, const BufferLayout& layout);
            void cleanUp();

            void setAttribute(uint32_t index, const void* data, uint32_t byte_size);

            const Buffer& getBuffer() const;

        protected:
            // protected helper functions

            // calculates the offsets for each of the types in the layout to 
            // comply to the alignment rules of uniform buffers
            std::vector<uint32_t> static calcOffsets(const BufferLayout& layout);

        };

    } // vulkan

} // undicht

#endif // UNIFORM_BUFFER_H