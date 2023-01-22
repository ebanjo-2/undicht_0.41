#include "image.h"
#include "formats.h"

namespace undicht {

    namespace vulkan {

        void Image::init(const VkDevice& device) {

            _device_handle = device;

            _mem_properties |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        }

        void Image::initWithExternalImage(const VkDevice& device, const VkImage& image, const VkFormat& format) {
            // i.e. with an image that is owned by the swap chain

            _device_handle = device;
            _image = image;
            _own_image = false;
            _format = format;
            _mip_levels = 1;
            _layers = 1;

            // creating the image view
            VkImageViewCreateInfo info = createImageViewCreateInfo(_image, _mip_levels, _layers, _format, chooseImageAspectFlags(_format));
            vkCreateImageView(_device_handle, &info, {}, &_image_view);

        }

        void Image::cleanUp() {

            if(_own_image) {
                vkDestroyImage(_device_handle, _image, {});
                vkFreeMemory(_device_handle, _memory, {});
            }

            vkDestroyImageView(_device_handle, _image_view, {});

        }

        const VkImage& Image::getImage() const {

            return _image;
        }

        const VkImageView& Image::getImageView() const {

            return _image_view;
        }

        const VkExtent3D& Image::getExtent() const {

            return _extent;
        }

        const VkFormat& Image::getFormat() const {

            return _format;
        }

        void Image::allocate(const LogicalDevice& device, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t mip_levels, VkFormat format) {
            
            _format = format;
            _extent = {width, height, depth};
            _layers = layers;
            _mip_levels = mip_levels;
            
            // freeing previously allocated memory 
            if(_image != VK_NULL_HANDLE) vkDestroyImage(_device_handle, _image, {});
            if(_image_view != VK_NULL_HANDLE) vkDestroyImageView(_device_handle, _image_view, {});
            if(_memory != VK_NULL_HANDLE) vkFreeMemory(_device_handle, _memory, {});

            _own_image = true;

            // creating the image
            VkImageCreateInfo info = createImageCreateInfo(_extent, layers, mip_levels, format, chooseImageUsageFlags(_format));
            vkCreateImage(_device_handle, &info, {}, &_image);

            // getting the drivers? requirements needed for the texture
            VkMemoryRequirements mem_requirements;
            vkGetImageMemoryRequirements(_device_handle, _image, &mem_requirements);

            // declaring the memory type
            VkMemoryType mem_type{};
            mem_type.heapIndex = mem_requirements.memoryTypeBits;
            mem_type.propertyFlags = _mem_properties;
            uint32_t mem_type_index = device.getMemoryTypeIndex(mem_type);

            // allocating the memory
            VkMemoryAllocateInfo allocate_info = createMemoryAllocateInfo(mem_requirements.size, mem_type_index);
            vkAllocateMemory(_device_handle, &allocate_info, {}, &_memory);

            // binding the memory to the image
            vkBindImageMemory(_device_handle, _image, _memory, 0);

            // creating the image view
            VkImageViewCreateInfo image_view_info = createImageViewCreateInfo(_image, _mip_levels, _layers, _format, chooseImageAspectFlags(_format));
            vkCreateImageView(_device_handle, &image_view_info, {}, &_image_view);

        }

        //////////////////////////// creating image related structs ////////////////////////////

        VkImageCreateInfo Image::createImageCreateInfo(VkExtent3D extent, uint32_t layers, uint32_t mip_levels, VkFormat format, VkImageUsageFlags usage) {

            VkImageCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            info.extent = extent;
            info.imageType = extent.depth == 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D;
            info.arrayLayers = layers;
            info.mipLevels = mip_levels;
            info.format = format;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;
            info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            info.usage = usage;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // used exclusively by the graphics queue
            info.samples = VK_SAMPLE_COUNT_1_BIT; // used for multisampling

            return info;
        }

        VkImageViewCreateInfo Image::createImageViewCreateInfo(const VkImage& image, uint32_t mip_levels, uint32_t layer_count, const VkFormat& format, VkImageAspectFlags flags) {

            VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D;

            if(layer_count > 1) view_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

            VkImageViewCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image = image;
            info.viewType = view_type;
            info.format = format;
            info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.subresourceRange.aspectMask = flags;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = mip_levels;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = layer_count;
        
            return info;
        }

        VkMemoryAllocateInfo Image::createMemoryAllocateInfo(uint32_t byte_size, uint32_t mem_type_index) {

            VkMemoryAllocateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            info.pNext = nullptr;
            info.allocationSize = byte_size;
            info.memoryTypeIndex = mem_type_index;

            return info;
        }

        VkImageSubresourceRange Image::createImageSubresourceRange(VkImageAspectFlags flags, uint32_t mip_levels, uint32_t layer_count) {

            VkImageSubresourceRange range{};
		    range.aspectMask = flags;
		    range.baseMipLevel = 0;
		    range.levelCount = mip_levels;
		    range.baseArrayLayer = 0;
		    range.layerCount = layer_count;

            return range;
        }

        VkImageMemoryBarrier Image::createImageMemoryBarrier(VkImage image, VkImageSubresourceRange range, VkImageLayout old_layout, VkImageLayout new_layout, VkAccessFlags src_access, VkAccessFlags dst_access) {
            // used to transition an image from one layout the another one

            VkImageMemoryBarrier barrier{};
		    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		    barrier.oldLayout = old_layout;
		    barrier.newLayout = new_layout;
		    barrier.image = image;
		    barrier.subresourceRange = range;
		    barrier.srcAccessMask = src_access;
		    barrier.dstAccessMask = dst_access;

            return barrier;
        }

        VkBufferImageCopy Image::createBufferImageCopy(VkExtent3D image_extent, VkOffset3D image_offset, VkImageAspectFlags flags, uint32_t layer) {

            VkBufferImageCopy region{};
            region.bufferOffset = 0; // layout of the data in the buffer
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = flags;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = layer;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = image_offset;
            region.imageExtent = image_extent;

            return region;
        }

        VkImageBlit Image::createImageBlit(int src_width, int src_height, uint32_t src_mip_level, uint32_t dst_mip_level) {

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = {src_width, src_height, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = src_mip_level;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = {src_width > 1 ? src_width / 2 : 1, src_height > 1 ? src_height / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = dst_mip_level;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            return blit;
        }


        VkImageAspectFlags Image::chooseImageAspectFlags(const VkFormat& format) {

            FixedType translated_type = translate(format);

            if(translated_type.m_type == Type::DEPTH_BUFFER)
                return VK_IMAGE_ASPECT_DEPTH_BIT;

            if(translated_type.m_type == Type::DEPTH_STENCIL_BUFFER)
                return VK_IMAGE_ASPECT_DEPTH_BIT;

            return VK_IMAGE_ASPECT_COLOR_BIT;
        }

        VkImageUsageFlags Image::chooseImageUsageFlags(const VkFormat& format) {

            FixedType translated_type = translate(format);

            if(translated_type.m_type == Type::DEPTH_BUFFER)
                return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

            if(translated_type.m_type == Type::DEPTH_STENCIL_BUFFER)
                return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

            // default for color images
            return VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }


    }

} // undicht