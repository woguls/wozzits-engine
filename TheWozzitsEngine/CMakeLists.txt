# Main CMakeLists.txt for TheWozzitsEngine

cmake_minimum_required(VERSION 3.14)
project(TheWozzitsEngine VERSION 1.0.0)

# Force the use of clang-cl compiler
set(CMAKE_C_COMPILER "clang-cl")
set(CMAKE_CXX_COMPILER "clang-cl")

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

# Add the platform subdirectory (includes error module)
add_subdirectory(platform)

# Enable testing
enable_testing()

# Build the logging tests executable
add_executable(logging_tests
    tests/platform/logging/test_logging.cpp
)

target_link_libraries(logging_tests
    GTest::gtest
    GTest::gtest_main
    wozzits_platform_logger
)

add_test(NAME logging_tests COMMAND logging_tests)

# Build the logging tests executable with AddressSanitizer
add_executable(logging_tests_asan
    tests/platform/logging/test_logging.cpp
)

target_link_libraries(logging_tests_asan
    GTest::gtest
    GTest::gtest_main
    wozzits_platform_logger
)

# AddressSanitizer is only supported by GCC and Clang
target_compile_options(logging_tests_asan PRIVATE -fsanitize=address)
target_link_options(logging_tests_asan PRIVATE -fsanitize=address)

# Build the logging tests executable with ThreadSanitizer (TSAN)
add_executable(logging_tests_tsan
    tests/platform/logging/test_logging.cpp
)

target_link_libraries(logging_tests_tsan
    GTest::gtest
    GTest::gtest_main
    wozzits_platform_logger
)

# TSAN flags for Clang/LLVM
target_compile_options(logging_tests_tsan PRIVATE -fsanitize=thread)
target_link_options(logging_tests_tsan PRIVATE -fsanitize=thread)
