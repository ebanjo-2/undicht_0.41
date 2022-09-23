#ifndef FENCE_H
#define FENCE_H

#include "vulkan/vulkan.h"

namespace undicht {

    namespace vulkan {

        class Fence {
            /** "fences" are used as barriers to sync gpu and cpu
            * fences can be signaled once certain processes on the gpu have finished
            * the cpu can wait for that to happen */
        protected:

            VkDevice _device_handle;

            VkFence _fence;
            bool _in_use = false;

        public:

            void init(const VkDevice& device, bool init_in_signaled_state = true);
            void cleanUp();

            bool isInUse() const;

            const VkFence& getFence(bool set_in_use = true);

            void reset();

            /** if the fence is currently waiting on a gpu process to finish
             * the cpu will now wait until that process has finished
             * @param max_wait_time: time in nanoseconds, 1000000000ns == 1sec*/
            void waitForProcessToFinish(bool reset_when_finished = true, uint32_t max_wait_time = UINT32_MAX);

        protected:
            // creating fence related structs

            VkFenceCreateInfo static createFenceCreateInfo(bool signaled);

        };

    } // vulkan

} // undicht

#endif // FENCE_H