cmake_minimum_required (VERSION 3.0.0)

SET(TARGET "vx-mpp")

include_directories(../include)
aux_source_directory(./base DIR_SRCS)

ADD_LIBRARY(vx-mpp STATIC  ${DIR_SRCS})
#ADD_LIBRARY(vx-mpp SHARED buffer.c)

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib")
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib")