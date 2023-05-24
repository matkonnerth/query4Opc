#include <cypher-parser.h>
#include <errno.h>
#include <stdio.h>
#include <string>
#include <vector>

void printQueryAst(const std::string& query)
{
   cypher_parse_result_t* result = cypher_parse(query.c_str(), NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
   if (result == NULL)
   {
      return;
   }

   printf("Parsed %d AST nodes\n", cypher_parse_result_nnodes(result));
   printf("Read %d statements\n", cypher_parse_result_ndirectives(result));
   printf("Encountered %d errors\n", cypher_parse_result_nerrors(result));

   cypher_parse_result_fprint_ast(result, stdout, 20, cypher_parser_ansi_colorization, 0);


   cypher_parse_result_free(result);
}

int main(int argc, char* argv[])
{

   std::vector<std::string> queries;
   queries.emplace_back("MATCH (obj:Object) RETURN obj");
   queries.emplace_back("MATCH (obj:Object)-[:HasTypeDefinition]->(t:ObjectType{NodeId:\"MyType\"}) RETURN obj");
   queries.emplace_back("MATCH (obj:Object{NodeId:\"MyObject\"}) return obj");
   queries.emplace_back("MATCH (:Variable)-[:HasProperty]-(obj:Object)-[:HasTypeDefinition]->(t:ObjectType{NodeId:\"MyType\"}) RETURN obj");
   queries.emplace_back(
      R"(   MATCH (obj:Object)-[:HasTypeDefinition]->(t:ObjectType{NodeId:"MyType"})
            MATCH (obj)--(:Variable)
            RETURN obj)");
   queries.emplace_back(R"(
   MATCH (:Variable)-[:HasProperty]-(obj:Object)-[:HasTypeDefinition]->(t:ObjectType{NodeId: "MyType"})
   WHERE root:Object{NodeId: "i=85"}
   RETURN obj)");

   for (const auto& q : queries)
   {
      printQueryAst(q);
   }
   return EXIT_SUCCESS;
}