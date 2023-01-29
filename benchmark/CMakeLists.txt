add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark PRIVATE CONAN_PKG::benchmark open62541::open62541 graphForOpc cypher-parser)