# Main CMakeLists.txt for TheWozzitsEngine

cmake_minimum_required(VERSION 3.14)
project(TheWozzitsEngine VERSION 1.0.0)

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

# Build the logging tests executable
add_executable(logging_tests
    tests/test_logging.cpp
)
target_link_options(logging_tests PRIVATE /PROFILE)
target_link_libraries(logging_tests
    GTest::gtest
    GTest::gtest_main
    wozzits_api
)

add_test(NAME logging_tests COMMAND logging_tests)

# Build the MPSC queue tests executable
add_executable(test_mpsc_queue
    tests/test_mpsc_queue.cpp
)
target_link_libraries(test_mpsc_queue
    GTest::gtest
    GTest::gtest_main
    wozzits_api
)

add_test(NAME test_mpsc_queue COMMAND test_mpsc_queue)
