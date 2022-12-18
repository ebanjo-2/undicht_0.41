#include "frame.h"

void Frame::init(const undicht::vulkan::LogicalDevice& device, const std::vector<undicht::vulkan::DescriptorSetLayout>& descriptor_layouts) {

    // init sync objects
    _render_finished_fence.init(device.getDevice(), true);
    _swap_image_ready.init(device.getDevice());
    _render_finished_semaphore.init(device.getDevice());

    // init draw command
    _draw_command.init(device.getDevice(), device.getGraphicsCmdPool());

    // init descriptor caches
    for(const undicht::vulkan::DescriptorSetLayout& layout : descriptor_layouts) {

        undicht::vulkan::DescriptorSetCache cache;
        cache.init(device, layout);
        _descriptor_set_caches.push_back(cache);
    }


}

void Frame::cleanUp() {

    // destroy descriptor caches
    for(undicht::vulkan::DescriptorSetCache& cache : _descriptor_set_caches)
        cache.cleanUp();

    // desctroy draw command
    _draw_command.cleanUp();

    // destroy sync objects
    _render_finished_fence.cleanUp();
    _swap_image_ready.cleanUp();
    _render_finished_semaphore.cleanUp();

}

void Frame::begin() {

    // waiting for the previous rendering to finish
    _render_finished_fence.waitForProcessToFinish(true, 1000000000); // 1 sec

    for(undicht::vulkan::DescriptorSetCache& cache : _descriptor_set_caches)
        cache.reset();

}