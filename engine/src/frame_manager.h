#ifndef FRAME_MANAGER_H
#define FRAME_MANAGER_H

#include "frame.h"
#include "vector"
#include "core/vulkan/swap_chain.h"
#include "window/glfw/window.h"

namespace undicht {

    class FrameManager {

      protected:

        vulkan::LogicalDevice _device_handle;

        std::vector<Frame> _frames;

        uint32_t _swap_image_id = 0;
        uint32_t _frame_id = -1;

        vulkan::SwapChain _swap_chain;

      public:

        void init(const vulkan::LogicalDevice& device, graphics::Window& window, bool vsync = true, uint32_t frames_in_flight = 1);
        void cleanUp();

        /// @brief begin the frame (starts the draw command buffer)
        /// @return true, if the frame was started successfully, false if not (maybe the swap chain is out of date?)
        bool beginFrame();

        /// @brief submits the draw command buffer and the current swap image
        void endFrame();

        uint32_t getFrameID() const;
        Frame& getCurrentFrame() const;
        Frame& getPreviousFrame() const;
        vulkan::CommandBuffer& getDrawCmd() const;
        vulkan::SwapChain& getSwapChain() const;

        void onWindowResize(const graphics::Window& window);

    };

} // undicht

#endif // FRAME_MANAGER_H