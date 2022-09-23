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
)

set(GRAPHICS_WINDOW_SOURCES

    src/window/glfw/window_api.h
    src/window/glfw/window_api.cpp

    src/window/glfw/monitor.h
    src/window/glfw/monitor.cpp

    src/window/glfw/window.h
    src/window/glfw/window.cpp
)

set(GRAPHICS_PIPELINE_SOURCES

)

set(GRAPHICS_USER_INTERFACE_SOURCES

)

add_library(graphics

    ${GRAPHICS_WINDOW_SOURCES}
    ${GRAPHICS_CORE_SOURCES}
    ${GRAPHICS_PIPELINE_SOURCES}
    ${GRAPHICS_USER_INTERFACE_SOURCES}

)

target_include_directories(graphics PUBLIC src)

# linking undicht core
target_link_libraries(graphics core)

# linking glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/glfw)
target_link_libraries(graphics glfw)

# linking vulkan
target_include_directories(graphics PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(graphics ${Vulkan_LIBRARIES})

# adding a custom target for building the engines shaders
add_custom_target(shader COMMAND bash ${PROJECT_SOURCE_DIR}/graphics/src/shader/compile.sh)
