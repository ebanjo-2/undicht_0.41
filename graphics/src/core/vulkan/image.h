#ifndef IMAGE_H
#define IMAGE_H

#include "vulkan/vulkan.h"
#include "logical_device.h"

namespace undicht {

    namespace vulkan {

        class Image {

        protected:

            VkDevice _device_handle;

            VkImage _image = VK_NULL_HANDLE;
            VkDeviceMemory _memory = VK_NULL_HANDLE;
            VkMemoryPropertyFlags _mem_properties{};
            VkImageView _image_view;

            VkFormat _format;
            VkExtent3D _extent;
            uint32_t _layers;

            bool _own_image = true;

        public:

            void init(const VkDevice& device);
            void initWithExternalImage(const VkDevice& device, const VkImage& image, const VkFormat& format); // i.e. with an image that is owned by the swap chain
            void cleanUp();

            const VkImage& getImage() const;
            const VkImageView& getImageView() const;
            const VkExtent3D& getExtent() const;

            void allocate(const LogicalDevice& device, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, VkFormat format);

        protected:
            // creating image related structs

            VkImageCreateInfo static createImageCreateInfo(VkExtent3D extent, uint32_t layers, VkFormat format);
            VkImageViewCreateInfo static createImageViewCreateInfo(const VkImage& image, const VkFormat& format);
            VkMemoryAllocateInfo static createMemoryAllocateInfo(uint32_t byte_size, uint32_t mem_type_index);

        public:

            VkImageSubresourceRange static createImageSubresourceRange();
            VkImageMemoryBarrier static createImageMemoryBarrier(VkImage image, VkImageSubresourceRange range, VkImageLayout old_layout, VkImageLayout new_layout, VkAccessFlags src_access, VkAccessFlags dst_access);
            VkBufferImageCopy static createBufferImageCopy(VkExtent3D extent);
        };

    } // vulkan

} // undicht

#endif // IMAGE_H