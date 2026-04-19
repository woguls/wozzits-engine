# Combined core and api library
add_library(wozzits_engine
    core/impl/logger_worker.cpp
    core/logger_worker.h
    core/mpsc_queue.h
    core/spsc_queue.h
    ../api/src/logger.cpp
    ../api/logger.h
    ../api/wozzits_time.h
    ../api/wozzits_threading.h
    ../api/src/wozzits_threading.cpp
    ../api/filesystem.h
    ../api/src/filesystem_os.cpp
    ../api/wozzits_async.h
    ../api/src/wozzits_async.cpp
    ../api/window_types.h
    ../api/window.h
    ../api/src/window.cpp
    platform/win32.h
    platform/src/win32.cpp)

target_compile_definitions(wozzits_engine PRIVATE
    $<$<BOOL:${WZ_ENABLE_TESTING}>:WZ_ENABLE_TESTING=1>
)

target_include_directories(wozzits_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/api
)
target_compile_features(wozzits_engine PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(wozzits_engine PRIVATE
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
    )
endif()

if(WIN32)
    target_compile_definitions(wozzits_engine PRIVATE PLATFORM_WINDOWS)
        target_link_libraries(wozzits_engine
        PRIVATE
        user32
        gdi32
        dwmapi
        )
elseif(UNIX)
    target_compile_definitions(wozzits_engine PRIVATE PLATFORM_POSIX)
endif()
