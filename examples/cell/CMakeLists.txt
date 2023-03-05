
set(WORLD_SOURCES
    src/world/drawable_world.h
    src/world/drawable_world.cpp

    src/world/chunk_system/chunk_system.h
    src/world/chunk_system/chunk_system.cpp
    src/world/chunk_system/chunk.h
    src/world/chunk_system/chunk.cpp
    src/world/chunk_system/chunk_buffer.h
    src/world/chunk_system/chunk_buffer.cpp
    
    src/world/cells/cell.h
    src/world/cells/cell.cpp
    src/world/cells/mini_chunk.h
    src/world/cells/mini_chunk.cpp
    src/world/cells/cell_chunk.h
    src/world/cells/cell_chunk.cpp
    src/world/cells/cell_world.h
    src/world/cells/cell_world.cpp
    src/world/cells/cell_buffer.h
    src/world/cells/cell_buffer.cpp
    
    src/world/lights/light.h
    src/world/lights/light.cpp
    src/world/lights/light_chunk.h
    src/world/lights/light_chunk.cpp
    src/world/lights/light_world.h
    src/world/lights/light_world.cpp
    src/world/lights/light_buffer.h
    src/world/lights/light_buffer.cpp
)


set(RENDERER_SOURCES
    src/renderer/master_renderer.h
	src/renderer/master_renderer.cpp
	src/renderer/world_renderer.h
	src/renderer/world_renderer.cpp
    src/renderer/final_renderer.h
    src/renderer/final_renderer.cpp
    src/renderer/light_renderer.h
    src/renderer/light_renderer.cpp
    src/renderer/shadow_renderer.h
    src/renderer/shadow_renderer.cpp
    src/renderer/brdf_integration_map.h
    src/renderer/brdf_integration_map.cpp
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

set(FILES_SOURCES
	src/files/world_file.h
	src/files/world_file.cpp
	src/files/material_file.h
	src/files/material_file.cpp
	src/files/chunk_file.h
	src/files/chunk_file.cpp
)

set(ENVIRONMENT_SOURCES
	src/environment/environment.h
	src/environment/environment.cpp
	src/environment/environment_generator.h
	src/environment/environment_generator.cpp
	src/environment/cloud_layer.h
	src/environment/cloud_layer.cpp
)

add_executable(cell 

    src/main.cpp
    src/app.h 
    src/app.cpp
    
    ${WORLD_SOURCES}
    ${RENDERER_SOURCES}
    ${MATERIALS_SOURCES}
    ${PLAYER_SOURCES}
    ${FILES_SOURCES}
    ${ENVIRONMENT_SOURCES}
)

target_link_libraries(cell core graphics tools engine)
target_include_directories(cell PUBLIC src)


# adding a custom target for building the shaders
add_custom_target(cell_shader COMMAND python3 ${SHADER_COMPILE_SCRIPT} "${PROJECT_SOURCE_DIR}/examples/cell/src/renderer/shader")
add_dependencies(cell cell_shader)
