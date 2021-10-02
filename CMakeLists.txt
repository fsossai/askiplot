cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(AskiPlot)

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/extern/cxxopts/include)
    message(FATAL_ERROR
        "Missing 'cxxopts' external dependency."
        "Please run 'git submodule update --init' to fix the problem."
    )
endif()


# Getting the current version 
file(READ ${CMAKE_SOURCE_DIR}/VERSION version_str)
string(STRIP ${version_str} version_str)
string(REPLACE "." ";" version_str ${version_str}) # String split
list(GET version_str 0 VERSION_MAJOR)
list(GET version_str 1 VERSION_MINOR)
list(GET version_str 2 VERSION_BUILD)
message(STATUS "AskiPlot version: " ${VERSION_MAJOR} "." ${VERSION_MINOR} "." ${VERSION_BUILD})
configure_file(${CMAKE_SOURCE_DIR}/include/AskiPlot/version.hpp.config ${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp @ONLY)

# The target 'askiplot' is the command-line tool
add_executable(askiplot ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_include_directories(askiplot PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/extern/cxxopts/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
set_property(TARGET askiplot PROPERTY CXX_STANDARD 14)