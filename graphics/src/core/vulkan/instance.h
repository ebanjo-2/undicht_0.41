#ifndef VULKAN_INSTANCE_H
#define VULKAN_INSTANCE_H

#include "string"
#include "vector"
#include "vulkan/vulkan.h"

namespace undicht {

    namespace vulkan {

        class Instance {

        protected:

            VkInstance _instance;

        public:

            void init();
            void cleanUp();

            const VkInstance& getInstance() const;
            VkPhysicalDevice chooseGPU(const VkSurfaceKHR& surface) const;

        protected:
            // choosing a physical device

            uint32_t static ratePhysicalDevice(const VkPhysicalDevice& device);
            bool static deviceExtensionSupport(const VkPhysicalDevice& device, const std::string& extension);

        protected:
            // specifying features of the vulkan instance

            std::vector<const char*> static getExtensions();
            std::vector<const char*> static getValidationLayers();

        protected:
            // creating instance related structs

            VkApplicationInfo static createApplicationInfo(const std::string& engine_name, const std::string& app_name);
            VkInstanceCreateInfo static createInstanceCreateInfo(const VkApplicationInfo& app_info, const std::vector<const char*>& val_layers, const std::vector<const char*>& extensions);

        };

    } // vulkan

} // undicht

#endif // VULKAN_INSTANCE_H
