cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(AskiPlot)

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/extern/cxxopts/include)
    message(FATAL_ERROR
        "Missing 'cxxopts' external dependency."
        "Please run 'git submodule update --init' to fix the problem."
    )
endif()


# The target 'askiplot' is the command-line tool
add_executable(askiplot ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_include_directories(askiplot PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/extern/cxxopts/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
set_property(TARGET askiplot PROPERTY CXX_STANDARD 14)

if (DEBUG)
    target_compile_options(askiplot PRIVATE -Wall -Wextra -Wconversion)
endif()