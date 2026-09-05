add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark PRIVATE benchmark::benchmark open62541::open62541 graphForOpc cypher-parser)

add_executable(stdFunction stdFunction.cpp)
target_link_libraries(stdFunction PRIVATE benchmark::benchmark)