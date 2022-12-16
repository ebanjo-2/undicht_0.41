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

    void WorldRenderer::init(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {

        _device_handle = gpu;
        _render_pass_handle = render_pass;

        // Shader
        _shader.addVertexModule(gpu.getDevice(), getFilePath(UND_CODE_SRC_FILE) + "shader/bin/world.vert.spv");
        _shader.addFragmentModule(gpu.getDevice(), getFilePath(UND_CODE_SRC_FILE) + "shader/bin/world.frag.spv");
        _shader.init(gpu.getDevice());

        // Pipeline
        _descriptor_set_layout.setBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // global data
        _descriptor_set_layout.setBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // per chunk data
        _descriptor_set_layout.setBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        _descriptor_set_layout.init(gpu.getDevice());

        _pipeline.setViewport(viewport);
        _pipeline.setShaderStages(_shader.getShaderModules(), _shader.getShaderStages());
        _pipeline.setBlending(0, false);
        _pipeline.setDepthStencilState(true, true);
        _pipeline.setInputAssembly();
        _pipeline.setRasterizationState(true, false, false);
        _pipeline.setShaderInput(_descriptor_set_layout.getLayout());

        setVertexBinding(0, 0, CUBE_VERTEX_LAYOUT, _pipeline); // per vertex data
        setVertexBinding(1, CUBE_VERTEX_LAYOUT.m_types.size(), CELL_LAYOUT, _pipeline); // per cell data

        _pipeline.init(gpu.getDevice(), render_pass.getRenderPass());

        // renderer
        _sampler.setMinFilter(VK_FILTER_LINEAR);
        _sampler.setMaxFilter(VK_FILTER_LINEAR);
        _sampler.setMipMapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
        _sampler.init(gpu.getDevice());

        // uniform buffer layout:
        // mat4 proj
        // mat4 view
        // vec4 relative chunk position
        _global_uniform_buffer.init(gpu, BufferLayout({UND_MAT4F, UND_MAT4F}));

    }

    void WorldRenderer::cleanUp() {
        
        for(auto& ubo : _per_chunk_uniform_buffer) ubo.cleanUp();
        _global_uniform_buffer.cleanUp();
        _sampler.cleanUp();
        _pipeline.cleanUp();
        _descriptor_set_layout.cleanUp();
        _shader.cleanUp();

    }

    void WorldRenderer::onViewportResize(const undicht::vulkan::LogicalDevice& gpu, VkExtent2D viewport, const undicht::vulkan::RenderPass& render_pass) {

        _pipeline.cleanUp();
        _pipeline.setViewport(viewport);
        _pipeline.init(gpu.getDevice(), render_pass.getRenderPass());
    }

    void WorldRenderer::loadCamera(PerspectiveCamera3D& camera) {

        _global_uniform_buffer.setAttribute(0, glm::value_ptr(camera.getCameraProjectionMatrix()), 16 * sizeof(float));
        _global_uniform_buffer.setAttribute(1, glm::value_ptr(camera.getViewMatrix()), 16 * sizeof(float));

    }

    void WorldRenderer::draw(const WorldBuffer& world, undicht::vulkan::CommandBuffer& cmd, undicht::vulkan::DescriptorSetCache& descriptor_set_cache) {

        cmd.bindGraphicsPipeline(_pipeline.getPipeline());

        cmd.bindVertexBuffer(world.getBuffer().getVertexBuffer().getBuffer(), 0);
        cmd.bindVertexBuffer(world.getBuffer().getInstanceBuffer().getBuffer(), 1);

        // drawing the chunks
        uint32_t cell_byte_size = CELL_LAYOUT.getTotalSize();
        createPerChunkUBOs(world.getDrawAreas().size());
        for(const WorldBuffer::BufferEntry& entry : world.getDrawAreas()) {

            // create a descriptor set pointing to the uniform buffers and the cell texture
            undicht::vulkan::DescriptorSet& descriptor_set = descriptor_set_cache.accquire();
            descriptor_set.bindUniformBuffer(0, _global_uniform_buffer.getBuffer());
            // descriptor_set.bindImage(1, _model._textures[_model._texture_ids[i]].getImage().getImageView(), _model._textures[_model._texture_ids[i]].getLayout(), _sampler.getSampler());

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

    void WorldRenderer::resetPerChunkUBOs() {

        _last_used_chunk_ubo = -1;
    }


    const undicht::vulkan::DescriptorSetLayout& WorldRenderer::getDescriptorSetLayout() const {

        return _descriptor_set_layout;
    }


    ///////////////////////////////// private renderer functions /////////////////////////////////

    void WorldRenderer::setVertexBinding(uint32_t id, uint32_t location_offset, const undicht::BufferLayout& layout, undicht::vulkan::Pipeline& pipeline) {

        uint32_t total_size = layout.getTotalSize();
        pipeline.addVertexBinding(id, total_size);

        uint32_t current_offset = 0;
        uint32_t attribute_id = location_offset;
        for(const undicht::FixedType& t : layout.m_types) {

            pipeline.addVertexAttribute(id, attribute_id, current_offset, vulkan::translate(t)); // position

            current_offset += t.getSize();
            attribute_id += 1;
        }

    }

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