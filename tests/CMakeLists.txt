macro(opc_add_test arg)
    add_executable(${arg} ${arg}.cpp)
    target_compile_features(${arg} PRIVATE cxx_std_17)
    target_link_libraries(${arg} PRIVATE graphForOpc NodesetLoader CONAN_PKG::gtest)
    add_test(NAME ${arg}_Test WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin COMMAND ${arg} ${CMAKE_CURRENT_LIST_DIR})
    target_include_directories(${arg} PRIVATE ${CMAKE_SOURCE_DIR})
endmacro()

opc_add_test("filter")



add_executable(cypher cypher.cpp)
target_link_libraries(cypher PRIVATE cypher-parser)