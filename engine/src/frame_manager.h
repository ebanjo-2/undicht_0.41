#ifndef FRAME_MANAGER_H
#define FRAME_MANAGER_H

#include "frame.h"
#include "vector"
#include "core/vulkan/swap_chain.h"
#include "window/glfw/window.h"
#include "core/vulkan/command_buffer.h"
#include "renderer/vulkan/transfer_buffer.h"

namespace undicht {

    class FrameManager {

      protected:

        vulkan::LogicalDevice _device_handle;

        std::vector<Frame> _frames;

        uint32_t _swap_image_id = 0;
        uint32_t _frame_id = 0;

        vulkan::SwapChain _swap_chain;

      public:

        void init(const vulkan::LogicalDevice& device, graphics::Window& window, bool vsync = true, uint32_t frames_in_flight = 1);
        void cleanUp();

        void beginFramePreperation(); // begins the frames transfer cmd (can be used to load resources to the gpu for the frame)
        void endFramePreperation(); // if not called before, this will be called by endFrame()

        /// @brief begin the frame (starts the draw command buffer)
        /// @return true, if the frame was started successfully, false if not (maybe the swap chain is out of date?)
        bool beginFrame();

        /// @brief submits the draw command buffer and the current swap image
        void endFrame();

        uint32_t getFrameID() const;
        uint32_t getSwapImageID() const;
        Frame& getCurrentFrame() const;
        Frame& getPreviousFrame() const;
        vulkan::CommandBuffer& getTransferCmd() const;
        vulkan::TransferBuffer& getTransferBuf() const;
        vulkan::CommandBuffer& getDrawCmd() const;
        vulkan::SwapChain& getSwapChain() const;

        void onWindowResize(const graphics::Window& window);

    };

} // undicht

#endif // FRAME_MANAGER_H