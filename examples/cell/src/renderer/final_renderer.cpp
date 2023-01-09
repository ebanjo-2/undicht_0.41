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

        _device_handle = device;
        _subpass = subpass;

        _screen_quad.init(device);
        _screen_quad.setVertexData(SCREEN_QUAD_VERTICES.data(), SCREEN_QUAD_VERTICES.size() * sizeof(float), 0);

        // Shader
        _shader.addVertexModule(device.getDevice(), getFilePath(UND_CODE_SRC_FILE) + "shader/bin/final.vert.spv");
        _shader.addFragmentModule(device.getDevice(), getFilePath(UND_CODE_SRC_FILE) + "shader/bin/final.frag.spv");
        _shader.init(device.getDevice());

        // Pipeline
        _descriptor_set_layout.setBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // global data
        _descriptor_set_layout.setBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // material atlas
        _descriptor_set_layout.setBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT); // depth texture input
        _descriptor_set_layout.setBinding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT); // color texture input
        _descriptor_set_layout.setBinding(4, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT); // light map input
        _descriptor_set_layout.init(device.getDevice());

        _descriptor_cache.init(device, _descriptor_set_layout);

        _pipeline.setViewport(viewport);
        _pipeline.setShaderStages(_shader.getShaderModules(), _shader.getShaderStages());
        _pipeline.setBlending(0, false);
        _pipeline.setDepthStencilState(false, false);
        _pipeline.setInputAssembly();
        _pipeline.setRasterizationState(false, false, false);
        _pipeline.setShaderInput(_descriptor_set_layout.getLayout());
        _pipeline.setVertexBinding(0, 0, SCREEN_QUAD_VERTEX_LAYOUT);

        _pipeline.init(device.getDevice(), render_pass.getRenderPass(), subpass);

        // renderer
        _sampler.setMinFilter(VK_FILTER_NEAREST);
        _sampler.setMaxFilter(VK_FILTER_NEAREST);
        _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _sampler.init(device.getDevice());

        // global uniform buffer
        // view, proj, inverse view, inverse projection, tile size
        _global_uniform_buffer.init(device, BufferLayout({UND_MAT4F, UND_MAT4F, UND_MAT4F, UND_MAT4F, UND_VEC2F}));
    }

    void FinalRenderer::cleanUp() {
        
        _global_uniform_buffer.cleanUp();
        _sampler.cleanUp();
        _pipeline.cleanUp();
        _descriptor_cache.cleanUp();
        _descriptor_set_layout.cleanUp();
        _shader.cleanUp();
        _screen_quad.cleanUp();
    }

    void FinalRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {

        _pipeline.cleanUp();
        _pipeline.setViewport(viewport);
        _pipeline.init(gpu.getDevice(), render_pass.getRenderPass(), _subpass);
    }

    void FinalRenderer::loadCamera(undicht::tools::PerspectiveCamera3D& camera) {

        const glm::mat4& view = camera.getViewMatrix();
        const glm::mat4& proj = camera.getCameraProjectionMatrix();

        glm::mat4 inv_view = glm::inverse(view);
        glm::mat4 inv_proj = glm::inverse(proj);

        _global_uniform_buffer.setAttribute(0, glm::value_ptr(view), 16 * sizeof(float));
        _global_uniform_buffer.setAttribute(1, glm::value_ptr(proj), 16 * sizeof(float));
        _global_uniform_buffer.setAttribute(2, glm::value_ptr(inv_view), 16 * sizeof(float));
        _global_uniform_buffer.setAttribute(3, glm::value_ptr(inv_proj), 16 * sizeof(float));
    }

    void FinalRenderer::draw(const MaterialAtlas& materials, undicht::vulkan::CommandBuffer& cmd, const undicht::vulkan::Image& color_input, const undicht::vulkan::Image& depth_input, const undicht::vulkan::Image& light_input) {

        cmd.bindGraphicsPipeline(_pipeline.getPipeline());
        cmd.bindVertexBuffer(_screen_quad.getVertexBuffer().getBuffer(), 0);

        // storing the material tile maps dimensions in the global ubo
        float tile_map_unit[2];
        tile_map_unit[0] = 1.0f / MaterialAtlas::TILE_MAP_COLS; // width of a tile (in ndc)
        tile_map_unit[1] = 1.0f / MaterialAtlas::TILE_MAP_ROWS; // height of a tile (in ndc)
        _global_uniform_buffer.setAttribute(4, tile_map_unit, 2 * sizeof(float));

        // create a descriptor set pointing to the uniform buffers and the material atlas texture
        undicht::vulkan::DescriptorSet& descriptor_set = _descriptor_cache.accquire();
        descriptor_set.bindUniformBuffer(0, _global_uniform_buffer.getBuffer());
        descriptor_set.bindImage(1, materials.getTileMap().getImage().getImageView(), materials.getTileMap().getLayout(), _sampler.getSampler());
        descriptor_set.bindInputAttachment(2, depth_input.getImageView());
        descriptor_set.bindInputAttachment(3, color_input.getImageView());
        descriptor_set.bindInputAttachment(4, light_input.getImageView());

        // bind the descriptor set
        cmd.bindDescriptorSet(descriptor_set.getDescriptorSet(), _pipeline.getPipelineLayout());

        // draw command
        cmd.draw(6); // 2 triangles make up the screen quad, so 6 vertices in total

    }

    void FinalRenderer::beginFrame() {

        _descriptor_cache.reset();
    }

} // cell