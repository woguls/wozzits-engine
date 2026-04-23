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
    )

    target_link_libraries(${name}
        GTest::gtest
        GTest::gtest_main
        wozzits_engine
    )

    add_test(NAME ${name} COMMAND ${name})
endfunction()

wz_add_test(mpsc_queue)
wz_add_test(mpsc_ring_buffer)
wz_add_test(spsc_queue)
wz_add_test(logging)
wz_add_test(threading)
wz_add_test(time)
wz_add_test(file_0)
wz_add_test(test_window)
wz_add_test(hormuz_0)
wz_add_test(engine)
