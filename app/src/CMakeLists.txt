cmake_minimum_required (VERSION 3.0.0)

add_definitions(-g)

#关闭警告
#add_definitions(-w)
add_definitions(-W)


SET(TARGET "vx-mpp")


# include
include_directories(../include)
include_directories(../include/alsa)
include_directories(../include/omx_il)

include_directories(./base)
include_directories(./base/inc)


include_directories(./common/inc)

include_directories(./osal)
include_directories(./osal/allocator)
include_directories(./osal/inc)

# src
aux_source_directory(base DIR_SRCS)
aux_source_directory(osal DIR_SRCS)
aux_source_directory(osal/allocator DIR_SRCS)
aux_source_directory(osal/android DIR_SRCS)
aux_source_directory(osal/linux DIR_SRCS)
aux_source_directory(osal/windows DIR_SRCS)
aux_source_directory(common DIR_SRCS)
#aux_source_directory(components/alsa  DIR_SRCS)
#aux_source_directory(components/av1  DIR_SRCS)

message(WARNING  "compile with drm support")

if (HAVE_DRM)
    message(WARNING  "compile with drm support")
else()
    message(WARNING  "compile without drm support :")
    list(REMOVE_ITEM DIR_SRCS "osal/allocator/allocator_drm.c")
endif()
ADD_LIBRARY(${TARGET} STATIC  ${DIR_SRCS})
ADD_LIBRARY(vx_mpp_share SHARED  ${DIR_SRCS})

set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(vx_mpp_share PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


#静态库：     ARCHIVE_OUTPUT_DIRECTORY_<CONFIG>
#动态库：     LIBRARY_OUTPUT_DIRECTORY_<CONFIG>
#可执行文件：  RUNTIME_OUTPUT_DIRECTORY_<CONFIG>
 
#"把可执行Target放到${CMAKE_BINARY_DIR}/bin目录中"
#set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
 
#"把DEBUG版的可执行Target放到${CMAKE_BINARY_DIR}/bin目录中"
#set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
 
#"把RELEASE版的可执行Target放到${CMAKE_BINARY_DIR}/bin目录中"
#set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
add_subdirectory(./components/av1)

