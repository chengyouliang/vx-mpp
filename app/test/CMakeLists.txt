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
        target_link_libraries(${test_name}  vx-mpp  pthread)
        set_target_properties(${test_name} PROPERTIES FOLDER "osal/test")
        add_test(NAME ${test_name} COMMAND ${test_name})
    endif()
endmacro()

# log system unit test
add_mpp_osal_test(mpp_log)

# env system unit test
add_mpp_osal_test(mpp_env)

# malloc system unit test
add_mpp_osal_test(mpp_mem)

# mpp_mem_pool unit test
add_mpp_osal_test(mpp_mem_pool)

# trace system unit test
add_mpp_osal_test(mpp_trace)

# software runtime feature detection unit test
add_mpp_osal_test(mpp_runtime)

# mpp buffer test 
add_mpp_osal_test(mpp_buffer)


#dma device test
add_mpp_osal_test(mpp_dmabuffer_driver)



