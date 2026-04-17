# Main CMakeLists.txt for TheWozzitsEngine

cmake_minimum_required(VERSION 3.14)
project(TheWozzitsEngine VERSION 1.0.0)

# Add the platform subdirectory (includes error module)
add_subdirectory(platform)

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest REQUIRED)

# Include test source directory
include_directories(${CMAKE_SOURCE_DIR}/tests)

# Build the logging tests executable
add_executable(logging_tests
    tests/platform/logging/test_logging.cpp
    platform/src/logging.cpp
)

# Link Google Test and pthreads
target_link_libraries(logging_tests
    GTest::GTest
    GTest::Main
    pthread
)

# Register the test
add_test(NAME logging_tests COMMAND logging_tests)
