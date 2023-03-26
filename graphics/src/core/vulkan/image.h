#ifndef IMAGE_H
#define IMAGE_H

#include "vulkan/vulkan.h"
#include "logical_device.h"

#include "vk_mem_alloc.h"

namespace undicht {

    namespace vulkan {

        class Image {

        protected:

            VkDevice _device_handle = VK_NULL_HANDLE;

            VkImage _image = VK_NULL_HANDLE;
            /*VkDeviceMemory _memory = VK_NULL_HANDLE;
            VkMemoryPropertyFlags _mem_properties{};*/
            VmaAllocation _memory;
            VkImageView _image_view = VK_NULL_HANDLE;

            VkFormat _format = VK_FORMAT_UNDEFINED;
            VkExtent3D _extent = {};
            uint32_t _layers = 1;
            uint32_t _mip_levels = 1;
            bool _is_cube_map = false;

            bool _own_image = true;

        public:

            void init(const VkDevice& device, bool is_cube_map = false);
            void initWithExternalImage(const VkDevice& device, const VkImage& image, const VkFormat& format); // i.e. with an image that is owned by the swap chain
            void cleanUp();

            const VkImage& getImage() const;
            const VkImageView& getImageView() const;
            const VkExtent3D& getExtent() const;
            const VkFormat& getFormat() const;

            void allocate(const LogicalDevice& device, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t mip_levels, VkFormat format);

        protected:
            // creating image related structs

            VkImageCreateInfo static createImageCreateInfo(VkExtent3D extent, uint32_t layers, uint32_t mip_levels, VkFormat format, VkImageUsageFlags usage, VkImageCreateFlags flags);
            VkImageViewCreateInfo static createImageViewCreateInfo(const VkImage& image, uint32_t mip_levels, uint32_t layer_count, VkExtent3D extent, bool cube_map, const VkFormat& format, VkImageAspectFlags flags);
            VkMemoryAllocateInfo static createMemoryAllocateInfo(uint32_t byte_size, uint32_t mem_type_index);

        public:

            VkImageSubresourceRange static createImageSubresourceRange(VkImageAspectFlags flags, uint32_t base_mip_level, uint32_t num_mip_levels, uint32_t base_layer, uint32_t layer_count);
            VkImageMemoryBarrier static createImageMemoryBarrier(VkImage image, VkImageSubresourceRange range, VkImageLayout old_layout, VkImageLayout new_layout, VkAccessFlags src_access, VkAccessFlags dst_access);
            VkBufferImageCopy static createBufferImageCopy(VkExtent3D image_extent, VkOffset3D image_offset, VkImageAspectFlags flags, uint32_t layer = 0, uint32_t mip_level = 0, uint32_t src_offset = 0);
            VkImageBlit static createImageBlit(int src_width, int src_height, uint32_t src_mip_level, uint32_t dst_mip_level);

            VkImageAspectFlags static chooseImageAspectFlags(const VkFormat& format);
            VkImageUsageFlags static chooseImageUsageFlags(const VkFormat& format);
        };

    } // vulkan

} // undicht

#endif // IMAGE_H