add_executable(filter filter.cpp)
target_include_directories(filter PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(filter PRIVATE open62541::open62541)

add_executable(cypher cypher.cpp)
target_link_libraries(cypher PRIVATE cypher-parser)