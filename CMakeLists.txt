cmake_minimum_required(VERSION 3.10)

project(graphForOpc LANGUAGES C CXX)

find_package(open62541 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(benchmark REQUIRED)
find_package(httplib REQUIRED)
find_package(GTest REQUIRED)

add_library(project_warnings INTERFACE)
include(${CMAKE_SOURCE_DIR}/cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

add_library(graphForOpc ${CMAKE_CURRENT_SOURCE_DIR}/src/cypher/Parser.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/cypher/Path.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/MatchClause.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/PathMatcher.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/Path.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/tracing.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/QueryEngine.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/HierachicalVisitor.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/SourceColumn.cpp)
target_include_directories(graphForOpc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(graphForOpc PUBLIC project_warnings project_options open62541::open62541 nlohmann_json::nlohmann_json)

#add_subdirectory(csvExport)
add_subdirectory(playground)

include(CTest)
enable_testing()

add_subdirectory(tests)
add_subdirectory(benchmark)
