#include "frame_manager.h"
#include "vulkan/vulkan.h"
#include "debug.h"

namespace undicht {

    void FrameManager::init(const vulkan::LogicalDevice& device, graphics::Window& window, bool vsync, uint32_t frames_in_flight) {

        _device_handle = device;

        if(vsync) {
            // should be better than VK_PRESENT_MODE_FIFO_KHR
            _swap_chain.init(device, window.getSurface(), VK_PRESENT_MODE_FIFO_RELAXED_KHR); 
        } else {
            _swap_chain.init(device, window.getSurface(), VK_PRESENT_MODE_IMMEDIATE_KHR);
        }

        _frames.resize(frames_in_flight);
        for(Frame& f : _frames) {

            f.init(device);
        }

    }

    void FrameManager::cleanUp() {

        // destroying the swap chain
        _swap_chain.cleanUp();

        for(Frame& f : _frames) {

            f.cleanUp();
        }
    }

    uint32_t FrameManager::getFrameID() const {

        return _frame_id;
    }

    uint32_t FrameManager::getSwapImageID() const {

        return _swap_image_id;
    }

    void FrameManager::beginFramePreperation() {
        // begins the frames transfer cmd (can be used to load resources to the gpu for the frame)
        
        getCurrentFrame().beginFramePreparation();
    } 
    
    void FrameManager::endFramePreperation() {
        // if not called before, this will be called by endFrame()

        getCurrentFrame().endFramePreparation();
    }

    bool FrameManager::beginFrame() {
        /// @brief begin the frame (starts the draw command buffer)
        /// @return true, if the frame was started successfully, false if not (maybe the swap chain is out of date?)

        _swap_image_id = _swap_chain.acquireNextSwapImage(getCurrentFrame().getSwapImageReadySemaphore().getAsSignal());

        if(_swap_image_id == -1) return false; // failed to accquire a swap image

        getCurrentFrame().beginFrame();

        return true;
    }

    void FrameManager::endFrame() {
        /// @brief submits the draw command buffer and the current swap image

        endFramePreperation();

        // submits the frames draw command
        getCurrentFrame().endFrame();

        // submit the swap image
        _device_handle.presentOnPresentQueue(_swap_chain.getSwapchain(), _swap_image_id, {getCurrentFrame().getRenderFinishedSemaphore().getAsWaitOn()});
    
        // advance the frame id
        _frame_id = (_frame_id + 1) % _frames.size();

    }

    Frame& FrameManager::getCurrentFrame() const {
        
        return (Frame&)_frames.at(_frame_id);
    }

    Frame& FrameManager::getPreviousFrame() const {

        if(_frame_id == 0)
            return (Frame&)_frames.back();
        else
            return (Frame&)_frames.at(_frame_id - 1);
    }

    vulkan::CommandBuffer& FrameManager::getTransferCmd() const {

        return getCurrentFrame().getTransferCmd();
    }

    vulkan::TransferBuffer& FrameManager::getTransferBuf() const {

        return getCurrentFrame().getTransferBuf();
    }

    vulkan::CommandBuffer& FrameManager::getDrawCmd() const {

        return getCurrentFrame().getDrawCmd();
    }

    vulkan::SwapChain& FrameManager::getSwapChain() const {

        return (vulkan::SwapChain&)_swap_chain;
    }

    void FrameManager::onWindowResize(const graphics::Window& window) {

        _swap_chain.recreate(window.getSurface());
    }

} // undicht