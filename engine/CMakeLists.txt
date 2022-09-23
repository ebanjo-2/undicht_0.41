add_library("engine" STATIC

    src/engine.h
    src/engine.cpp

)

target_link_libraries("engine" core graphics tools)

target_include_directories("engine" PUBLIC src)
