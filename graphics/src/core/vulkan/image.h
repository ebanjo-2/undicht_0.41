#ifndef IMAGE_H
#define IMAGE_H

#include "vulkan/vulkan.h"

namespace undicht {

    namespace vulkan {

        class Image {

        protected:

            VkDevice _device_handle;

            VkImage _image;
            VkImageView _image_view;
            VkFormat _format;
            VkExtent2D _extent;
            VkFormat _memory;
            VkSampler _sampler;

            bool _own_image = true;

        public:

            void init(const VkDevice& device);
            void initWithExternalImage(const VkDevice& device, const VkImage& image, const VkFormat& format); // i.e. with an image that is owned by the swap chain
            void cleanUp();

            const VkImageView& getImageView() const;

        protected:
            // creating image related structs

            VkImageViewCreateInfo static createImageViewCreateInfo(const VkImage& image, const VkFormat& format);

        };

    } // vulkan

} // undicht

#endif // IMAGE_H