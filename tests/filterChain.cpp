#include <graph/FilterChain.h>
#include <gtest/gtest.h>
#include <iostream>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

std::string g_path = "";

TEST(serverType, findServerObjectWithPath)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   cypher::Path p;
   p.nodes.emplace_back(cypher::Node{"obj", "Object"});
   cypher::Node b;
   b.properties.emplace(std::make_pair("NodeId", "i=2004"));
   p.nodes.emplace_back(b);
   p.relations.emplace_back(cypher::Relationship{"i=40", 1});

   auto f = createFilterChain(p, std::vector<std::reference_wrapper<const FilterChain>>{}, server);
   f->run();

   ASSERT_EQ(f->results()->size(), 1);
   ASSERT_EQ(f->results("obj")->size(), 1);

   UA_Server_delete(server);
}


int main(int argc, char** argv)
{

   testing::InitGoogleTest(&argc, argv);

   if (!(argc > 1))
      return 1;
   g_path = argv[1];

   return RUN_ALL_TESTS();
}