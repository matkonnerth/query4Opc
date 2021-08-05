#include <cypher/Parser.h>
#include <cypher-parser.h>
#include <iostream>

std::optional<Query> Parser::parse(const std::string& queryString)
{
   cypher_parse_result_t* result = cypher_parse(queryString.c_str(), NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
   if (result == NULL)
   {
      std::cout << "parsing failed" << "\n";
      return std::nullopt;
   }

   printf("Parsed %d AST nodes\n", cypher_parse_result_nnodes(result));
   printf("Read %d statements\n", cypher_parse_result_ndirectives(result));
   printf("Encountered %d errors\n", cypher_parse_result_nerrors(result));

   cypher_parse_result_fprint_ast(result, stdout, 20, cypher_parser_ansi_colorization, 0);

   if(cypher_parse_result_nerrors(result))
   {
      std::cout << "parsing failed with errors"
                << "\n";
      cypher_parse_result_free(result);
      return std::nullopt;
   }

   const cypher_astnode_t* ast = cypher_parse_result_get_directive(result, 0);
   if (cypher_astnode_type(ast) != CYPHER_AST_STATEMENT)
   {
      std::cout << "unsupported statement"
                << "\n";
      cypher_parse_result_free(result);
      return std::nullopt;
   }

   const cypher_astnode_t* query = cypher_ast_statement_get_body(ast);
   if(cypher_astnode_type(query)!=CYPHER_AST_QUERY)
   {
      std::cout << "unsupported statement body: only qeries are supported" << "\n";
      cypher_parse_result_free(result);
      return std::nullopt;
   } 

   if(cypher_ast_query_nclauses(query) != 2)
   {
      std::cout << "unsupport query: only exactly 1 match clause and 1 return clause are supported" << "\n";
      cypher_parse_result_free(result);
      return std::nullopt;
   }

   const cypher_astnode_t* clause = cypher_ast_query_get_clause(query, 0);
   if(cypher_astnode_type(clause)!=CYPHER_AST_MATCH)
   {
      std::cout << "unsupport query: 1st clause must be a match clause"
                << "\n";
      cypher_parse_result_free(result);
      return std::nullopt;
   }
   clause = cypher_ast_query_get_clause(query, 1);
   if(cypher_astnode_type(clause)!=CYPHER_AST_RETURN)
   {
      std::cout << "unsupported query: 2nd clause must be the return clause" << "\n";
      cypher_parse_result_free(result);
      return std::nullopt;
   }

   auto matchClause = cypher_ast_query_get_clause(query, 0);
   auto pattern = cypher_ast_match_get_pattern(matchClause);
   if(cypher_astnode_type(pattern)!=CYPHER_AST_PATTERN)
   {
      std::cout << "unsupported query: there should be a pattern"
                << "\n";
      cypher_parse_result_free(result);
      return std::nullopt;
   }
   auto patternPath = cypher_ast_pattern_get_path(pattern, 0);
   if(cypher_astnode_type(patternPath)!=CYPHER_AST_PATTERN_PATH)
   {
      std::cout << "unsupported query: there should be a patternPath"
                << "\n";
      cypher_parse_result_free(result);
      return std::nullopt;
   }

   Query q;
   q.matchClauses.emplace_back(Match());
   for(auto i=0u; i<cypher_ast_pattern_path_nelements(patternPath); ++i)
   {
      auto elem = cypher_ast_pattern_path_get_element(patternPath, i);
      if(i%2==0)
      {
         //node
         Node n;
         auto identifier = cypher_ast_node_pattern_get_identifier(elem);
         if(identifier)
         {
            n.identifier = cypher_ast_identifier_get_name(identifier);
         }
         auto label = cypher_ast_node_pattern_get_label(elem, 0);
         if(label)
         {
            n.label = cypher_ast_label_get_name(label);
         }         
         auto properties = cypher_ast_node_pattern_get_properties(elem);
         if(properties)
         {
            for(auto ii=0u;ii<cypher_ast_map_nentries(properties); ++ii)
            {
               auto key = cypher_ast_prop_name_get_value(cypher_ast_map_get_key(properties, ii));
               auto value = cypher_ast_string_get_value(cypher_ast_map_get_value(properties, ii));
               n.properties.emplace(std::make_pair(key, value));
            }
         }
         q.matchClauses[0].path.nodes.push_back(n);
      }
      else
      {
         // node
         Relationship r;
         if(cypher_ast_rel_pattern_nreltypes(elem)==1)
         {
            auto type = cypher_ast_rel_pattern_get_reltype(elem, 0);
            if (type)
            {
               r.type = cypher_ast_reltype_get_name(type);
            }
         }
         
         q.matchClauses[0].path.relations.push_back(r);
      }
   }
   


   cypher_parse_result_free(result);
   return q;
}