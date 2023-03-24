#include "immediate_command.h"

namespace undicht {

    namespace vulkan {

        ImmediateCommand::ImmediateCommand(const LogicalDevice& device) {

            _device_handle = device;

            CommandBuffer::init(device.getDevice(), device.getGraphicsCmdPool());
            CommandBuffer::beginCommandBuffer(true);

        }

        ImmediateCommand::~ImmediateCommand() {
            // submit onto a queue, wait for execution to finish and clean up

            CommandBuffer::endCommandBuffer();
            _device_handle.submitOnGraphicsQueue(getCommandBuffer());
            _device_handle.waitGraphicsQueueIdle();
            CommandBuffer::cleanUp();
        }

    } // vulkan

} // undicht