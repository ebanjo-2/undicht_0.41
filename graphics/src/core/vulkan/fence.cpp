#include "fence.h"

namespace undicht {

    namespace vulkan {

        void Fence::init(const VkDevice& device, bool init_in_signaled_state) {

            _device_handle = device;

            VkFenceCreateInfo info = createFenceCreateInfo(init_in_signaled_state);
            vkCreateFence(device, &info, {}, &_fence);

        }

        void Fence::cleanUp() {

            vkDestroyFence(_device_handle, _fence, {});

        }

        bool Fence::isInUse() const {

            return _in_use;
        }

        const VkFence& Fence::getFence(bool set_in_use) {

            if(set_in_use)
                _in_use = true;

            return _fence;
        }

        void Fence::reset() {

            vkResetFences(_device_handle, 1, &_fence);

        }

        void Fence::waitForProcessToFinish(bool reset_when_finished, uint32_t max_wait_time) {
            /** if the fence is currently waiting on a gpu process to finish
            * the cpu will now wait until that process has finished
            * @param max_wait_time: time in nanoseconds, 1000000000ns == 1sec*/

            if(_in_use)
                vkWaitForFences(_device_handle, 1, &_fence, true, max_wait_time);

            if(reset_when_finished)
                vkResetFences(_device_handle, 1, &_fence);

        }

        ///////////////////////////// creating fence related structs /////////////////////////////

        VkFenceCreateInfo Fence::createFenceCreateInfo(bool init_signaled) {

            VkFenceCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.pNext = nullptr;

            if(init_signaled)
                info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            return info;
        }

    } // vulkan

} // undicht