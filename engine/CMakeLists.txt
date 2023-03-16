add_library("engine" STATIC

    src/engine.h
    src/engine.cpp
		src/frame_manager.h
		src/frame_manager.cpp
		src/frame.h
		src/frame.cpp
)

target_link_libraries("engine" core graphics tools)

target_include_directories("engine" PUBLIC src)
