#include "imgui_api.h"
#include "vector"
#include "core/vulkan/command_buffer.h"


namespace undicht {

    namespace vulkan {

        // static objects
        VkDescriptorPool ImGuiAPI::_descriptor_pool;

        void ImGuiAPI::init(const VkInstance& instance, const LogicalDevice& device, const VkRenderPass& render_pass, GLFWwindow* window) {
            // source: https://vkguide.dev/docs/extra-chapter/implementing_imgui/

            //1: create descriptor pool for IMGUI
	        // the size of the pool is very oversize, but it's copied from imgui demo itself.
	        std::vector<VkDescriptorPoolSize> pool_sizes = {
		        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	        };

            VkDescriptorPoolCreateInfo pool_info = {};
	        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	        pool_info.maxSets = 1000;
	        pool_info.poolSizeCount = pool_sizes.size();
	        pool_info.pPoolSizes = pool_sizes.data();

            vkCreateDescriptorPool(device.getDevice(), &pool_info, nullptr, &_descriptor_pool);

        	// 2: initialize imgui library

        	//this initializes the core structures of imgui
        	ImGui::CreateContext();

        	//this initializes imgui for SDL
        	ImGui_ImplGlfw_InitForVulkan(window, true);

        	//this initializes imgui for Vulkan
        	ImGui_ImplVulkan_InitInfo init_info = {};
        	init_info.Instance = instance;
        	init_info.PhysicalDevice = device.getPhysicalDevice();
        	init_info.Device = device.getDevice();
	        init_info.Queue = device.getGraphicsQueue();
	        init_info.DescriptorPool = _descriptor_pool;
	        init_info.MinImageCount = 3;
	        init_info.ImageCount = 3;
	        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        	ImGui_ImplVulkan_Init(&init_info, render_pass);

        	//execute a gpu command to upload imgui font textures
            CommandBuffer cmd;
            cmd.init(device.getDevice(), device.getGraphicsCmdPool());
			cmd.beginCommandBuffer(true);
        	ImGui_ImplVulkan_CreateFontsTexture(cmd.getCommandBuffer());
			cmd.endCommandBuffer();
            device.submitOnGraphicsQueue(cmd.getCommandBuffer());
            device.waitGraphicsQueueIdle();

        	//clear font textures from cpu data
        	ImGui_ImplVulkan_DestroyFontUploadObjects();

        }

        void ImGuiAPI::cleanUp(const VkDevice& device) {

		    vkDestroyDescriptorPool(device, _descriptor_pool, nullptr);
		    ImGui_ImplVulkan_Shutdown();
        }
    }

} // undicht