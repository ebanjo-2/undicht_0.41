#include "shadow_renderer.h"
#include "world/cells/cell.h"
#include "file_tools.h"
#include "glm/gtc/type_ptr.hpp"
#include "debug.h"

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    void ShadowRenderer::init(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass, uint32_t subpass, uint32_t num_frames) {
        
        _device_handle = gpu;

        _renderer.setDeviceHandle(gpu);
        _renderer.setShaders(getFilePath(UND_CODE_SRC_FILE) + "shader/bin/direct_shadow.vert.spv", getFilePath(UND_CODE_SRC_FILE) + "shader/bin/direct_shadow.frag.spv");
        _renderer.setDescriptorSetLayout({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}, 0, 0); // global descriptors
        _renderer.setDescriptorSetLayout({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}, 1, 0); // renderer specific descriptors
        _renderer.setDescriptorSetLayout({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}, 2, 1000); // chunk specific descriptors
        _renderer.setVertexInputLayout(CUBE_VERTEX_LAYOUT, CELL_LAYOUT);
        _renderer.setDepthStencilTest(true, true);
        _renderer.setRasterizer(true, true, false);
        _renderer.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        //_renderer.setBlending(0, false);
        _renderer.init(viewport, render_pass, subpass, num_frames);

    }
    
    void ShadowRenderer::cleanUp() {

        for(auto& ubo : _per_chunk_uniform_buffer) 
            ubo.cleanUp();

        _renderer.cleanUp();
    }

    void ShadowRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {
        
        _renderer.resizeViewport(viewport);
    }

    void ShadowRenderer::beginFrame(undicht::vulkan::CommandBuffer& cmd, const undicht::vulkan::DescriptorSet& global_descriptor_set, uint32_t frame_id) {

        _renderer.beginFrame(frame_id);

        _last_used_chunk_ubo = -1;
        _renderer.resetDescriptorCache(2);

        // bind global descriptor set
        _renderer.bindDescriptorSet(cmd, global_descriptor_set, 0);

    }

    void ShadowRenderer::draw(const CellBuffer& world, undicht::vulkan::CommandBuffer& cmd) {
        // draw to shadow map

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

    void ShadowRenderer::createPerChunkUBOs(uint32_t num) {

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

} // cell