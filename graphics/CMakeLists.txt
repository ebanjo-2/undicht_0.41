find_package(Vulkan REQUIRED)

set(GRAPHICS_CORE_SOURCES

    src/core/vulkan/instance.h
    src/core/vulkan/instance.cpp 

    src/core/vulkan/logical_device.h
    src/core/vulkan/logical_device.cpp 
    
    src/core/vulkan/swap_chain.h
    src/core/vulkan/swap_chain.cpp 
    
    src/core/vulkan/image.h
    src/core/vulkan/image.cpp 
    
    src/core/vulkan/command_buffer.h
    src/core/vulkan/command_buffer.cpp
    
    src/core/vulkan/renderpass.h
    src/core/vulkan/renderpass.cpp
    
    src/core/vulkan/framebuffer.h
    src/core/vulkan/framebuffer.cpp
    
    src/core/vulkan/fence.h
    src/core/vulkan/fence.cpp
    
    src/core/vulkan/semaphore.h
    src/core/vulkan/semaphore.cpp
    
    src/core/vulkan/shader.h
    src/core/vulkan/shader.cpp
    
    src/core/vulkan/pipeline.h
    src/core/vulkan/pipeline.cpp
    
    src/core/vulkan/buffer.h
    src/core/vulkan/buffer.cpp
    
    src/core/vulkan/formats.h
    src/core/vulkan/formats.cpp
    
    src/core/vulkan/descriptor_set.h
    src/core/vulkan/descriptor_set.cpp
    
    src/core/vulkan/descriptor_pool.h
    src/core/vulkan/descriptor_pool.cpp
    
    src/core/vulkan/descriptor_set_layout.h
    src/core/vulkan/descriptor_set_layout.cpp
    
    src/core/vulkan/sampler.h
    src/core/vulkan/sampler.cpp
    
)

set(GRAPHICS_WINDOW_SOURCES

    src/window/glfw/window_api.h
    src/window/glfw/window_api.cpp

    src/window/glfw/monitor.h
    src/window/glfw/monitor.cpp

    src/window/glfw/window.h
    src/window/glfw/window.cpp
)

set(GRAPHICS_RENDERER_SOURCES
		src/renderer/vulkan/vertex_buffer.h
	  src/renderer/vulkan/vertex_buffer.cpp
	  
		src/renderer/vulkan/uniform_buffer.h
	  src/renderer/vulkan/uniform_buffer.cpp
	  
	  src/renderer/vulkan/texture.h
	  src/renderer/vulkan/texture.cpp
	  
	  src/renderer/vulkan/descriptor_set_cache.h
	  src/renderer/vulkan/descriptor_set_cache.cpp
	  
	  src/renderer/vulkan/renderer.h
	  src/renderer/vulkan/renderer.cpp
	  
	  src/renderer/vulkan/render_target.h
	  src/renderer/vulkan/render_target.cpp
	  
	  src/renderer/vulkan/immediate_command.h
	  src/renderer/vulkan/immediate_command.cpp
	  
	  src/renderer/vulkan/transfer_buffer.h
	  src/renderer/vulkan/transfer_buffer.cpp
)

set(GRAPHICS_USER_INTERFACE_SOURCES

)

set(GRAPHICS_IMGUI_SOURCES
		src/imgui/vulkan/imgui_api.h
		src/imgui/vulkan/imgui_api.cpp
)

add_library(graphics

    ${GRAPHICS_WINDOW_SOURCES}
    ${GRAPHICS_CORE_SOURCES}
    ${GRAPHICS_RENDERER_SOURCES}
    ${GRAPHICS_USER_INTERFACE_SOURCES}
	${GRAPHICS_IMGUI_SOURCES}
)

target_include_directories(graphics PUBLIC src)

# setting the shader compile script path
set(SHADER_COMPILE_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/src/shader/compile.py CACHE STRING "SHADER_COMPILE_SCRIPT") 


# linking undicht core
target_link_libraries(graphics core)

# linking glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/glfw)
target_link_libraries(graphics glfw)

#linking imgui
set(IMGUI_PATH  "extern/imgui")
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp) 
file(GLOB IMGUI_BACKEND ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp ${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp) 
add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_BACKEND})
target_include_directories(imgui PUBLIC extern/imgui ${Vulkan_INCLUDE_DIRS})
target_link_libraries(imgui ${Vulkan_LIBRARIES} glfw)

target_link_libraries(graphics imgui)

# linking vulkan
target_include_directories(graphics PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(graphics ${Vulkan_LIBRARIES})

# linking vma
add_subdirectory(extern/vma)
target_link_libraries(graphics vma)


# adding a custom target for building the engines shaders
add_custom_target(shader COMMAND python3 ${SHADER_COMPILE_SCRIPT} "${PROJECT_SOURCE_DIR}/graphics/src/shader")
add_dependencies(graphics shader)
