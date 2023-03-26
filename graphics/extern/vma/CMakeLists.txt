find_package(Vulkan REQUIRED)

add_library(vma
	# the only purpose of this library is the implementation of vma
	src/vma_implementation.cpp
	src/vma_global_allocator.h
)

target_include_directories(vma PUBLIC src)

# adding the vma directory (included in the vulkan sdk)
target_include_directories(vma PRIVATE ${Vulkan_INCLUDE_DIRS})