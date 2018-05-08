cmake_minimum_required(VERSION 2.8)

macro (add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate SRCS to parent directory
        set (SRCS ${SRCS} PARENT_SCOPE)
    endif()
endmacro()

add_sources(
    src/main.cpp
    src/dep_parser/parser.cpp
    src/extensions/error_reporter.cpp)

add_executable(lazyrt ${SRCS})
target_include_directories(lazyrt PRIVATE ${PROJECT_SOURCE_DIR}/include)

