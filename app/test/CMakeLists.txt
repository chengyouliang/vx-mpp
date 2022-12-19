cmake_minimum_required (VERSION 3.0.0)

PROJECT(vxmpp_test)
include_directories(../include)

ADD_EXECUTABLE(vxmpp_test main.c)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include") #要到haha目录，否则找不到

TARGET_LINK_LIBRARIES(vxmpp_test  vx-mpp)

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin")