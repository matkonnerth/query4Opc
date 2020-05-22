cmake_minimum_required(VERSION 3.0)

project(graphForOpc)

find_package(NodesetLoader REQUIRED)
find_package(open62541 REQUIRED)

add_executable(server main.c)
target_link_libraries(server PRIVATE open62541::open62541)
target_link_libraries(server PRIVATE NodesetLoader)
