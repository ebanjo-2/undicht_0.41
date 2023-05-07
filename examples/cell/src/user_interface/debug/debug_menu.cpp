#include "debug_menu.h"
#include "imgui/vulkan/imgui_api.h"
#include "file_tools.h"
#include "environment/environment_generator.h"
#include "world/chunk_system/chunk_system.h"
#include "world/cells/cell.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    void DebugMenu::open() {

        _open = true;
    }

    void DebugMenu::close() {

        _open = false;
    }

    bool DebugMenu::isOpen() const {

        return _open;
    }

    void DebugMenu::applyUpdates( Environment& env, CommandBuffer& load_cmd, TransferBuffer& load_buf) {

        if(_update_environment) {
            EnvironmentGenerator gen;
            gen.init();
            gen.setCloudCoverage(_cloud_coverage); 
            gen.setCloudDensity(_cloud_density);
            gen.setSkyBrightness(_sky_brightness);
            gen.setCloudBrightness(_cloud_brightness);

            gen.generate(_env_map_data);
            env.load(_env_map_data, load_cmd, load_buf);
        }

        if(_update_light_maps) {
            env.calcLightingMaps(_env_map_data, load_cmd, load_buf);
        }

        _update_environment = false;
        _update_light_maps = false;

    }

    void DebugMenu::display(double fps, const glm::vec3& pos, const glm::vec3& dir) {

        if(!_open) return;

        glm::ivec3 chunk = ChunkSystem<Cell>::calcChunkPosition(glm::ivec3(pos));

        std::string title = "Cell Debug Menu";
        std::string frames = "Frames: " + toStr(fps);
        std::string player_pos = "Position: " + toStr(pos.x) + " : " + toStr(pos.y) + " : " + toStr(pos.z);
        std::string player_dir = "Direction: " + toStr(dir.x) + " : " + toStr(dir.y) + " : " + toStr(dir.z);
        std::string chunk_pos = "Chunk: " + toStr(chunk.x) + " : " + toStr(chunk.y) + " : " + toStr(chunk.z);

        ImGui::Begin(title.data(), &_open);
        ImGui::Text(frames.data(), "");
        ImGui::Text(player_pos.data(), "");
        ImGui::Text(player_dir.data(), "");
        ImGui::Text(chunk_pos.data(), "");
        ImGui::SliderFloat("Cloud Coverage", &_cloud_coverage, 0.0f, 2.0f);
        ImGui::SliderFloat("Cloud Density", &_cloud_density, 0.0f, 10.0f);
        ImGui::SliderFloat("Sky Brightness", &_sky_brightness, 0.0f, 2.0f);
        ImGui::SliderFloat("Cloud Brightness", &_cloud_brightness, 0.0f, 2.0f);
        _update_environment = ImGui::Button("Generate");
        _update_light_maps = ImGui::Button("update Lighting");
        ImGui::End();

    }

} // cell