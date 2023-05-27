add_executable(queryServer queryServer.cpp)
target_link_libraries(queryServer PRIVATE graphForOpc cypher-parser)

add_executable(queryClient queryClient.cpp)
target_link_libraries(queryClient PRIVATE open62541::open62541 CONAN_PKG::cpp-httplib project_options)

configure_file(index.html ${CMAKE_BINARY_DIR}/bin COPYONLY)