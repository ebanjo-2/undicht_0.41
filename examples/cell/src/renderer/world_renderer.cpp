#include "renderer/world_renderer.h"
#include "file_tools.h"
#include "core/vulkan/formats.h"
#include "world/cell.h"
#include "renderer/world_buffer.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "debug.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    void WorldRenderer::init(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass) {
        
        // setting up the renderer
        setDeviceHandle(gpu);
        setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/world.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/world.frag.spv");
        setDescriptorSetLayout({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
        setVertexInputLayout(CUBE_VERTEX_LAYOUT, CELL_LAYOUT);
        setDepthStencilTest(true, true);
        setRasterizer(true);
        setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        setBlending(0, false);
        Renderer::init(viewport, render_pass, subpass);

        // renderer
        _sampler.setMinFilter(VK_FILTER_NEAREST);
        _sampler.setMaxFilter(VK_FILTER_NEAREST);
        _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _sampler.init(gpu.getDevice());

        /*// uniform buffer layout:
        // mat4 proj
        // mat4 view
        // vec2 tile map unit (size of one tile in normalized device coords)
        _global_uniform_buffer.init(gpu, BufferLayout({UND_MAT4F, UND_MAT4F, UND_VEC2F}));*/

    }

    void WorldRenderer::cleanUp() {
        
        for(auto& ubo : _per_chunk_uniform_buffer) 
            ubo.cleanUp();

        // _global_uniform_buffer.cleanUp();
        _sampler.cleanUp();

        Renderer::cleanUp();
    }

    void WorldRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {

        Renderer::resizeViewport(viewport);
    }


    void WorldRenderer::draw(const WorldBuffer& world, const MaterialAtlas& materials, const undicht::vulkan::UniformBuffer& global_ubo, undicht::vulkan::CommandBuffer& cmd) {

        cmd.bindGraphicsPipeline(_pipeline.getPipeline());

        cmd.bindVertexBuffer(world.getBuffer().getVertexBuffer().getBuffer(), 0);
        cmd.bindVertexBuffer(world.getBuffer().getInstanceBuffer().getBuffer(), 1);

        // drawing the chunks
        uint32_t cell_byte_size = CELL_LAYOUT.getTotalSize();
        createPerChunkUBOs(world.getDrawAreas().size());
        for(const WorldBuffer::BufferEntry& entry : world.getDrawAreas()) {

            // create a descriptor set pointing to the uniform buffers and the cell texture
            undicht::vulkan::DescriptorSet& descriptor_set = _descriptor_cache.accquire();
            descriptor_set.bindUniformBuffer(0, global_ubo.getBuffer());
            descriptor_set.bindImage(2, materials.getTileMap().getImage().getImageView(), materials.getTileMap().getLayout(), _sampler.getSampler());

            // loading the chunk position to the ubo
            _last_used_chunk_ubo++;
            UniformBuffer& per_chunk_ubo = _per_chunk_uniform_buffer.at(_last_used_chunk_ubo);
            per_chunk_ubo.setAttribute(0, glm::value_ptr(entry._chunk_pos), 3 * sizeof(int32_t));

            // binding the ubo to the shader
            descriptor_set.bindUniformBuffer(1, per_chunk_ubo.getBuffer());
            cmd.bindDescriptorSet(descriptor_set.getDescriptorSet(), _pipeline.getPipelineLayout());

            // draw command
            cmd.draw(36, false, entry.byte_size / cell_byte_size, 0, entry.offset / cell_byte_size);
        }

    }

    void WorldRenderer::beginFrame() {

        _last_used_chunk_ubo = -1;
        _descriptor_cache.reset();
    }


    const undicht::vulkan::DescriptorSetLayout& WorldRenderer::getDescriptorSetLayout() const {

        return _descriptor_set_layout;
    }


    ///////////////////////////////// private renderer functions /////////////////////////////////


    void WorldRenderer::createPerChunkUBOs(uint32_t num) {

        uint32_t old_ubo_count = _per_chunk_uniform_buffer.size();

        if(num < (old_ubo_count - _last_used_chunk_ubo))
            return; // no need to create new ubos

        // creating new ubos
        _per_chunk_uniform_buffer.resize(_last_used_chunk_ubo + num + 1);
        
        for(int i = old_ubo_count; i < _per_chunk_uniform_buffer.size();i ++) {
            // layout of the per chunk ubo:
            // ivec3 to store the chunk position
            _per_chunk_uniform_buffer.at(i).init(_device_handle, BufferLayout({UND_VEC3I})); 
        }
        
    }


} // namespace cell