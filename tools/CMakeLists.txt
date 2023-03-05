add_library("tools" 

	src/images/image_file.h
	src/images/image_file.cpp
	src/images/image_data.h
	src/images/image_data.cpp
	src/images/cube_map_data.h
	src/images/cube_map_data.cpp
	src/images/image_data_3d.h
	src/images/image_data_3d.cpp
	
	src/math/math_tools.h
	src/math/orientation_3d.h
	src/math/orientation_3d.cpp
	src/math/orthographic_projection.h
	src/math/orthographic_projection.cpp
	src/math/perspective_projection.h
	src/math/perspective_projection.cpp
	
	src/3D/camera/camera_3d.h
	src/3D/camera/camera_3d.cpp
	src/3D/camera/perspective_camera_3d.h
	src/3D/camera/perspective_camera_3d.cpp
	
	src/xml/xml_tag_attribute.h
	src/xml/xml_tag_attribute.cpp
	src/xml/xml_file.h
	src/xml/xml_file.cpp
	src/xml/xml_element.h
	src/xml/xml_element.cpp
	
	src/model_loading/model_loader.h
	src/model_loading/model_loader.cpp
	src/model_loading/collada/collada_file.h
	src/model_loading/collada/collada_file.cpp
	src/model_loading/obj/obj_file.h
	src/model_loading/obj/obj_file.cpp
	src/model_loading/obj/mtl_file.h
	src/model_loading/obj/mtl_file.cpp
	
	src/file_tools.h
	src/file_tools.cpp
	
	src/IBL/ibl.h
	src/IBL/ibl.cpp
	
	src/binary_data/binary_data_buffer.h
	src/binary_data/binary_data_buffer.cpp
	src/binary_data/binary_data_file.h
	src/binary_data/binary_data_file.cpp
	
	extern/stb_implementation.cpp
)

target_include_directories("tools" PUBLIC src extern/glm)


add_subdirectory(extern/stb)

target_link_libraries("tools" PUBLIC core graphics stb_image)
