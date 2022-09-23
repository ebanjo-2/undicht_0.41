#include "image.h"

namespace undicht {

    namespace vulkan {

        void Image::init(const VkDevice& device) {

            _device_handle = device;

        }

        void Image::initWithExternalImage(const VkDevice& device, const VkImage& image, const VkFormat& format) {
            // i.e. with an image that is owned by the swap chain

            _device_handle = device;
            _image = image;
            _own_image = false;
            _format = format;

            // creating the image view
            VkImageViewCreateInfo info = createImageViewCreateInfo(_image, _format);
            vkCreateImageView(_device_handle, &info, {}, &_image_view);

        }

        void Image::cleanUp() {

            if(_own_image)
                vkDestroyImage(_device_handle, _image, {});

            vkDestroyImageView(_device_handle, _image_view, {});

        }

        const VkImageView& Image::getImageView() const {

            return _image_view;
        }


        //////////////////////////// creating image related structs ////////////////////////////

        VkImageViewCreateInfo Image::createImageViewCreateInfo(const VkImage& image, const VkFormat& format) {

            VkImageViewCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image = image;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = format;
            info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;

            return info;
        }

    }

} // undicht