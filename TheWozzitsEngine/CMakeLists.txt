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

add_executable(test_threading
    tests/test_threading.cpp
)

add_executable(test_time
    tests/test_wozzits_time.cpp)

add_executable(test_file
    tests/test_file_0.cpp)

add_executable(test_window
    tests/test_window.cpp)

add_executable(test_spsc_queue
    tests/test_spsc_queue.cpp)

target_compile_definitions(test_time PRIVATE WZ_ENABLE_TESTING=1)
target_compile_definitions(test_mpsc_queue PRIVATE WZ_ENABLE_TESTING=1)
target_compile_definitions(test_spsc_queue PRIVATE WZ_ENABLE_TESTING=1)
target_compile_definitions(test_threading PRIVATE WZ_ENABLE_TESTING=1)
target_compile_definitions(test_logging PRIVATE WZ_ENABLE_TESTING=1)
target_compile_definitions(test_file PRIVATE WZ_ENABLE_TESTING=1)
target_compile_definitions(test_window PRIVATE WZ_ENABLE_TESTING=1)

target_link_libraries(test_time
    wozzits_engine
)

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

target_link_libraries(test_spsc_queue
    GTest::gtest
    GTest::gtest_main
    wozzits_engine
)

target_link_libraries(test_threading
    GTest::gtest
    GTest::gtest_main
    wozzits_engine
)

target_link_libraries(test_file
    GTest::gtest
    GTest::gtest_main
    wozzits_engine
)

target_link_libraries(test_window
    GTest::gtest
    GTest::gtest_main
    wozzits_engine
)

add_test(NAME test_mpsc_queue COMMAND test_mpsc_queue)
add_test(NAME test_logging COMMAND test_logging)
add_test(NAME test_time COMMAND test_time)
add_test(NAME test_threading COMMAND test_threading)
add_test(NAME test_file COMMAND test_file)
add_test(NAME test_window COMMAND test_window)
add_test(NAME test_spsc_queue COMMAND test_spsc_queue)