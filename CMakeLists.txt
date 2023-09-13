cmake_minimum_required(VERSION 3.27.0)

project(conan-imgui-avi-player LANGUAGES CXX C VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)

find_package(Eigen3 REQUIRED)
find_package(ffmpeg REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(imgui REQUIRED)

if(WIN32) 
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:100000000")
find_package(freeglut REQUIRED)
find_package(opengl REQUIRED)
find_package(zlib REQUIRED)
endif(WIN32) 

find_package(opencv REQUIRED)

# find Matlab's matrix and mex libraries
find_package(Matlab COMPONENTS MAT_LIBRARY MX_LIBRARY)
if (NOT Matlab_FOUND)
    message(FATAL_ERROR "Matlab dependencies not found. Is the MATLAB_PATH environment variable set?")
endif()

# build the mex
matlab_add_mex(
    NAME imguiAviMex
    SHARED 
	SRC  bindings/imgui_impl_glfw.cpp
         bindings/imgui_impl_glfw.h
         bindings/imgui_impl_opengl3.cpp
         bindings/imgui_impl_opengl3.h
         bindings/imgui_impl_opengl3_loader.h 
 	     source/Decoder.cpp
	     source/imguiAviMex.cpp
	     source/ImGuiPlayer.cpp
		 source/WebCam.cpp
	     include/Decoder.h
	     include/ImGuiPlayer.h
	     include/TinyLib.h
	     include/PlayAvi.h
		 include/WebCam.h
		 nnm/inference_graph.pb
		 nnm/object_detection_classes.txt
		 nnm/ssd_mobilenet.pbtxt.txt
    LINK_TO  Eigen3::Eigen imgui::imgui opengl::opengl ffmpeg::ffmpeg Freetype::Freetype glu::glu opengl::opengl glfw GLEW::GLEW ZLIB::ZLIB opencv::opencv
)
target_include_directories(imguiAviMex PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/source ${PROJECT_SOURCE_DIR}/bindings)

set(CPACK_NSIS_CONTACT "rajiv.sithiravel@gmail.com")

include(CTest) 
# CTest sets the BUILD_TESTING variable to ON
if (BUILD_TESTING)

add_executable( ${PROJECT_NAME}
				include/Decoder.h
				include/ImGuiPlayer.h
				include/TinyLib.h
				include/PlayAvi.h
				include/WebCam.h
				source/Decoder.cpp
				source/ImGuiPlayer.cpp
				source/WebCam.cpp
				Test/TestPlayImGuiAvi.cpp
                bindings/imgui_impl_glfw.cpp
                bindings/imgui_impl_glfw.h
                bindings/imgui_impl_opengl3.cpp
                bindings/imgui_impl_opengl3.h
                bindings/imgui_impl_opengl3_loader.h
				nnm/inference_graph.pb
				nnm/object_detection_classes.txt
				nnm/ssd_mobilenet.pbtxt.txt
				)
	
add_custom_command(TARGET ${PROJECT_NAME}
POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/nnm/inference_graph.pb ${PROJECT_BINARY_DIR}
COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/nnm/object_detection_classes.txt ${PROJECT_BINARY_DIR}
COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/nnm/ssd_mobilenet.pbtxt.txt ${PROJECT_BINARY_DIR}
)				

if(WIN32) 				
target_compile_options(${PROJECT_NAME} PRIVATE "/Od")   
endif(WIN32)
				
target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)			
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/bindings)
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen imgui::imgui opengl::opengl ffmpeg::ffmpeg Freetype::Freetype glu::glu opengl::opengl glfw GLEW::GLEW ZLIB::ZLIB opencv::opencv)

endif()

