add_library(wozzits_engine)

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(src)

target_compile_features(wozzits_engine PUBLIC cxx_std_20)

target_include_directories(wozzits_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/api

        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/platform
        ${CMAKE_CURRENT_SOURCE_DIR}/core 
)

target_compile_definitions(wozzits_engine PRIVATE
    $<$<BOOL:${WZ_ENABLE_TESTING}>:WZ_ENABLE_TESTING=1>
)

if(WIN32)
    target_compile_definitions(wozzits_engine PRIVATE PLATFORM_WINDOWS)
    target_link_libraries(wozzits_engine PRIVATE user32 gdi32 dwmapi)
elseif(UNIX)
    target_compile_definitions(wozzits_engine PRIVATE PLATFORM_POSIX)
endif()