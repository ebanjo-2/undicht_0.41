#ifndef IMMEDIATE_COMMAND_H
#define IMMEDIATE_COMMAND_H

#include "core/vulkan/logical_device.h"
#include "core/vulkan/command_buffer.h"

namespace undicht {

    namespace vulkan {

        class ImmediateCommand : public CommandBuffer {
          /**
           * @brief a command buffer that is only executed once
           * i.e. for loading data to a texture / vertex buffer
           * will automatically submit itself onto the graphics queue in its destructor
           * after that it will wait for the graphics queue to idle, so use this cautiosly
           * because it can have big affects on performance
           */
          protected:

            LogicalDevice _device_handle;

          public:

            ImmediateCommand(const LogicalDevice& device);
            virtual ~ImmediateCommand();

        };

    } // vulkan

} // undicht

#endif // IMMEDIATE_COMMAND_H