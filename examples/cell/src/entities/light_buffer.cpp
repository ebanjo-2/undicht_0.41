#include "light_buffer.h"
#include "model_loading/obj/obj_file.h"
#include "file_tools.h"
#include "algorithm"
#include "debug.h"
#include "glm/gtc/type_ptr.hpp"

namespace cell {

    using namespace undicht;
    using namespace tools;

    const undicht::BufferLayout LIGHT_VERTEX_LAYOUT({UND_VEC3F}); // per vertex data
    const undicht::BufferLayout LIGHT_LAYOUT({UND_VEC3F, UND_VEC3F, UND_FLOAT32}); // per instance data

    void LightBuffer::init(const undicht::vulkan::LogicalDevice& device) {

        _point_light_buffer.init(device);

        MeshData sphere_data;
        OBJFile obj_file(UND_ENGINE_SOURCE_DIR + "examples/cell/res/entities/sphere.obj");
        obj_file.getMesh(sphere_data, 0, true, false, false);

        _point_light_buffer.setVertexData(sphere_data.vertices.data(), sphere_data.vertices.size() * sizeof(float), 0);

    }

    void LightBuffer::cleanUp() {

        _point_light_buffer.cleanUp();
    }


    uint32_t LightBuffer::addPointLight(const PointLight& light) {
        /// @brief add a point light to the buffer
        /// @return an unique id with which the light can be accessed

        // trying to reuse unused space in the _point_light_buffer
        std::vector<bool>::iterator it = std::find(_lights_in_use.begin(), _lights_in_use.end(), false);
        uint32_t id = it - _lights_in_use.begin();


        if(id == _lights_in_use.size()) {
            _lights_in_use.resize(id + 1);
            _point_light_buffer.setInstanceData(nullptr, 0, (id + 1) * 7 * sizeof(float) * 1.5); // reserving memory for future lights
        }

        _point_light_buffer.setInstanceData(glm::value_ptr(light._color), 3 * sizeof(float), 7 * sizeof(float) * id + 0);
        _point_light_buffer.setInstanceData(glm::value_ptr(light._pos), 3 * sizeof(float), 7 * sizeof(float) * id + 3 * sizeof(float));
        _point_light_buffer.setInstanceData(&light._intensity, 1 * sizeof(float), 7 * sizeof(float) * id + 6 * sizeof(float));
        _lights_in_use.at(id) = true;

        return id;
    }

    void LightBuffer::updatePointLight(uint32_t id, const PointLight& light) {

        if(id >= _lights_in_use.size()) {
            UND_ERROR << "failed to update point light: " << id << " because it doesnt exist\n";
            return;
        }

        _point_light_buffer.setInstanceData(glm::value_ptr(light._color), 3 * sizeof(float), 7 * sizeof(float) * id + 0);
        _point_light_buffer.setInstanceData(glm::value_ptr(light._pos), 3 * sizeof(float), 7 * sizeof(float) * id + 3 * sizeof(float));
        _point_light_buffer.setInstanceData(&light._intensity, 1 * sizeof(float), 7 * sizeof(float) * id + 6 * sizeof(float));
        _lights_in_use.at(id) = true;
    }

    void LightBuffer::freePointLight(uint32_t id) {

        if(id >= _lights_in_use.size()) {
            UND_ERROR << "failed to free point light: " << id << " because it doesnt exist\n";
            return;
        }

        _lights_in_use.at(id) = false;
    }

    uint32_t LightBuffer::getPointLightModelVertexCount() const {

        return _point_light_buffer.getVertexBuffer().getUsedSize() / LIGHT_VERTEX_LAYOUT.getTotalSize();
    }

    uint32_t LightBuffer::getPointLightCount() const {

        return _lights_in_use.size();
    }

    const undicht::vulkan::VertexBuffer& LightBuffer::getPointLightBuffer() const {

        return _point_light_buffer;
    }

} // cell