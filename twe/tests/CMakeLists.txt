find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(math_tests
    math/test_vec3.cpp
    math/test_mat4.cpp
)

target_link_libraries(math_tests
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME math_tests COMMAND math_tests)