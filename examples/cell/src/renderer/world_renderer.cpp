#include "renderer/world_renderer.h"
#include "file_tools.h"
#include "core/vulkan/formats.h"
#include "world/cell.h"
#include "renderer/world_buffer.h"
#include "glm/gtc/type_ptr.hpp"

namespace cell {

    using namespace undicht;
    using namespace tools;
    using namespace vulkan;

    void WorldRenderer::init(const undicht::vulkan::LogicalDevice& gpu, const undicht::vulkan::SwapChain& swap_chain, const undicht::vulkan::RenderPass& render_pass) {

        // Shader
        _shader.addVertexModule(gpu.getDevice(), getFilePath(UND_CODE_SRC_FILE) + "shader/bin/world.vert.spv");
        _shader.addFragmentModule(gpu.getDevice(), getFilePath(UND_CODE_SRC_FILE) + "shader/bin/world.frag.spv");
        _shader.init(gpu.getDevice());

        // Pipeline
        _descriptor_set_layout.setBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        _descriptor_set_layout.setBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        _descriptor_set_layout.init(gpu.getDevice());

        _pipeline.setViewport(swap_chain.getExtent());
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
        _uniform_buffer.init(gpu, BufferLayout({UND_MAT4F, UND_MAT4F})); 

    }

    void WorldRenderer::cleanUp() {

        _uniform_buffer.cleanUp();
        _sampler.cleanUp();
        _pipeline.cleanUp();
        _descriptor_set_layout.cleanUp();
        _shader.cleanUp();

    }

    void WorldRenderer::loadCamera(PerspectiveCamera3D& camera) {

        _uniform_buffer.setAttribute(0, glm::value_ptr(camera.getCameraProjectionMatrix()), 16 * sizeof(float));
        _uniform_buffer.setAttribute(1, glm::value_ptr(camera.getViewMatrix()), 16 * sizeof(float));

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

} // namespace cell