#include "semaphore.h"


namespace undicht {

    namespace vulkan {


        void Semaphore::init(const VkDevice& device) {

            _device_handle = device;

            VkSemaphoreCreateInfo info = createSemaphoreCreateInfo();
            vkCreateSemaphore(device, &info, {}, &_semaphore);

        }

        void Semaphore::cleanUp() {

            vkDestroySemaphore(_device_handle, _semaphore, {});

        }

        bool Semaphore::isInUse() const {

            return _in_use;
        }

        VkSemaphore Semaphore::getAsSignal() {
            // the semaphore will be signaled

            _in_use = true;

            return _semaphore;
        }

        VkSemaphore Semaphore::getAsWaitOn() {
            // the semaphore will be waited on

            if(_in_use) {
                _in_use = false;
                return _semaphore;
            } else {
                // the semaphore is not in use, can't be waited on
                return VK_NULL_HANDLE;
            }
        }
        //////////////////////////////// creating semaphore related structs ////////////////////////////////

        VkSemaphoreCreateInfo Semaphore::createSemaphoreCreateInfo() {

            VkSemaphoreCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;

            return info;
        }


    } // vulkan

} // undicht