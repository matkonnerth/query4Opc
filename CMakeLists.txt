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

find_package(NodesetLoader REQUIRED)
find_package(open62541 REQUIRED)

add_executable(server main.cpp QueryLite.cpp)
target_link_libraries(server PRIVATE open62541::open62541)
target_link_libraries(server PRIVATE NodesetLoader)

add_subdirectory(csvExport)

include(CTest)
enable_testing()

add_subdirectory(tests)
