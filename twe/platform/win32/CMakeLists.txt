target_sources(wozzits_engine PRIVATE
    win32.cpp
    w_time.cpp
    ri_win32.cpp
)

target_include_directories(wozzits_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)