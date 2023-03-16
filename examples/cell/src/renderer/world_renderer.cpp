#include "renderer/world_renderer.h"
#include "file_tools.h"
#include "core/vulkan/formats.h"
#include "world/cells/cell.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "debug.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    void WorldRenderer::init(const undicht::vulkan::LogicalDevice& gpu, const undicht::vulkan::DescriptorSetLayout& global_descriptor_layout, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass, uint32_t num_frames) {
        
        _device_handle = gpu;
        
        // setting up the renderer
        _renderer.setDeviceHandle(gpu);
        _renderer.setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/world.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/world.frag.spv");
        _renderer.setDescriptorSetLayout(global_descriptor_layout, 0, 0); // global ubo
        _renderer.setDescriptorSetLayout({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER /*local ubo*/, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER /*tile map*/}, 1);
        _renderer.setDescriptorSetLayout({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER /*per chunk data*/}, 2);
        _renderer.setVertexInputLayout(CUBE_VERTEX_LAYOUT, CELL_LAYOUT);
        _renderer.setDepthStencilTest(true, true);
        _renderer.setRasterizer(true, false);
        _renderer.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        _renderer.setBlending(0, false); // albedo roughness output
        _renderer.setBlending(1, false); // normal metalness output
        _renderer.setBlending(2, false); // position rel cam output
        _renderer.setBlending(3, false); // shadow map pos

        _renderer.init(viewport, render_pass, subpass, num_frames);

        // Sampler
        _tile_map_sampler.setMinFilter(VK_FILTER_NEAREST);
        _tile_map_sampler.setMaxFilter(VK_FILTER_NEAREST);
        _tile_map_sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _tile_map_sampler.init(gpu.getDevice());

        // uniform buffer layout:
        // vec2 tile map unit (size of one tile in normalized device coords)
        _local_uniform_buffer.init(gpu, BufferLayout({UND_VEC2F}));

    }

    void WorldRenderer::cleanUp() {
        
        for(auto& ubo : _per_chunk_uniform_buffer) 
            ubo.cleanUp();

        _local_uniform_buffer.cleanUp();
        _tile_map_sampler.cleanUp();

        _renderer.cleanUp();
    }

    void WorldRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {

        _renderer.resizeViewport(viewport);
    }

    void WorldRenderer::beginFrame(const MaterialAtlas& materials, const undicht::vulkan::DescriptorSet& global_descriptor_set, undicht::vulkan::CommandBuffer& cmd, uint32_t frame_id) {

        _renderer.beginFrame(frame_id);

        _last_used_chunk_ubo = -1;
        _renderer.resetDescriptorCache(1);
        _renderer.resetDescriptorCache(2);

        // calculating the size of a tile on the tile map
        float tile_map_unit[2];
        tile_map_unit[0] = 1.0f / MaterialAtlas::TILE_MAP_COLS; // width of a tile (in ndc)
        tile_map_unit[1] = 1.0f / MaterialAtlas::TILE_MAP_ROWS; // height of a tile (in ndc)
        _local_uniform_buffer.setAttribute(0, tile_map_unit, 2 * sizeof(float));

        // updating + binding the local descriptor set
        _renderer.accquireDescriptorSet(1);
        _renderer.bindUniformBuffer(1, 0, _local_uniform_buffer.getBuffer());
        _renderer.bindImage(1, 1, materials.getTileMap().getImage().getImageView(), materials.getTileMap().getLayout(), _tile_map_sampler.getSampler());
        _renderer.bindDescriptorSet(cmd, 1);

        // binding the global descriptor set
        _renderer.bindDescriptorSet(cmd, global_descriptor_set, 0);

    }

    void WorldRenderer::draw(const CellBuffer& world, undicht::vulkan::CommandBuffer& cmd) {
        
        _renderer.bindPipeline(cmd);
        _renderer.bindVertexBuffer(cmd, world.getBuffer(), false, true);

        // drawing the chunks
        uint32_t cell_byte_size = CELL_LAYOUT.getTotalSize();
        createPerChunkUBOs(world.getDrawAreas().size());
        for(const CellBuffer::BufferEntry& entry : world.getDrawAreas()) {

            // loading the chunk position to the chunk ubo
            _last_used_chunk_ubo++;
            UniformBuffer& per_chunk_ubo = _per_chunk_uniform_buffer.at(_last_used_chunk_ubo);
            per_chunk_ubo.setAttribute(0, glm::value_ptr(entry._chunk_pos), 3 * sizeof(int32_t));

            // create a descriptor set pointing to the uniform buffers and the cell texture
            _renderer.accquireDescriptorSet(2);
            _renderer.bindUniformBuffer(2, 0, per_chunk_ubo.getBuffer());

            // binding the ubo to the shader
            _renderer.bindDescriptorSet(cmd, 2);

            // draw command
            _renderer.draw(cmd, world.getCellModelVertexCount(), false, entry.byte_size / cell_byte_size, 0, entry.offset / cell_byte_size);

        }

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