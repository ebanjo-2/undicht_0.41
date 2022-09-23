#ifndef SHADER_H
#define SHADER_H

#include "string"
#include "vector"
#include "vulkan/vulkan.h"

namespace undicht {

    namespace vulkan {

        class Shader {

        protected:

            VkDevice _device_handle;

            //VkShader _shader;
            VkShaderModule _vert_module = VK_NULL_HANDLE;
            VkShaderModule _frag_module = VK_NULL_HANDLE;

        public:

            void addVertexModule(const VkDevice& device, const std::string& file_name);
            void addFragmentModule(const VkDevice& device,const std::string& file_name);

            /** the modules should be added before initializing the shader */
            void init(const VkDevice& device);
            void cleanUp();

            std::vector<VkShaderModule> getShaderModules() const;
            std::vector<VkShaderStageFlagBits> getShaderStages() const;

        protected:
            // creating shader related structs

            bool static loadShaderModule(const VkDevice& device, VkShaderModule& module, const std::string& file_name);
            VkShaderModuleCreateInfo static createShaderModuleCreateInfo(const std::vector<uint32_t>& spirv_binary);
        };

    } // vulkan

} // undicht

#endif // SHADER_H