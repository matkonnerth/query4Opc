cmake_minimum_required(VERSION 3.0)

project(graphForOpc)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(CONANFILE conanfile.txt
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

find_package(open62541 REQUIRED)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)
include(${CMAKE_SOURCE_DIR}/cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

add_library(graphForOpc ${CMAKE_CURRENT_SOURCE_DIR}/src/cypher/Parser.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/cypher/Path.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/FilterChain.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/PathMatcher.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/Path.cpp)
target_include_directories(graphForOpc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(graphForOpc PUBLIC project_warnings project_options open62541::open62541)

add_subdirectory(csvExport)

include(CTest)
enable_testing()

#add_subdirectory(libcypher-parser)
add_subdirectory(tests)
