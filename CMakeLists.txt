# CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# set(name "facerec")
# project(facerec_cpp_samples)

# #SET(OpenCV_DIR /path/to/your/opencv/installation)

# add_compile_options(-std=c++11)

# # packages
# find_package(OpenCV REQUIRED) # http://opencv.org

# # probably you should loop through the sample files here
# add_executable(facerec_demo facerec_demo.cpp)
# target_link_libraries(facerec_demo opencv_core opencv_face opencv_imgproc opencv_highgui)


cmake_minimum_required(VERSION 2.4)
project(xdmademo)

set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)
enable_language(CUDA)

message("msg CMAKE_PREFIX_PATH:::${CMAKE_PREFIX_PATH}")
# set(OpenCV_DIR "/usr/local/lib/cmake/opencv4/OpenCVConfig.cmake")
find_package( OpenCV  REQUIRED )
find_package( CUDA )

message(STATUS "OpenCV_VERSION: ${OpenCV_VERSION}")
message(STATUS "OpenCV_INCLUDE_DIRS：${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIB_DIR：${OpenCV_LIB_DIR}")
message(STATUS "OpenCV_LIBS：${OpenCV_LIBS}")

include_directories(
    ${CUDA_INCLUDE_DIRS}
    ./include/
 )

# add_compile_options(-std=c++11 )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -pthread -fopenmp")


set(CPP_FILES ${PROJECT_SOURCE_DIR}/src/cxdma_demo.cpp)

set(LIB_CPP_FILES ${PROJECT_SOURCE_DIR}/src/sdireader.cpp
                ${PROJECT_SOURCE_DIR}/src/cxdma_reader.cpp  
                ${PROJECT_SOURCE_DIR}/src/cxdma_receiver.cpp
                ${PROJECT_SOURCE_DIR}/src/cxdma_utils.cpp
                ${PROJECT_SOURCE_DIR}/src/CircleBuffer.cpp
                ${PROJECT_SOURCE_DIR}/src/event.cpp
                ${PROJECT_SOURCE_DIR}/src/common.cpp
                ${PROJECT_SOURCE_DIR}/src/cpthread.cpp)

message(STATUS "CPP_FILES${CPP_FILES}")

add_library(sdireader SHARED ${LIB_CPP_FILES})
target_link_libraries( sdireader ${OpenCV_LIBRARIES})

add_executable(xdmademo ${CPP_FILES})
target_link_libraries( xdmademo sdireader )
