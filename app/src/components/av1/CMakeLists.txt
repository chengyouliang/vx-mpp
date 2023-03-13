cmake_minimum_required (VERSION 3.0.0)

add_definitions(-g)

#关闭警告
#add_definitions(-w)
add_definitions(-W)


SET(TARGET "vx_av1")

# include
include_directories(../../../include/omx_il)
include_directories(../base/inc)
include_directories(../common/inc)

# src
SET(SRC_LIST omx_videodec_component.c/
             omx_videodec_component.h/
             omx_videoenc_component.c/
             omx_videoenc_component.h/
             library_entry_point.c)


target_link_libraries(${test_name}  vx_mpp_share)
#ADD_LIBRARY(${TARGET} STATIC  ${DIR_SRCS})
ADD_LIBRARY(vx_av1 SHARED  ${SRC_LIST})


set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(vx_av1 PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


#静态库：     ARCHIVE_OUTPUT_DIRECTORY_<CONFIG>
#动态库：     LIBRARY_OUTPUT_DIRECTORY_<CONFIG>
#可执行文件：  RUNTIME_OUTPUT_DIRECTORY_<CONFIG>
 
#"把可执行Target放到${CMAKE_BINARY_DIR}/bin目录中"
#set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
 
#"把DEBUG版的可执行Target放到${CMAKE_BINARY_DIR}/bin目录中"
#set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
 
#"把RELEASE版的可执行Target放到${CMAKE_BINARY_DIR}/bin目录中"
#set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
