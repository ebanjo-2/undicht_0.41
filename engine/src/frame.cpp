#include "frame.h"
#include "vulkan/vulkan.h"

namespace undicht {

    void Frame::init(const vulkan::LogicalDevice& device) {

        _device_handle = device;

        // init sync objects
        _render_finished_fence.init(device.getDevice(), true);
        _swap_image_ready.init(device.getDevice());
        _render_finished_semaphore.init(device.getDevice());
        _transfer_finished_fence.init(device.getDevice(), true);
        _transfer_finished_semaphore.init(device.getDevice());

        // init draw command
        _draw_command.init(device.getDevice(), device.getGraphicsCmdPool());
        _transfer_command.init(device.getDevice(), device.getGraphicsCmdPool());

        // init transfer buffer
        _transfer_buffer.init(device);
    }

    void Frame::cleanUp() {

        // destroy draw command
        _draw_command.cleanUp();
        _transfer_command.cleanUp();

        // destroy transfer buffer
        _transfer_buffer.cleanUp();

        // destroy sync objects
        _render_finished_fence.cleanUp();
        _swap_image_ready.cleanUp();
        _render_finished_semaphore.cleanUp();
        _transfer_finished_fence.cleanUp();
        _transfer_finished_semaphore.cleanUp();

    }

    void Frame::beginFramePreparation() {
        // starts recording of the transfer command buffer

        // technically the program could reach here again before the command has finished execution
        // since its is only waited on when the draw command is waited on
        _transfer_finished_fence.waitForProcessToFinish(true, 1000000000); // 1 sec

        _transfer_command.resetCommandBuffer();
        _transfer_command.beginCommandBuffer(true);

        _transfer_buffer.clearStagedTransfers();

        _frame_in_preparation = true;
    }

    void Frame::endFramePreparation() {
        // submit the transfer command buffer

        if(_frame_in_preparation) {
            // end transfer command
            _transfer_command.endCommandBuffer();
            _device_handle.submitOnGraphicsQueue(_transfer_command.getCommandBuffer(), _transfer_finished_fence.getFence(), {}, {}, {_transfer_finished_semaphore.getAsSignal()});
        }

        _frame_in_preparation = false;
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

        VkSemaphore transfer_finished = _transfer_finished_semaphore.getAsWaitOn();
        if(transfer_finished != VK_NULL_HANDLE) {
            wait_on.push_back(transfer_finished);
            wait_stages.push_back(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
        }

        _device_handle.submitOnGraphicsQueue(_draw_command.getCommandBuffer(), _render_finished_fence.getFence(), wait_on, wait_stages, {_render_finished_semaphore.getAsSignal()});

    }

    vulkan::TransferBuffer& Frame::getTransferBuf() const {

        return (vulkan::TransferBuffer&)_transfer_buffer;
    }

    vulkan::CommandBuffer& Frame::getTransferCmd() const {

        return (vulkan::CommandBuffer&)_transfer_command;
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

    vulkan::Semaphore& Frame::getTransferFinishedSemaphore() const {

        return (vulkan::Semaphore&)_transfer_finished_semaphore;
    }

} // undicht