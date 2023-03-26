#ifndef VMA_GLOBAL_ALLOCATOR_H
#define VMA_GLOBAL_ALLOCATOR_H

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

namespace undicht {

	namespace vma {
		
		// the vma documentation says i should create a global allocator
		class GlobalAllocator {
			
		  protected:
		  	
		  	static VmaAllocator global_allocator;

		  public:

			static void init(VkInstance instance, VkDevice device, VkPhysicalDevice physical_device);
			static void cleanUp();

			static VmaAllocator& get();
		
		};
	
	} // vma

} // undicht

#endif // VMA_GLOBAL_ALLOCATOR_H
