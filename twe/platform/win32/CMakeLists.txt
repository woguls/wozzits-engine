target_sources(wozzits_engine PRIVATE
    win32copy.cpp
    w_time.cpp
)

target_include_directories(wozzits_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)