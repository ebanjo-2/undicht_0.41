
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

set(ENTITY_SOURCES
    src/entities/light_buffer.h
    src/entities/light_buffer.cpp
)

set(RENDERER_SOURCES
    src/renderer/master_renderer.h
	src/renderer/master_renderer.cpp
	src/renderer/world_renderer.h
	src/renderer/world_renderer.cpp
	src/renderer/world_buffer.h
    src/renderer/world_buffer.cpp
    src/renderer/drawable_world.h
    src/renderer/drawable_world.cpp
    src/renderer/final_renderer.h
    src/renderer/final_renderer.cpp
    src/renderer/light_renderer.h
    src/renderer/light_renderer.cpp
)

set(MATERIALS_SOURCES
    src/materials/material.h
    src/materials/material.cpp
    src/materials/material_atlas.h
    src/materials/material_atlas.cpp
)

set(PLAYER_SOURCES
	src/player/player.h
	src/player/player.cpp
)

add_executable(cell 

    src/main.cpp
    src/app.h 
    src/app.cpp
    
    ${WORLD_SOURCES}
    ${ENTITY_SOURCES}
    ${RENDERER_SOURCES}
    ${MATERIALS_SOURCES}
    ${PLAYER_SOURCES}
)

target_link_libraries(cell core graphics tools engine)
target_include_directories(cell PUBLIC src)

# adding a custom target for building the shaders
add_custom_target(cell_shader COMMAND bash ${PROJECT_SOURCE_DIR}/examples/cell/src/renderer/shader/compile.sh)
add_dependencies(cell cell_shader)
