#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "vulkan/vulkan.h"

namespace undicht {

    namespace vulkan {

        class Semaphore {
            /** "semaphores" are used as barriers to sync processes on the gpu
             * many commands that run on the gpu have the option to set a signal and a wait_on semaphore
             * additionally a point at which the command should wait on the wait_on semaphore can be defined */
        protected:

            VkDevice _device_handle;

            VkSemaphore _semaphore;
            bool _in_use = false;

        public:

            void init(const VkDevice& device);
            void cleanUp();

            bool isInUse() const;

            VkSemaphore getAsSignal();
            VkSemaphore getAsWaitOn();

        protected:
            // creating semaphore related structs

            VkSemaphoreCreateInfo static createSemaphoreCreateInfo();

        };

    } // vulkan

} // undicht

#endif // SEMAPHORE_H