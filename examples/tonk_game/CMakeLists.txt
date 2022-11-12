add_executable(tonk_game

 	src/main.cpp
	src/tonk_game.h
	src/tonk_game.cpp
	src/renderer.h
	src/renderer.cpp

	
	src/map/map.h
	src/map/map.cpp
	src/map/map_generator.h
	src/map/map_generator.cpp
	src/map/tile_map.h
	src/map/tile_map.cpp
	src/map/tile_set.h
	src/map/tile_set.cpp
	src/map/tile_set_file.h
	src/map/tile_set_file.cpp
	src/map/tile.h
	src/map/tile.cpp
	
	src/ui/dev_ui.h
	src/ui/dev_ui.cpp
	
)

target_link_libraries(tonk_game core graphics tools engine)
target_include_directories(tonk_game PUBLIC src)

add_custom_target(run_tonk_game COMMAND gnome-terminal -- ${PROJECT_SOURCE_DIR}/build/examples/tonk_game/tonk_game)

# adding a custom target for building the shaders
add_custom_target(tonk_shader COMMAND bash ${PROJECT_SOURCE_DIR}/examples/tonk_game/src/shader/compile.sh)
add_dependencies(tonk_game tonk_shader)
