#ifndef FRAME_H
#define FRAME_H

#include "core/vulkan/logical_device.h"
#include "core/vulkan/semaphore.h"
#include "core/vulkan/fence.h"
#include "core/vulkan/command_buffer.h"

#include "renderer/vulkan/transfer_buffer.h"

namespace undicht {

    class Frame {

      protected:

        vulkan::LogicalDevice _device_handle;

        vulkan::Fence _render_finished_fence;
        vulkan::Semaphore _swap_image_ready;
        vulkan::Semaphore _render_finished_semaphore;
        vulkan::Fence _transfer_finished_fence;
        vulkan::Semaphore _transfer_finished_semaphore;

        vulkan::CommandBuffer _transfer_command;
        vulkan::CommandBuffer _draw_command;

        vulkan::TransferBuffer _transfer_buffer;

        bool _frame_in_preparation = false;

      public:

        void init(const undicht::vulkan::LogicalDevice& device);
        void cleanUp();

        /** the per frame transfer buffer can be used to update 
         * i.e. vertex buffers and textures that need to be ready before drawing */
        void beginFramePreparation(); // starts recording of the transfer command buffer
        void endFramePreparation(); // submit the transfer command buffer
        void beginFrame(); // starts recording of the draw command buffer 
        void endFrame(); // ends and submits the draw command buffer (will wait for the transfer buffer to finish)

        vulkan::TransferBuffer& getTransferBuf() const;
        vulkan::CommandBuffer& getTransferCmd() const;
        vulkan::CommandBuffer& getDrawCmd() const;
        vulkan::Fence& getRenderFinishedFence() const;
        vulkan::Semaphore& getSwapImageReadySemaphore() const;
        vulkan::Semaphore& getRenderFinishedSemaphore() const;
        vulkan::Semaphore& getTransferFinishedSemaphore() const;

    };

} // undicht

#endif // FRAME_H