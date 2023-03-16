#include "frame.h"
#include "vulkan/vulkan.h"

namespace undicht {

    void Frame::init(const vulkan::LogicalDevice& device) {

        _device_handle = device;

        // init sync objects
        _render_finished_fence.init(device.getDevice(), true);
        _swap_image_ready.init(device.getDevice());
        _render_finished_semaphore.init(device.getDevice());

        // init draw command
        _draw_command.init(device.getDevice(), device.getGraphicsCmdPool());

    }

    void Frame::cleanUp() {

        // destroy draw command
        _draw_command.cleanUp();

        // destroy sync objects
        _render_finished_fence.cleanUp();
        _swap_image_ready.cleanUp();
        _render_finished_semaphore.cleanUp();

    }

    void Frame::beginFrame() {

        // waiting for the previous rendering to finish
        _render_finished_fence.waitForProcessToFinish(true, 1000000000); // 1 sec

        // begin draw command buffer
        _draw_command.resetCommandBuffer();
        _draw_command.beginCommandBuffer(true);

    }

    void Frame::endFrame() {

        // end command buffer
        _draw_command.endCommandBuffer();

        std::vector<VkSemaphore> wait_on = {_swap_image_ready.getAsWaitOn()};
        std::vector<VkPipelineStageFlags> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};


        _device_handle.submitOnGraphicsQueue(_draw_command.getCommandBuffer(), _render_finished_fence.getFence(), wait_on, wait_stages, {_render_finished_semaphore.getAsSignal()});

    }

    vulkan::CommandBuffer& Frame::getDrawCmd() const {              
        
        return (vulkan::CommandBuffer&)_draw_command;
    }

    vulkan::Fence& Frame::getRenderFinishedFence() const {
        
        return (vulkan::Fence&)_render_finished_fence;
    }

    vulkan::Semaphore& Frame::getSwapImageReadySemaphore() const {
        
        return (vulkan::Semaphore&)_swap_image_ready;
    }

    vulkan::Semaphore& Frame::getRenderFinishedSemaphore() const {
        
        return (vulkan::Semaphore&)_render_finished_semaphore;
    }

} // undicht