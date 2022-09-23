#include "instance.h"
#include "debug.h"
#include "GLFW/glfw3.h"

namespace undicht {

    namespace vulkan {

        void Instance::init() {

            // getting the features the instance should have
            std::vector<const char*> extensions = getExtensions();
            std::vector<const char*> val_layers = getValidationLayers();

            // instance create structs
            VkApplicationInfo app_info = createApplicationInfo("undicht_engine", "app");
            VkInstanceCreateInfo create_info = createInstanceCreateInfo(app_info, val_layers, extensions);

            // creating the instance
            vkCreateInstance(&create_info, {}, &_instance);

        }

        void Instance::cleanUp() {

            vkDestroyInstance(_instance, {});

        }

        const VkInstance& Instance::getInstance() const {

            return _instance;
        }

        VkPhysicalDevice Instance::chooseGPU(const VkSurfaceKHR& surface) const {

            // querying for available gpus
            unsigned physical_device_count;
            vkEnumeratePhysicalDevices(_instance, &physical_device_count, nullptr);
            std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
            vkEnumeratePhysicalDevices(_instance, &physical_device_count, physical_devices.data());

            // finding the best one
            uint32_t highscore = 0;
            uint32_t id = 0;

            for(int i = 0; i < physical_device_count; i++) {

                uint32_t score = ratePhysicalDevice(physical_devices.at(i));

                if(score > highscore) {
                    highscore = score;
                    id = i;
                }

            }

            if(highscore == 0) {
                UND_ERROR << "failed to find a suitable gpu\n";
                return VK_NULL_HANDLE;
            }

            return physical_devices.at(id);
        }

        ////////////////////////////////////// choosing a physical device //////////////////////////////

        uint32_t Instance::ratePhysicalDevice(const VkPhysicalDevice& device) {

            uint32_t score = 0;

            // getting features and properties
            VkPhysicalDeviceProperties properties{};
            VkPhysicalDeviceFeatures features{};

            vkGetPhysicalDeviceProperties(device, &properties);
            vkGetPhysicalDeviceFeatures(device, &features);

            // Discrete GPUs have a significant performance advantage
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                score += 1000;

            // Maximum possible size of textures speaks for a better gpu
            score += properties.limits.maxImageDimension2D;

            // features that are absolutely necessary
            if (!features.geometryShader)
                return 0;

            if(!features.samplerAnisotropy)
                return 0;

            if(!deviceExtensionSupport(device, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
                return 0;

            return score;
        }

        bool Instance::deviceExtensionSupport(const VkPhysicalDevice& device, const std::string& extension) {

            // querying for all extensions that are supported
            unsigned extension_count;
            vkEnumerateDeviceExtensionProperties(device, {}, &extension_count, nullptr);
            std::vector<VkExtensionProperties> extensions(extension_count);
            vkEnumerateDeviceExtensionProperties(device, {}, &extension_count, extensions.data());

            // trying to find the extension
            for(VkExtensionProperties& p : extensions)
                if (p.extensionName == extension)
                    return true;

            return false;
        }


        ////////////////////////////// specifying features of the vulkan instance //////////////////////////////

        std::vector<const char*> Instance::getExtensions() {

            std::vector<const char*> extensions;

            // specifying the extensions needed by the engine

            // getting the extensions that are required by glfw
            uint32_t ext_count = 0;
            const char **extns = glfwGetRequiredInstanceExtensions(&ext_count);

            // storing the glfw extensions in the vector
            for(int i = 0; i < ext_count; i++)
                extensions.emplace_back(extns[i]);

            return extensions;
        }

        std::vector<const char*> Instance::getValidationLayers() {

            std::vector<const char*> val_layers = {"VK_LAYER_KHRONOS_validation"};

            return val_layers;
        }

        ////////////////////////////////// creating instance related structs //////////////////////////////////

        VkApplicationInfo Instance::createApplicationInfo(const std::string& engine_name, const std::string& app_name) {

            VkApplicationInfo info{};
            info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            info.pEngineName = engine_name.c_str();
            info.pApplicationName = app_name.c_str();

            return info;
        }

        VkInstanceCreateInfo Instance::createInstanceCreateInfo(const VkApplicationInfo& app_info, const std::vector<const char*>& val_layers, const std::vector<const char*>& extensions) {

            VkInstanceCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            info.pApplicationInfo = &app_info;
            info.enabledLayerCount = val_layers.size();
            info.ppEnabledLayerNames = val_layers.data();
            info.enabledExtensionCount = extensions.size();
            info.ppEnabledExtensionNames = extensions.data();

            return info;
        }


    }

}
