add_executable(queryServer queryServer.cpp)
target_link_libraries(queryServer PRIVATE graphForOpc cypher-parser)

add_executable(queryClient queryClient.cpp)
target_link_libraries(queryClient PRIVATE open62541::open62541 project_options)