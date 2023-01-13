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
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // material atlas
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, // depth texture input
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, // color texture input
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, // light map input
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
        // contains the tile map's tile size 
        _ubo.init(device, BufferLayout({UND_VEC2F}));
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

    void FinalRenderer::draw(const MaterialAtlas& materials, const undicht::vulkan::UniformBuffer& global_ubo, undicht::vulkan::CommandBuffer& cmd, VkImageView color_input, VkImageView depth_input, VkImageView light_input) {

        cmd.bindGraphicsPipeline(_pipeline.getPipeline());
        cmd.bindVertexBuffer(_screen_quad.getVertexBuffer().getBuffer(), 0);

        // storing the material tile maps dimensions in the global ubo
        float tile_map_unit[2];
        tile_map_unit[0] = 1.0f / MaterialAtlas::TILE_MAP_COLS; // width of a tile (in ndc)
        tile_map_unit[1] = 1.0f / MaterialAtlas::TILE_MAP_ROWS; // height of a tile (in ndc)
        _ubo.setAttribute(0, tile_map_unit, 2 * sizeof(float));

        // create a descriptor set pointing to the uniform buffers and the material atlas texture
        undicht::vulkan::DescriptorSet& descriptor_set = _descriptor_cache.accquire();
        descriptor_set.bindUniformBuffer(0, global_ubo.getBuffer());
        descriptor_set.bindUniformBuffer(1, _ubo.getBuffer());
        descriptor_set.bindImage(2, materials.getTileMap().getImage().getImageView(), materials.getTileMap().getLayout(), _sampler.getSampler());
        descriptor_set.bindInputAttachment(3, depth_input);
        descriptor_set.bindInputAttachment(4, color_input);
        descriptor_set.bindInputAttachment(5, light_input);

        // bind the descriptor set
        cmd.bindDescriptorSet(descriptor_set.getDescriptorSet(), _pipeline.getPipelineLayout());

        // draw command
        cmd.draw(6); // 2 triangles make up the screen quad, so 6 vertices in total

    }

    void FinalRenderer::beginFrame() {

        _descriptor_cache.reset();
    }

} // cell