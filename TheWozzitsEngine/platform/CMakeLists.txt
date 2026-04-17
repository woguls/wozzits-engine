# Platform abstraction layer CMake configuration

# Add the error module as a library
add_library(wozzits_platform_logger src/logging.cpp)

# Include the current directory for headers
target_include_directories(wozzits_platform_logger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

# Ensure C++17 is used (for std::string_view, structured bindings, etc.)      
target_compile_features(wozzits_platform_logger PUBLIC cxx_std_17)

# Platform-specific settings (if needed in future)
if(WIN32)
    target_compile_definitions(wozzits_platform_logger PRIVATE
        PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(wozzits_platform_logger PRIVATE
        PLATFORM_POSIX)
endif()
