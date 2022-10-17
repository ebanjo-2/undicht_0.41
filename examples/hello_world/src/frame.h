#ifndef FRAME_H
#define FRAME_H

#include "core/vulkan/semaphore.h"
#include "core/vulkan/fence.h"
#include "core/vulkan/logical_device.h"
#include "core/vulkan/image.h"
#include "core/vulkan/swap_chain.h"

#include "renderer/vulkan/descriptor_set_cache.h"

class Frame {

public:

    undicht::vulkan::Fence _render_finished_fence;
    undicht::vulkan::Semaphore _swap_image_ready;
    undicht::vulkan::Semaphore _render_finished_semaphore;

    undicht::vulkan::CommandBuffer _draw_command;

    std::vector<undicht::vulkan::DescriptorSetCache> _descriptor_set_caches;

    void init(const undicht::vulkan::LogicalDevice& device, const std::vector<undicht::vulkan::DescriptorSetLayout>& descriptor_layouts = {});
    void cleanUp();

    void begin();

};


#endif // FRAME_H