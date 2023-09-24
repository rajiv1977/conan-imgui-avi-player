cmake_minimum_required(VERSION 3.27.0)

project(conan-video-player LANGUAGES CXX C VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)

if(WIN32) 
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:100000000")
endif(WIN32) 
find_package(opencv REQUIRED)

## find Matlab's matrix and mex libraries
find_package(Matlab COMPONENTS MAT_LIBRARY MX_LIBRARY)
if (NOT Matlab_FOUND)
    message(FATAL_ERROR "Matlab dependencies not found. Is the MATLAB_PATH environment variable set?")
endif()

# build the mex
matlab_add_mex(
    NAME videoMex
    SHARED 
	SRC include/Video.h
		source/Video.cpp
	    source/videoMex.cpp
   LINK_TO  opencv::opencv
)
target_include_directories(videoMex PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/source ${PROJECT_SOURCE_DIR}/nnm)

set(CPACK_NSIS_CONTACT "rajiv.sithiravel@gmail.com")

include(CTest) 
# CTest sets the BUILD_TESTING variable to ON
if (BUILD_TESTING)

add_executable( ${PROJECT_NAME}
				include/Video.h
				source/Video.cpp
				Test/TestVideoGui.cpp
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
						
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/nnm)
target_link_libraries(${PROJECT_NAME} opencv::opencv)

endif()

