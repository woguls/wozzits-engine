# Main CMakeLists.txt for TheWozzitsEngine

cmake_minimum_required(VERSION 3.14)
project(TheWozzitsEngine VERSION 1.0.0)

option(WZ_ENABLE_TESTING "Enable test hooks" ON)

# Set runtime library for MSVC to use dynamic runtime (MD/MDd)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

include(FetchContent)

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
add_subdirectory(src)

# Enable testing
enable_testing()


# Build the MPSC queue tests executable
add_executable(test_mpsc_queue
    tests/test_mpsc_queue.cpp
)

add_executable(test_logging
    tests/test_logging.cpp
)

target_compile_definitions(test_logging PRIVATE WZ_ENABLE_TESTING=1)

target_link_libraries(test_logging
    GTest::gtest
    GTest::gtest_main
    wozzits_engine
)

target_link_libraries(test_mpsc_queue
    GTest::gtest
    GTest::gtest_main
    wozzits_engine
)

add_test(NAME test_mpsc_queue COMMAND test_mpsc_queue)
add_test(NAME test_logging COMMAND test_logging)
