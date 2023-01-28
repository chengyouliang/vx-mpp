cmake_minimum_required (VERSION 3.0.0)

SET(TARGET "vx-mpp")

include_directories(../include)
include_directories(./include/rk)
include_directories(../include/omx_il)


include_directories(./base)
include_directories(./base/inc)

include_directories(./osal)
include_directories(./osal/allocator)
include_directories(./osal/inc)


aux_source_directory(base DIR_SRCS)
aux_source_directory(osal DIR_SRCS)
aux_source_directory(osal/allocator DIR_SRCS)
aux_source_directory(osal/android DIR_SRCS)
aux_source_directory(osal/linux DIR_SRCS)
aux_source_directory(osal/windows DIR_SRCS)


if (HAVE_DRM)
    message(WARNING  "compile with drm support")
else()
    message(WARNING  "compile without drm support :")
    list(REMOVE_ITEM DIR_SRCS "osal/allocator/allocator_drm.c")
endif()
ADD_LIBRARY(vx-mpp STATIC  ${DIR_SRCS})
#ADD_LIBRARY(vx-mpp SHARED buffer.c)

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib")
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib")