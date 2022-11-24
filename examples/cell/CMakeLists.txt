
set(WORLD_SOURCES
    src/world/cell.h
    src/world/cell.cpp
    
    src/world/mini_chunk.h
    src/world/mini_chunk.cpp
    src/world/chunk.h
    src/world/chunk.cpp
    src/world/world.h
    src/world/world.cpp
)

add_executable(cell 

    src/main.cpp
    src/app.h 
    src/app.cpp
    
    ${WORLD_SOURCES}
)

target_link_libraries(cell core graphics tools engine)
target_include_directories(cell PUBLIC src)

# adding a custom target for building the shaders
#add_custom_target(tonk_shader COMMAND bash ${PROJECT_SOURCE_DIR}/examples/tonk_game/src/shader/compile.sh)
#add_dependencies(tonk_game tonk_shader)
