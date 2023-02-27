add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark PRIVATE CONAN_PKG::benchmark open62541::open62541 graphForOpc cypher-parser)

add_executable(stdFunction stdFunction.cpp)
target_link_libraries(stdFunction PRIVATE CONAN_PKG::benchmark)

add_executable(stdVector stdVector.cpp)
target_link_libraries(stdVector PRIVATE CONAN_PKG::benchmark)