cmake_minimum_required(VERSION 2.8)
project(cmdline CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMDLINE_HEADERS
        include/cmdline/error.hpp
        include/cmdline/detail.hpp
        include/cmdline/options.hpp
        include/cmdline/reader.hpp
)

set(CMDLINE_HEADER
        include/cmdline/cmdline.hpp
)

set(CMDLINE_COMPILE_FILE src/compile.cpp)

if (BUILD_STATIC_LIBRARY)
    add_library(cmdline STATIC ${CMDLINE_HEADERS} ${CMDLINE_HEADER} ${CMDLINE_COMPILE_FILE})
endif ()

if (BUILD_SHARED_LIBRARY)
    add_library(cmdline SHARED ${CMDLINE_HEADERS} ${CMDLINE_HEADER} ${CMDLINE_COMPILE_FILE})
endif ()

if (BUILD_EXAMPLE)
    add_subdirectory(example)
endif ()