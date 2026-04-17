# Main CMakeLists.txt for TheWozzitsEngine

cmake_minimum_required(VERSION 3.14)
project(TheWozzitsEngine VERSION 1.0.0)

include(FetchContent)

# We need Google Test
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googletest)

# Add the platform subdirectory (includes error module)
add_subdirectory(platform)

# Enable testing
enable_testing()

# Build the logging tests executable
add_executable(logging_tests
    tests/platform/logging/test_logging.cpp
    platform/src/logging.cpp
)

target_link_libraries(logging_tests
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME logging_tests COMMAND logging_tests)





