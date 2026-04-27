# Main CMakeLists.txt for TheWozzitsEngine

cmake_minimum_required(VERSION 3.14)
project(TheWozzitsEngine VERSION 0.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



include(FetchContent)
option(WZ_ENABLE_TESTING "Enable test hooks" ON)

# Set runtime library for MSVC to use dynamic runtime (MD/MDd)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()



# We need Google Test
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Configure Google Test to use the same runtime library
if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

# Add the core and api subdirectories
add_subdirectory(twe)


# Enable testing
enable_testing()

function(wz_add_test name)

    add_executable(${name} twe/tests/${name}.cpp)

    target_compile_definitions(${name} PRIVATE WZ_ENABLE_TESTING=1)

    target_include_directories(${name}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/api
        
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/platform
            ${CMAKE_CURRENT_SOURCE_DIR}/core
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(${name}
        GTest::gtest
        GTest::gtest_main
        wozzits_engine
    )

    add_test(NAME ${name} COMMAND ${name})
endfunction()

function(wz_add_test_1 category name)

    set(target "${category}_${name}")

    add_executable(${target} twe/tests/${category}/${name}.cpp)
    set_target_properties(${target} PROPERTIES FOLDER "tests/${category}")

    target_compile_definitions(${target} PRIVATE WZ_ENABLE_TESTING=1)

    target_include_directories(${target}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/api
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/platform
            ${CMAKE_CURRENT_SOURCE_DIR}/core
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(${target}
        GTest::gtest
        GTest::gtest_main
        wozzits_engine
    )

    add_test(NAME ${target} COMMAND ${target})
endfunction()



wz_add_test_1(containers mpsc_queue)
wz_add_test_1(containers mpsc_ring_buffer)
wz_add_test_1(containers spsc_queue)
wz_add_test_1(containers buffer)
wz_add_test_1(containers buffer_2)

wz_add_test_1(algo algo_test_1)

# wz_add_test_1(render frame_builder_tests)

wz_add_test_1(scene transform_node_test)

wz_add_test(logging)
wz_add_test(threading)
wz_add_test(time)
wz_add_test(file_0)
wz_add_test(test_window)
wz_add_test(hormuz_0)
wz_add_test(engine)

wz_add_test_1(math test_vec3)
wz_add_test_1(math test_mat4)
wz_add_test_1(math test_quaternion)
wz_add_test_1(math test_quaternion_mat4)
wz_add_test_1(math test_camera)