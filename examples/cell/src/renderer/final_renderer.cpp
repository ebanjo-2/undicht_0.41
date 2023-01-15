#include "final_renderer.h"
#include "buffer_layout.h"
#include "file_tools.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    // screen quad vertex layout
    const BufferLayout SCREEN_QUAD_VERTEX_LAYOUT({UND_VEC2F, UND_VEC2F});

    const std::vector<float> SCREEN_QUAD_VERTICES = {
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };

    void FinalRenderer::init(const undicht::vulkan::LogicalDevice& device, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass) {

        // init the screen quad
        _screen_quad.init(device);
        _screen_quad.setVertexData(SCREEN_QUAD_VERTICES.data(), SCREEN_QUAD_VERTICES.size() * sizeof(float), 0);
        
        // setting up the renderer
        setDeviceHandle(device);
        setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/final.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/final.frag.spv");
        setDescriptorSetLayout({
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // global uniform buffer
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // local uniform buffer
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, // light texture input
        });
        setVertexInputLayout(SCREEN_QUAD_VERTEX_LAYOUT);
        setDepthStencilTest(false, false);
        setRasterizer(false);
        setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        setBlending(0, false);
        Renderer::init(viewport, render_pass, subpass);

        // renderer
        _sampler.setMinFilter(VK_FILTER_NEAREST);
        _sampler.setMaxFilter(VK_FILTER_NEAREST);
        _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _sampler.init(device.getDevice());

        // local uniform buffer
        // exposure setting
        // + gamma
        _ubo.init(device, BufferLayout({UND_FLOAT32, UND_FLOAT32}));
    }

    void FinalRenderer::cleanUp() {

        _screen_quad.cleanUp();
        _ubo.cleanUp();
        _sampler.cleanUp();

        Renderer::cleanUp();
    }

    void FinalRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {

        Renderer::resizeViewport(viewport);
    }

    void FinalRenderer::draw(const undicht::vulkan::UniformBuffer& global_ubo, undicht::vulkan::CommandBuffer& cmd, float exposure, float gamma, VkImageView light) {

        cmd.bindGraphicsPipeline(_pipeline.getPipeline());
        cmd.bindVertexBuffer(_screen_quad.getVertexBuffer().getBuffer(), 0);

        _ubo.setAttribute(0, &exposure, sizeof(float));
        _ubo.setAttribute(1, &gamma, sizeof(float));

        // create a descriptor set pointing to the uniform buffers and the material atlas texture
        undicht::vulkan::DescriptorSet& descriptor_set = _descriptor_cache.accquire();
        descriptor_set.bindUniformBuffer(0, global_ubo.getBuffer());
        descriptor_set.bindUniformBuffer(1, _ubo.getBuffer());
        descriptor_set.bindInputAttachment(2, light);

        // bind the descriptor set
        cmd.bindDescriptorSet(descriptor_set.getDescriptorSet(), _pipeline.getPipelineLayout());

        // draw command
        cmd.draw(6); // 2 triangles make up the screen quad, so 6 vertices in total

    }

    void FinalRenderer::beginFrame() {

        _descriptor_cache.reset();
    }

} // cell