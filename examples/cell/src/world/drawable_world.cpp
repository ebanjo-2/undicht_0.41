#include "world/drawable_world.h"
#include "debug.h"
#include "glm/gtx/string_cast.hpp"
#include "algorithm"
#include "core/vulkan/command_buffer.h"
#include "renderer/vulkan/transfer_buffer.h"

namespace cell {

    void DrawableWorld::init(const undicht::vulkan::LogicalDevice& device, undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {
        
        _sun.setType(Light::Type::Directional);
        _cell_world.init(device, load_cmd, load_buf);
        _light_world.init(device, load_cmd, load_buf);
        _materials.init(device);
        _environment.init(device);
        
    }

    void DrawableWorld::cleanUp() {

        _cell_world.cleanUp();
        _light_world.cleanUp();
        _materials.cleanUp();
        _environment.cleanUp();
    }

    ///////////////////////////////////////////// update parts of the drawable world //////////////////////////////////////////

    CellWorld& DrawableWorld::getCellWorld() {

        return _cell_world;
    }

    LightWorld& DrawableWorld::getLightWorld() {

        return _light_world;
    }

    MaterialAtlas& DrawableWorld::getMaterialAtlas() {

        return _materials;
    }

    Environment& DrawableWorld::getEnvironment() {

        return _environment;
    }

    void DrawableWorld::setSunDirection(const glm::vec3& dir) {

        _sun.setDirection(dir);

    }

    void DrawableWorld::setSunColor(const glm::vec3& color) {

        _sun.setColor(color);
    }

    void DrawableWorld::applyUpdates(undicht::vulkan::CommandBuffer& load_cmd, undicht::vulkan::TransferBuffer& load_buf) {

        _cell_world.applyUpdates(load_cmd, load_buf);
        _light_world.applyUpdates(load_cmd, load_buf);
    }

    /////////////////////////////////// access parts of the drawable world (for rendering) ///////////////////////////////////

    const CellBuffer& DrawableWorld::getCellBuffer() const {

        return _cell_world.getBuffer();
    }

    const LightBuffer& DrawableWorld::getLightBuffer() const {

        return _light_world.getBuffer();
    }

    const Light& DrawableWorld::getSun() const {

        return _sun;
    }

} // namespace cell