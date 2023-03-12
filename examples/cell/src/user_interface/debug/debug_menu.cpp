#include "debug_menu.h"
#include "imgui/vulkan/imgui_api.h"
#include "file_tools.h"
#include "environment/environment_generator.h"

namespace cell {

    using namespace undicht;
    using namespace tools;

    void DebugMenu::open() {

        _open = true;
    }

    void DebugMenu::close() {

        _open = false;
    }

    bool DebugMenu::isOpen() const {

        return _open;
    }

    void DebugMenu::display(double fps, Environment& env) {

        if(!_open) return;

        std::string title = "Cell Debug Menu";
        std::string frames = "Frames: " + toStr(fps);

        bool gen_environment;
        bool calc_lighting;

        ImGui::Begin(title.data(), &_open);
        ImGui::Text(frames.data());
        ImGui::SliderFloat("Cloud Coverage", &_cloud_coverage, 0.0f, 2.0f);
        ImGui::SliderFloat("Cloud Density", &_cloud_density, 0.0f, 10.0f);
        ImGui::SliderFloat("Sky Brightness", &_sky_brightness, 0.0f, 2.0f);
        ImGui::SliderFloat("Cloud Brightness", &_cloud_brightness, 0.0f, 2.0f);
        gen_environment = ImGui::Button("Generate");
        calc_lighting = ImGui::Button("update Lighting");
        ImGui::End();

        if(gen_environment) {
            EnvironmentGenerator gen;
            gen.init();
            gen.setCloudCoverage(_cloud_coverage); 
            gen.setCloudDensity(_cloud_density);
            gen.setSkyBrightness(_sky_brightness);
            gen.setCloudBrightness(_cloud_brightness);

            gen.generate(_env_map_data);
            env.load(_env_map_data);
        }

        if(calc_lighting) {
            env.calcLightingMaps(_env_map_data);
        }

    }

} // cell