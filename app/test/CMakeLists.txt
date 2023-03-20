# vim: syntax=cmake
# ----------------------------------------------------------------------------
# OSAL built-in unit test case
# ----------------------------------------------------------------------------
# macro for adding osal sub-module unit test
cmake_minimum_required (VERSION 3.0.0)
project(demo)

macro(add_mpp_osal_test module)
    set(test_name ${module}_test)
    string(TOUPPER ${test_name} test_tag)
    add_definitions("-Wall -g")
    #message(STATUS "moduule   : ${module}")
    #message(STATUS "test_name : ${test_name}")
    #message(STATUS "test_tag  : ${test_tag}")

    #option(${test_tag} "Build osal ${module} unit test" ${BUILD_TEST})
    if(1)
        add_executable(${test_name} ${test_name}.c)
        ## 增加连接头文件
        include_directories(../include)
        include_directories(../include/omx_il)
        include_directories(../src/osal)
        include_directories(../src/osal/allocator)
        include_directories(../src/osal/inc)
        include_directories(../src/base/inc)
        include_directories(../src/common/inc)
        include_directories(../src/components/avq)
        target_link_libraries(${test_name}  vx-mpp  pthread dl)
        set_target_properties(${test_name} PROPERTIES FOLDER "osal/test")
        add_test(NAME ${test_name} COMMAND ${test_name})
        set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
        
        #"把DEBUG版的可执行Target放到${CMAKE_BINARY_DIR}/bin目录中"
        #set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
        
        #"把RELEASE版的可执行Target放到${CMAKE_BINARY_DIR}/bin目录中"
        #set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    endif()
endmacro()

# log system unit test
add_mpp_osal_test(mpp_log)

# env system unit test
add_mpp_osal_test(mpp_env)

# malloc system unit test
add_mpp_osal_test(mpp_mem)

#dma device test
add_mpp_osal_test(mpp_dmabuffer_driver)


#dma  test
add_mpp_osal_test(vdec)


