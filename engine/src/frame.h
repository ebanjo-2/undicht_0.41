#ifndef FRAME_H
#define FRAME_H

#include "core/vulkan/logical_device.h"
#include "core/vulkan/semaphore.h"
#include "core/vulkan/fence.h"

namespace undicht {

    class Frame {

      protected:

        vulkan::LogicalDevice _device_handle;

        vulkan::Fence _render_finished_fence;
        vulkan::Semaphore _swap_image_ready;
        vulkan::Semaphore _render_finished_semaphore;

        vulkan::CommandBuffer _draw_command;

      public:

        void init(const undicht::vulkan::LogicalDevice& device);
        void cleanUp();

        void beginFrame(); // starts recording of the draw command buffer
        void endFrame(); // ends and submits the draw command buffer

        vulkan::CommandBuffer& getDrawCmd() const;
        vulkan::Fence& getRenderFinishedFence() const;
        vulkan::Semaphore& getSwapImageReadySemaphore() const;
        vulkan::Semaphore& getRenderFinishedSemaphore() const;

    };

} // undicht

#endif // FRAME_H