set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

# 3.7 is required to find vulkan
cmake_minimum_required(VERSION 3.7)

# creating the project
project(undicht)

# setting cpp standard
set (CMAKE_CXX_STANDARD 11)

# setting compiles flags
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# adding libraries
add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(tools)
add_subdirectory(engine)

# adding examples
add_subdirectory(examples/hello_world)
add_subdirectory(examples/tonk_game)
