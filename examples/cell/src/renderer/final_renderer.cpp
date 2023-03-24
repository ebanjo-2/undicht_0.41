#include "final_renderer.h"
#include "buffer_layout.h"
#include "file_tools.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "renderer/vulkan/immediate_command.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    void FinalRenderer::init(const undicht::vulkan::LogicalDevice& device, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass, uint32_t num_frames) {

        // init the screen quad
        _screen_quad.init(device);
        {
            ImmediateCommand cmd(device);
            _screen_quad.setVertexData(SCREEN_QUAD_VERTICES.data(), SCREEN_QUAD_VERTICES.size() * sizeof(float), 0, cmd);
        }

        // setting up the renderer
        _renderer.setDeviceHandle(device);
        _renderer.setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/final.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/final.frag.spv");
        _renderer.setDescriptorSetLayout(std::vector<VkDescriptorType>({}), 0, 0); // global descriptor set is not used by the final renderer
        _renderer.setDescriptorSetLayout({
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // local uniform buffer
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, // light texture input
        }, 1);
        _renderer.setVertexInputLayout(SCREEN_QUAD_VERTEX_LAYOUT);
        _renderer.setDepthStencilTest(false, false);
        _renderer.setRasterizer(false);
        _renderer.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        _renderer.setBlending(0, false);
        _renderer.init(viewport, render_pass, subpass, num_frames);

        // renderer
        _sampler.setMinFilter(VK_FILTER_NEAREST);
        _sampler.setMaxFilter(VK_FILTER_NEAREST);
        _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _sampler.init(device.getDevice());

        // local uniform buffer
        // exposure setting
        _local_ubo.init(device, BufferLayout({UND_FLOAT32}));
    }

    void FinalRenderer::cleanUp() {

        _screen_quad.cleanUp();
        _local_ubo.cleanUp();
        _sampler.cleanUp();

        _renderer.cleanUp();
    }

    void FinalRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {

        _renderer.resizeViewport(viewport);
    }

    void FinalRenderer::beginFrame(undicht::vulkan::CommandBuffer& cmd, VkImageView light_hdr, uint32_t frame_id) {

        _renderer.beginFrame(frame_id);
        _renderer.resetDescriptorCache(1);
        _renderer.accquireDescriptorSet(1);
        
        // binding the light hdr texture
        _renderer.bindInputAttachment(1, 1, light_hdr);

    }

    void FinalRenderer::draw(undicht::vulkan::CommandBuffer& cmd, float exposure) {

        // update + bind local ubo
        _local_ubo.setAttribute(0, &exposure, sizeof(float));
        _renderer.bindUniformBuffer(1, 0, _local_ubo.getBuffer());
        _renderer.bindDescriptorSet(cmd, 1);

        _renderer.bindPipeline(cmd);
        _renderer.bindVertexBuffer(cmd, _screen_quad);
        _renderer.draw(cmd, 6);

    }


} // cell