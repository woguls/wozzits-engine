target_sources(wozzits_engine PRIVATE
    win32.cpp
    time.cpp
)

target_include_directories(wozzits_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)