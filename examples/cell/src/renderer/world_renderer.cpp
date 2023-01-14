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
        setDescriptorSetLayout({
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // global ubo
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // local ubo
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // per chunk data
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER // tile map
        });
        setVertexInputLayout(CUBE_VERTEX_LAYOUT, CELL_LAYOUT);
        setDepthStencilTest(true, true);
        setRasterizer(true);
        setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        setBlending(0, false); // material output
        setBlending(1, false); // normal output

        Renderer::init(viewport, render_pass, subpass);

        // renderer
        _sampler.setMinFilter(VK_FILTER_NEAREST);
        _sampler.setMaxFilter(VK_FILTER_NEAREST);
        _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _sampler.init(gpu.getDevice());

        // uniform buffer layout:
        // vec2 tile map unit (size of one tile in normalized device coords)
        _local_uniform_buffer.init(gpu, BufferLayout({UND_VEC2F}));

    }

    void WorldRenderer::cleanUp() {
        
        for(auto& ubo : _per_chunk_uniform_buffer) 
            ubo.cleanUp();

        _local_uniform_buffer.cleanUp();
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

        // calculating the size of a tile on the tile map        
        float tile_map_unit[2];
        tile_map_unit[0] = 1.0f / MaterialAtlas::TILE_MAP_COLS; // width of a tile (in ndc)
        tile_map_unit[1] = 1.0f / MaterialAtlas::TILE_MAP_ROWS; // height of a tile (in ndc)
        _local_uniform_buffer.setAttribute(0, tile_map_unit, 2 * sizeof(float));

        // drawing the chunks
        uint32_t cell_byte_size = CELL_LAYOUT.getTotalSize();
        createPerChunkUBOs(world.getDrawAreas().size());
        for(const WorldBuffer::BufferEntry& entry : world.getDrawAreas()) {

            // create a descriptor set pointing to the uniform buffers and the cell texture
            undicht::vulkan::DescriptorSet& descriptor_set = _descriptor_cache.accquire();
            descriptor_set.bindUniformBuffer(0, global_ubo.getBuffer());
            descriptor_set.bindUniformBuffer(1, _local_uniform_buffer.getBuffer());
            descriptor_set.bindImage(3, materials.getTileMap().getImage().getImageView(), materials.getTileMap().getLayout(), _sampler.getSampler());

            // loading the chunk position to the ubo
            _last_used_chunk_ubo++;
            UniformBuffer& per_chunk_ubo = _per_chunk_uniform_buffer.at(_last_used_chunk_ubo);
            per_chunk_ubo.setAttribute(0, glm::value_ptr(entry._chunk_pos), 3 * sizeof(int32_t));

            // binding the ubo to the shader
            descriptor_set.bindUniformBuffer(2, per_chunk_ubo.getBuffer());
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