cmake_minimum_required (VERSION 3.0.0)

add_definitions(-g)
add_definitions(-W)

# include
include_directories(${PROJECT_SOURCE_DIR}/include/omx_il)
include_directories(${PROJECT_SOURCE_DIR}/src/base/inc)
include_directories(${PROJECT_SOURCE_DIR}/src/common/inc)
include_directories(./vdec)
include_directories(.)

message(WARNING  "${PROJECT_SOURCE_DIR}")
# src
aux_source_directory(.  SRC_LIST)

link_directories(${CMAKE_BINARY_DIR}/bin)
#lib
ADD_LIBRARY(vx-h26x SHARED  ${SRC_LIST})
target_link_libraries(vx-h26x  vdec)

file(COPY lib/libvdec.so DESTINATION  ${CMAKE_BINARY_DIR}/bin)
file(COPY lib/libvdec.so.1  DESTINATION  ${CMAKE_BINARY_DIR}/bin)
file(COPY lib/libvdec.so.1.0.1 DESTINATION  ${CMAKE_BINARY_DIR}/bin)

set_target_properties(vx-h26x PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/component )


#静态库：     ARCHIVE_OUTPUT_DIRECTORY_<CONFIG>
#动态库：     LIBRARY_OUTPUT_DIRECTORY_<CONFIG>
#可执行文件：  RUNTIME_OUTPUT_DIRECTORY_<CONFIG>
 
#"把可执行Target放到${CMAKE_BINARY_DIR}/bin目录中"
#set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
 
#"把DEBUG版的可执行Target放到${CMAKE_BINARY_DIR}/bin目录中"
#set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
 
#"把RELEASE版的可执行Target放到${CMAKE_BINARY_DIR}/bin目录中"
#set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
