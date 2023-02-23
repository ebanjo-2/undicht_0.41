#include "light_buffer.h"
#include "model_loading/obj/obj_file.h"
#include "file_tools.h"

namespace cell {

    using namespace undicht;
    using namespace tools;

    const undicht::BufferLayout LIGHT_VERTEX_LAYOUT({UND_VEC3F}); // per vertex data

    void LightBuffer::init(const undicht::vulkan::LogicalDevice& device) {

        ChunkBuffer<Light>::init(device);

        // load the point light model
        MeshData sphere_data;
        OBJFile obj_file(UND_ENGINE_SOURCE_DIR + "examples/cell/res/entities/sphere.obj");
        obj_file.getMesh(sphere_data, 0, true, false, false);

        setBaseModel(sphere_data.vertices);

    }

    uint32_t LightBuffer::getPointLightModelVertexCount() const {

        return _buffer.getVertexBuffer().getUsedSize() / LIGHT_VERTEX_LAYOUT.getTotalSize();
    }


} // cell