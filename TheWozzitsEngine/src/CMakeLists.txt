# Core library
add_library(wozzits_core
    core/impl/logger_worker.cpp
    core/mpsc_queue.h
)
target_include_directories(wozzits_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)
target_compile_features(wozzits_core PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(wozzits_core PRIVATE
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
    )
endif()

if(WIN32)
    target_compile_definitions(wozzits_core PRIVATE PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(wozzits_core PRIVATE PLATFORM_POSIX)
endif()

# API library
add_library(wozzits_api
    api/src/logger.cpp
    api/logger.h
)
target_include_directories(wozzits_api PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/api
)
target_compile_features(wozzits_api PUBLIC cxx_std_17)
target_link_libraries(wozzits_api PUBLIC wozzits_core)

if(MSVC)
    target_compile_options(wozzits_api PRIVATE
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
    )
endif()

if(WIN32)
    target_compile_definitions(wozzits_api PRIVATE PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(wozzits_api PRIVATE PLATFORM_POSIX)
endif()
