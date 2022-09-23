#include "shader.h"
#include "fstream"
#include "debug.h"

namespace undicht {

    namespace vulkan {

        void Shader::addVertexModule(const VkDevice& device,const std::string& file_name) {

            if(!loadShaderModule(device, _vert_module, file_name))
                UND_ERROR << "Failed to load Vertex Shader: " << file_name << "\n";

        }

        void Shader::addFragmentModule(const VkDevice& device,const std::string& file_name) {

            if(!loadShaderModule(device, _frag_module, file_name))
                UND_ERROR << "Failed to load Fragment Shader: " << file_name << "\n";

        }

        void Shader::init(const VkDevice& device) {
            /** the modules should be added before initializing the shader */

            _device_handle = device;

        }

        void Shader::cleanUp() {

            vkDestroyShaderModule(_device_handle, _vert_module, {});
            vkDestroyShaderModule(_device_handle, _frag_module, {});

        }

        std::vector<VkShaderModule> Shader::getShaderModules() const {

            std::vector<VkShaderModule> modules;

            if(_vert_module != VK_NULL_HANDLE)
                modules.push_back(_vert_module);

            if(_frag_module != VK_NULL_HANDLE)
                modules.push_back(_frag_module);

            return modules;
        }

        std::vector<VkShaderStageFlagBits> Shader::getShaderStages() const {

            std::vector<VkShaderStageFlagBits> stages;

            if(_vert_module != VK_NULL_HANDLE)
                stages.push_back(VK_SHADER_STAGE_VERTEX_BIT);

            if(_frag_module != VK_NULL_HANDLE)
                stages.push_back(VK_SHADER_STAGE_FRAGMENT_BIT);

            return stages;
        }

        ///////////////////////////////// creating shader related structs /////////////////////////////////

        bool Shader::loadShaderModule(const VkDevice& device, VkShaderModule& module, const std::string& file_name) {
            /** loads a SPIRV shader binary file into the module
             * @return false, if errors occurred */

            // open the file, with the read position starting at the end
            std::ifstream file(file_name, std::ios::ate | std::ios::binary);
            if(!file.is_open())
                return false;

            size_t file_size = file.tellg();

            // loading the contents of the file
            std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));
            file.seekg(0);
            file.read((char*)buffer.data(), file_size);
            file.close();

            // load the binary data into the shader module
            VkShaderModuleCreateInfo info = createShaderModuleCreateInfo(buffer);
            return vkCreateShaderModule(device, &info, nullptr, &module) == VK_SUCCESS;
        }

        VkShaderModuleCreateInfo Shader::createShaderModuleCreateInfo(const std::vector<uint32_t>& spirv_binary) {

            VkShaderModuleCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            info.pNext = nullptr;
            info.codeSize = spirv_binary.size() * sizeof(uint32_t); // in bytes
            info.pCode = spirv_binary.data();

            return info;
        }

    } // vulkan

} // undicht